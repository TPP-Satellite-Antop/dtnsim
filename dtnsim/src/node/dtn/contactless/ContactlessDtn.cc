#include "ContactlessDtn.h"

#include "../../../../../../omnetpp-6.1/include/omnetpp/clog.h"
#include "../../MsgTypes.h"
#include "../routing/RoutingAntop.h"
#include "src/node/app/App.h"

#include <iostream>

Define_Module(ContactlessDtn);

ContactlessDtn::ContactlessDtn() {}

ContactlessDtn::~ContactlessDtn() = default;

void ContactlessDtn::setMetricCollector(MetricCollector *metricCollector) {
    this->metricCollector_ = metricCollector;
}

int ContactlessDtn::numInitStages() const {
    return 2;
}

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
        // Store this node eid
        this->eid_ = this->getParentModule()->getIndex();

        this->custodyTimeout_ = par("custodyTimeout");
        this->custodyModel_.setEid(eid_);
        this->custodyModel_.setSdr(&sdr_);
        this->custodyModel_.setCustodyReportByteSize(par("custodyReportByteSize"));

        // Get a pointer to graphics module
        graphicsModule = dynamic_cast<Graphics *>(this->getParentModule()->getSubmodule("graphics"));
        // Register this object as sdr observer, in order to display bundles stored in sdr properly.
        sdr_.addObserver(this);
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
            emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
            emit(sdrBytesStored, sdr_.getBytesStoredInSdr());
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

void ContactlessDtn::setMobilityMap(map<int, SatSGP4Mobility*> *mobilityMap) {
    this->mobilityMap_ = mobilityMap;
}

void ContactlessDtn::initializeRouting(string routingString) {
    this->sdr_.setEid(eid_);
    this->sdr_.setSize(par("sdrSize"));
    this->sdr_.setNodesNumber(this->getParentModule()->getParentModule()->par("nodesNumber"));

    if (routingString == "antop") {
        SatSGP4Mobility* mobility = dynamic_cast<SatSGP4Mobility*>(this->getParentModule()->getSubmodule("mobility"));
        (*this->mobilityMap_)[eid_] = mobility;
        this->routing = new RoutingAntop(this->antop_, this->eid_, &sdr_, mobilityMap_);
    } else {
        cout << "dtnsim error: unknown routing type: " << routingString << endl;
    }
}

void ContactlessDtn::finish() {
    // Last call to sample-hold type metrics
    if (eid_ != 0) {
        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());
    }

    // Delete all stored bundles
    sdr_.freeSdr();

    // BundleMap End
    if (saveBundleMap_)
        bundleMap_.close();

    delete routing;
}

/**
 * Reacts to a system message.
 *
 * @param: msg: A pointer to the received message
 *
 * @authors The original implementation was done by the authors of DTNSim and then modified by Simon
 * Rink
 */

void ContactlessDtn::handleMessage(cMessage *msg) {
    ///////////////////////////////////////////
    // New Bundle (from App or Com):
    ///////////////////////////////////////////
    switch (msg->getKind()) {
        case BUNDLE: {}
        case BUNDLE_CUSTODY_REPORT: {
            if (msg->arrivedOn("gateToCom$i"))
                emit(dtnBundleReceivedFromCom, true);
            if (msg->arrivedOn("gateToApp$i"))
                emit(dtnBundleReceivedFromApp, true);

            dispatchBundle(check_and_cast<BundlePkt *>(msg));
            break;
        }
        case CUSTODY_TIMEOUT: {
            // Custody timer expired, check if bundle still in custody memory space and retransmit it if positive.
            auto *custodyTimeout = check_and_cast<CustodyTimout *>(msg);

            if (BundlePkt *reSendBundle = this->custodyModel_.custodyTimerExpired(custodyTimeout); reSendBundle != nullptr)
                this->dispatchBundle(reSendBundle);
            delete custodyTimeout;
            break;
        }
        default: {
            std::cout << "Unable to handle message of type: " << msg->getKind() << std::endl;
            EV << "Unable to handle message of type: " << msg->getKind() << std::endl;
            break;
        }
    }
}

