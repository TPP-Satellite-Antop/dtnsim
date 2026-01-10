#include <iostream>
#include "ContactlessDtn.h"
#include "../../../../../../omnetpp-6.1/include/omnetpp/clog.h"
#include "../../../dtnsim_m.h"
#include "../../MsgTypes.h"
#include "../routing/RoutingAntop.h"
#include "src/node/mobility/SatelliteMobility.h"
#include "src/node/dtn/contactless/ContactlessSdrModel.h"

Define_Module(ContactlessDtn);

ContactlessDtn::ContactlessDtn() {}

ContactlessDtn::~ContactlessDtn() = default;

/**
 * Initializes the ContactlessDtn object
 *
 * @param stage: the stage for the dtn object
 *
 * @authors The original implementation was done by the authors of DTNSim and then modified by Simon
 * Rink
 */
void ContactlessDtn::initialize(const int stage) {
    if (stage == 1) {
        this->sdr_ = new ContactlessSdrModel();
        // Store this node eid
        this->eid_ = this->getParentModule()->getIndex();

        this->custodyTimeout_ = par("custodyTimeout");
        this->custodyModel_.setEid(eid_);
        this->custodyModel_.setSdr(sdr_);
        this->custodyModel_.setCustodyReportByteSize(par("custodyReportByteSize"));

        // Get a pointer to graphics module
        graphicsModule = dynamic_cast<Graphics *>(this->getParentModule()->getSubmodule("graphics"));
        // Register this object as sdr observer, in order to display bundles stored in sdr properly.
        sdr_->addObserver(this);
        update();

        this->initializeRouting(par("routing"));

        // Register signals
        dtnBundleSentToCom = registerSignal("dtnBundleSentToCom");
        dtnBundleSentToApp = registerSignal("dtnBundleSentToApp");
        dtnBundleSentToAppHopCount = registerSignal("dtnBundleSentToAppHopCount");
        dtnBundleSentToAppRevisitedHops = registerSignal("dtnBundleSentToAppRevisitedHops");
        dtnBundleReceivedFromCom = registerSignal("dtnBundleReceivedFromCom");
        dtnBundleReceivedFromApp = registerSignal("dtnBundleReceivedFromApp");
        dtnBundleReRouted = registerSignal("dtnBundleReRouted");
        sdrBundleStored = registerSignal("sdrBundleStored");
        sdrBytesStored = registerSignal("sdrBytesStored");

        if (eid_ != 0) {
            emit(sdrBundleStored, sdr_->getBundlesCountInSdr());
            emit(sdrBytesStored, sdr_->getBytesStoredInSdr());
        }

        // Initialize BundleMap
        this->saveBundleMap_ = par("saveBundleMap");
        if (saveBundleMap_ && eid_ != 0) {
            // create result folder if it doesn't exist
            struct stat st = {0};
            if (stat("results", &st) == -1) {
                mkdir("results", 0700);
            }

            string fileStr = "results/BundleMap_Node" + to_string(eid_) + ".csv";
            bundleMap_.open(fileStr);
            bundleMap_ << "SimTime"
                       << ","
                       << "SRC"
                       << ","
                       << "DST"
                       << ","
                       << "TSRC"
                       << ","
                       << "TDST"
                       << ","
                       << "BitLenght"
                       << ","
                       << "DurationSec" << endl;
        }
    }
}

void ContactlessDtn::setMobilityMap(map<int, inet::SatelliteMobility*> *mobilityMap) {
    this->mobilityMap_ = mobilityMap;
}

void ContactlessDtn::initializeRouting(const string& routingString) {
    const auto contactLessSdrModel = dynamic_cast<ContactlessSdrModel*>(sdr_);
    contactLessSdrModel->setEid(eid_);
    contactLessSdrModel->setSize(par("sdrSize"));
    contactLessSdrModel->setNodesNumber(this->getParentModule()->getParentModule()->par("nodesNumber"));

    if (routingString == "antop") {
        auto* mobility = dynamic_cast<inet::SatelliteMobility*>(this->getParentModule()->getSubmodule("mobility"));
        (*this->mobilityMap_)[eid_] = mobility;
        this->routing = new RoutingAntop(this->antop, this->eid_, mobilityMap_);
    } else {
        cout << "dtnsim error: unknown routing type: " << routingString << endl;
    }
}

