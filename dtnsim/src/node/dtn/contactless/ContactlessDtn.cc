#include "ContactlessDtn.h"

#include "../../../../../../omnetpp-6.1/include/omnetpp/clog.h"
#include "../../MsgTypes.h"
#include "../routing/RoutingAntop.h"
#include "src/node/app/App.h"

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

void ContactlessDtn::initializeRouting(const string& routingString) {
    this->sdr_.setEid(eid_);
    this->sdr_.setSize(par("sdrSize"));
    this->sdr_.setNodesNumber(this->getParentModule()->getParentModule()->par("nodesNumber"));

    if (routingString == "antop") {

    } else {
        cout << "dtnsim error: unknown routing type: " << routingString << endl;
        exit(1);
    }
}

void ContactlessDtn::finish() {
    // Last call to sample-hold type metrics
    if (eid_ != 0) {
        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());
    }

    // Delete scheduled forwardingMsg
    for (auto &[_, forwardingMsg] : forwardingMsgs_)
        cancelAndDelete(forwardingMsg);

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
    case BUNDLE:
    case BUNDLE_CUSTODY_REPORT:
        if (msg->arrivedOn("gateToCom$i"))
            emit(dtnBundleReceivedFromCom, true);
        if (msg->arrivedOn("gateToApp$i"))
            emit(dtnBundleReceivedFromApp, true);

        dispatchBundle(check_and_cast<BundlePkt *>(msg));
        break;
    case CUSTODY_TIMEOUT:
        // Custody timer expired, check if bundle still in custody memory space and retransmit it if positive.
        auto *custodyTimeout = check_and_cast<CustodyTimout *>(msg);

        if (BundlePkt *reSendBundle = this->custodyModel_.custodyTimerExpired(custodyTimeout); reSendBundle != nullptr)
            this->dispatchBundle(reSendBundle);
        delete custodyTimeout;
        break;
    default:
        std::cout << "Unable to handle message of type: " << msg->getKind() << std::endl;
        EV << "Unable to handle message of type: " << msg->getKind() << std::endl;
    }
}

void ContactlessDtn::dispatchBundle(BundlePkt *bundle) {
    // char bundleName[10];
    // sprintf(bundleName, "Src:%d,Dst:%d(id:%d)", bundle->getSourceEid() ,
    // bundle->getDestinationEid(), (int) bundle->getId()); cout << "Dispatching " << bundleName <<
    // endl;

    if (this->eid_ == bundle->getDestinationEid()) {
        // We are the final destination of this bundle
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
    } else {
        // This is a bundle in transit

        // Manage custody transfer
        if (bundle->getCustodyTransferRequested())
            this->dispatchBundle(this->custodyModel_.bundleWithCustodyRequestedArrived(bundle));

        // Either accepted or rejected custody, route bundle
        routing->msgToOtherArrive(bundle, simTime().dbl());

        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());

        // Wake-up sleeping forwarding threads
        this->refreshForwarding();
    }
}

void ContactlessDtn::refreshForwarding() {
    // Check all ongoing forwardingMsgs threads (contacts) and wake up those not scheduled.
    for (auto &[_, forwardingMsg] : forwardingMsgs_) {
        if (const int cid = forwardingMsg->getContactId(); !sdr_.isBundleForId(cid))
            // Notify routing protocol that it has messages to send and contacts for routing
            routing->refreshForwarding(contactTopology_.getContactById(cid));
        if (!forwardingMsg->isScheduled())
            scheduleAt(simTime(), forwardingMsg);
    }
}

void ContactlessDtn::setOnFault(bool onFault) {
    /*this->onFault = onFault;

    // Local and remote forwarding recovery
    if (onFault == false) {
        // Wake-up local un-scheduled forwarding threads
        this->refreshForwarding();

        // Wake-up remote un-scheduled forwarding threads
        for (auto &[_, forwardingMsg] : forwardingMsgs_) {
            const auto remoteContactlessDtn = dynamic_cast<ContactlessDtn *>(this->getParentModule()
                                       ->getParentModule()
                                       ->getSubmodule("node", forwardingMsg->getNeighborEid())
                                       ->getSubmodule("dtn"));
            remoteContactlessDtn->refreshForwarding();
        }
    }*/
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