void ContactlessDtn::dispatchBundle(BundlePkt *bundle) {
    if (this->eid_ == bundle->getDestinationEid()) { // We are the final destination of this bundle
        emit(dtnBundleSentToApp, true);
        emit(dtnBundleSentToAppHopCount, bundle->getHopCount());
        bundle->getVisitedNodesForUpdate().sort();
        bundle->getVisitedNodesForUpdate().unique();
        emit(dtnBundleSentToAppRevisitedHops, bundle->getHopCount() - bundle->getVisitedNodes().size());

        // Check if this bundle has previously arrived here
        if (routing->msgToMeArrive(bundle)) {
            // This is the first time this bundle arrives
            if (bundle->getBundleIsCustodyReport()) {
                // This is a custody report destined to me

                // If custody was rejected, reroute
                if (BundlePkt *reSendBundle = this->custodyModel_.custodyReportArrived(bundle);
                    reSendBundle != nullptr)
                    this->dispatchBundle(reSendBundle);
            } else {
                // This is a data bundle destined to me
                if (bundle->getCustodyTransferRequested())
                    this->dispatchBundle(this->custodyModel_.bundleWithCustodyRequestedArrived(bundle));

                // Send to app layer
                send(bundle, "gateToApp$o");
            }
        } else
            // A copy of this bundle was previously received
            delete bundle;
    } else { // This is a bundle in transit
        // Manage custody transfer
        if (bundle->getCustodyTransferRequested())
            this->dispatchBundle(this->custodyModel_.bundleWithCustodyRequestedArrived(bundle));

        // Either accepted or rejected custody, route bundle
        routing->msgToOtherArrive(bundle, simTime().dbl());

        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());

        if (sdr_.enqueueBundleError())
            delete bundle; //TODO o que?
        else if (sdr_.enqueuedBundle()) {
            std::cout << "Node " << eid_ 
                    << " --- Bundle " << bundle->getBundleId() 
                    << " enqueued in SDR ---" << std::endl;
            //TODO send new msg
        } else 
            sendMsg(bundle);

        sdr_.resetEnqueuedBundleFlags();
    }
}

void ContactlessDtn::sendMsg(BundlePkt *bundle) {
    const int neighborEid = bundle->getNextHopEid();
    const auto neighborContactDtn = check_and_cast<ContactlessDtn *>(this
        ->getParentModule()
        ->getParentModule()
        ->getSubmodule("node", neighborEid)
        ->getSubmodule("dtn")
    );

    // ToDo: handle fault tolerance
    // if ((!neighborContactDtn->onFault) && (!this->onFault)) {

    // ToDo: calculate data rate and Tx duration
    /*double dataRate = contactTopology_.getContactById(contactId)->getDataRate();
    double txDuration = static_cast<double>(bundle->getByteLength()) / dataRate;
    double linkDelay = contactTopology_.getRangeBySrcDst(eid_, neighborEid);*/

    // ToDo: if the message can be fully transmitted before the end of the contact, transmit it
    // if ((simTime() + txDuration + linkDelay) <= contact->getEnd()) {

    // Set bundle metadata (set by intermediate nodes)
    bundle->setSenderEid(eid_);
    bundle->setHopCount(bundle->getHopCount() + 1);
    bundle->setXmitCopiesCount(0);

    std::cout << "Node " << eid_ << " --- Sending bundle to --> Node "<< bundle->getNextHopEid() << std::endl;
    send(bundle, "gateToCom$o");

    // If custody requested, store a copy of the bundle until report received
    if (bundle->getCustodyTransferRequested()) {
        sdr_.enqueueTransmittedBundleInCustody(bundle->dup());
        this->custodyModel_.printBundlesInCustody();

        // Enqueue a retransmission event in case custody acceptance not received
        auto *custodyTimeout = new CustodyTimout("custodyTimeout", CUSTODY_TIMEOUT);
        custodyTimeout->setBundleId(bundle->getBundleId());
        scheduleAt(simTime() + this->custodyTimeout_, custodyTimeout);
    }

    emit(dtnBundleSentToCom, true);
    emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
    emit(sdrBytesStored, sdr_.getBytesStoredInSdr());
}

void ContactlessDtn::setOnFault(bool onFault) {
}

Routing *ContactlessDtn::getRouting() {
    return this->routing;
}

/**
 * Implementation of method inherited from observer to update gui according to the number of
 * bundles stored in sdr.
 */
void ContactlessDtn::update() {
    // Update srd size text
    graphicsModule->setBundlesInSdr(sdr_.getBundlesCountInSdr());
}

void ContactlessDtn::setRoutingAlgorithm(Antop* antop) {
    this->antop_ = antop;
}