void ContactlessDtn::finish() {
    // Last call to sample-hold type metrics
    if (eid_ != 0) {
        emit(sdrBundleStored, sdr_->getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_->getBytesStoredInSdr());
    }

    sdr_->freeSdr();

    // Delete FWDs in flight.
    for (auto &[_, fwd] : fwdByEid_)
        cancelAndDelete(fwd);
    fwdByEid_.clear();

    // ToDo: cancel and delete routing retry message(s).

    delete routing;
}

/**
 * Reacts to a system message.
 *
 * @param: msg: A pointer to the received message
 *
 * @authors Gastón Frenkel & Valentina Adelsflügel.
 */
void ContactlessDtn::handleMessage(cMessage *msg) {
    switch (msg->getKind()) {
        case BUNDLE:
	    if (msg->arrivedOn("gateToCom$i"))
                emit(dtnBundleReceivedFromCom, true);
            if (msg->arrivedOn("gateToApp$i")) {
                emit(dtnBundleReceivedFromApp, true);
		// ToDo: figure out where to place arrival time metrics.
                // this->metricCollector_->intializeArrivalTime(bundle->getBundleId(), std::chrono::steady_clock::now());
            }
            handleBundle(check_and_cast<BundlePkt *>(msg));
            break;

        case FORWARDING_MSG_START:
            handleForwardingStart(check_and_cast<ForwardingMsgStart *>(msg));
            break;

	// ToDo: implement bundle custody!!!!!!!!!

        case ROUTING_RETRY:
            handleRoutingRetry();
            delete msg;
            break;

        default:
            EV << "Unhandled message type: " << msg->getKind() << endl;
    }
}

/*
 * Handles an inbound bundle.
 *
 * If the bundle's destination is the node's EID, the bundle gets dispatched to the application layer. Otherwise, it
 * gets routed and scheduled for transmission.
 */
void ContactlessDtn::handleBundle(BundlePkt *bundle) {
    if (eid_ != bundle->getDestinationEid()) {
	routing->msgToOtherArrive(bundle, simTime().dbl());
        scheduleBundle(bundle);
    } else {
        emit(dtnBundleSentToApp, true);
        emit(dtnBundleSentToAppHopCount, bundle->getHopCount());
        send(bundle, "gateToApp$o");
    }
}

/*
 * Handles a forwarding start message, which signal the opportunity to dispatch a new bundle towards the message's
 * destination EID.
 *
 * If no bundles are waiting to be dispatched to said EID, the link is freed. Otherwise, the awaiting bundle gets
 * re-routed to ensure that the previously calculated route continues being valid. If the bundle's route changes,
 * it gets either stored in the node's SDR or scheduled for transmission with the new, corresponding next hop EID. The
 * bundle may also be stored in the node's SDR if its transmission finishes after the next mobility update occurs,
 * as a topology change can change the bundle's would-be next hop's availability.
 *
 * If all checks succeed, the bundle gets sent to the Com layer to be sent towards its next hop, and the same
 * forwarding start message gets re-scheduled with a delay equal to the bundle's transmission duration, effectively
 * simulating transmission delays.
 */
void ContactlessDtn::handleForwardingStart(ForwardingMsgStart *fwd) {
    const int nextHop = fwd->getNeighborEid();

    if (!sdr_->isBundleForId(nextHop)) { // No bundles to route.
        fwdByEid_.erase(nextHop);
        delete fwd;
        return;
    }

    BundlePkt *bundle = sdr_->getBundle(nextHop);
    sdr_->popBundleFromId(nextHop);

    routing->msgToOtherArrive(bundle, simTime().dbl());
    if (nextHop != bundle->getNextHopEid()) { // While awaiting a transmission delay, satellite movement occurred.
	scheduleBundle(bundle);
        scheduleAt(simTime(), fwd);
        return;
    }

    // ToDo: compute transmission time
    // double txDuration = bundle->getByteLength() / dataRate;
    constexpr double txDuration = 0;

    if (simTime() + txDuration >= (*mobilityMap_)[eid_]->getNextUpdateTime()) {
	scheduleRoutingRetry(bundle);
        scheduleAt(simTime(), fwd);
	return;
    }

    std::cout << "Sending bundle " << std::dec << bundle->getBundleId() << " from " << eid_ << " to " << bundle->getNextHopEid() << std::endl;
    bundle->setHopCount(bundle->getHopCount() + 1);
    bundle->setSenderEid(eid_);

    send(bundle, "gateToCom$o");

    scheduleAt(simTime() + txDuration, fwd);
}

/*
 * Handles a routing retry message.
 *
 * A bundle gets popped from the node's SDR generic queue and gets immediately scheduled to be routed.
 */
void ContactlessDtn::handleRoutingRetry() {
    // ToDo: Loop through all stored bundles after this is implemented:
    //       - Only schedule one retry bundle to avoid flooding handler.

    const auto sdr = dynamic_cast<ContactlessSdrModel*>(sdr_);
    const auto bundle = sdr->popBundle();
    std::cout << "Popped bundle " << bundle->getBundleId() << " from SDR to retry routing." << std::endl;
    scheduleAt(simTime(), bundle); // Resend bundle to be handled alongisde transmission delay logic.
}

/*
 * Schedules a routed bundle to be dispatched.
 *
 * If its next hop is the same as the node's EID, meaning that no route was found for the bundle's destination, the
 * bundle gets stored into SDR and a routing retry message is scheduled for the next satellite movement update.
 *
 * Otherwise, the bundle gets stored into its next hop's queue, and a forwarding start message is generated, as long
 * as the link is not busy, to handle the bundle's dispatch. If the link is busy (meaning that there's another bundle
 * being transmitted towards the same next hop), it's assumed that the forwarding start message loop in handleMessage
 * will eventually provoke the dispatch of the enqueued bundle.
 */
void ContactlessDtn::scheduleBundle(BundlePkt *bundle) {
    const int nextHop = bundle->getNextHopEid();
    if (nextHop == eid_) {
        scheduleRoutingRetry(bundle);
        return;
    }

    sdr_->pushBundleToId(bundle, nextHop);

    if (fwdByEid_.find(nextHop) == fwdByEid_.end()) {
        auto *fwd = new ForwardingMsgStart("forwardingStart", FORWARDING_MSG_START);
        fwd->setNeighborEid(nextHop);
        fwdByEid_[nextHop] = fwd;
	scheduleAt(simTime(), fwd);
    }
}

/*
 * Schedules a routing retry message.
 *
 * An attempt to save the bundle into the node's SDR generic queue is made (since routing must not be successful for
 * this function to be called, there's no next hop with which to index the indexed queues of SDR). If space is not
 * enough, the bundle gets dropped and unhandled as it's expected for upper layers to detect and handle packet loss.
 *
 * If saving to SDR is successful, a routing retry message is scheduled for the next mobility update, as a topology
 * change may result in new paths being available for the bundle to reach its destination.
 */
void ContactlessDtn::scheduleRoutingRetry(BundlePkt *bundle) {
    if(!sdr_->pushBundle(bundle)) {
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR. Dropping bundle..." << std::endl;
        return;
    }

    // ToDo: only schedule one retry bundle to avoid flooding handler.

    const auto mobilityModule = (*mobilityMap_)[eid_];
    const auto retryBundle = new RoutingRetry("RoutingRetry", ROUTING_RETRY);

    // ToDo: that one-second policy seems arbitrary. Can we assume the bundle to be re-routed would be in SDR, signaling
    //		 that it shouldn't simply be dropped if the node is down?
    const auto scheduleTime = mobilityModule ? mobilityModule->getNextUpdateTime() : simTime() + 1; // if mobilityModule is null, node is down, schedule retry in 1 second
    std::cout << "Scheduling bundle retry... - Current time: " << simTime().dbl() <<  " - Scheduling time: " << scheduleTime << std::endl;

    scheduleAt(scheduleTime, retryBundle);
    // this->pendingBundles_.push_back(retryBundle);
}

void ContactlessDtn::setOnFault(bool onFault) {
    this->onFault = onFault;

    if (onFault){
        this->mobilityMap_->erase(eid_); // ToDo: why do we have to erase the mobility module when we already have a flag to know if the node is on fault?
    } else {
        auto* mobility = dynamic_cast<inet::SatelliteMobility*>(this->getParentModule()->getSubmodule("mobility"));
        (*this->mobilityMap_)[eid_] = mobility;
    }
}

void ContactlessDtn::setRoutingAlgorithm(Antop* antop) {
    this->antop = antop;
}