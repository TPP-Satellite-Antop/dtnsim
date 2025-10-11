#include "ContactDtn.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/contactplan/ContactHistory.h"
#include "src/node/dtn/contactplan/Contact.h"
#include "src/node/app/App.h"
#include "src/node/dtn/routing/RoutingAntop.h"
#include "src/node/dtn/routing/RoutingBRUF1T.h"
#include "src/node/dtn/routing/RoutingCgrModel350.h"
#include "src/node/dtn/routing/RoutingCgrModel350_2Copies.h"
#include "src/node/dtn/routing/RoutingCgrModel350_Hops.h"
#include "src/node/dtn/routing/RoutingCgrModel350_Probabilistic.h"
#include "src/node/dtn/routing/RoutingCgrModelRev17.h"
#include "src/node/dtn/routing/RoutingCgrModelYen.h"
#include "src/node/dtn/routing/RoutingDirect.h"
#include "src/node/dtn/routing/RoutingEpidemic.h"
#include "src/node/dtn/routing/RoutingPRoPHET.h"
#include "src/node/dtn/routing/RoutingSprayAndWait.h"
#include "src/node/dtn/routing/RoutingUncertainUniboCgr.h"
#include "src/node/dtn/routing/brufncopies/RoutingBRUFNCopies.h"
#include "src/node/dtn/routing/cgrbrufpowered/CGRBRUFPowered.h"

Define_Module(ContactDtn);

ContactDtn::ContactDtn() {}

ContactDtn::~ContactDtn() = default;

void ContactDtn::setContactPlan(ContactPlan &contactPlan) {
    this->contactPlan_ = contactPlan;
}

void ContactDtn::setContactTopology(ContactPlan &contactTopology) {
    this->contactTopology_ = contactTopology;
}

void ContactDtn::setMetricCollector(MetricCollector *metricCollector) {
    this->metricCollector_ = metricCollector;
}

int ContactDtn::numInitStages() const {
    return 2;
}

/**
 * Initializes the ContactDtn object
 *
 * @param stage: the stage for the dtn object
 *
 * @authors The original implementation was done by the authors of DTNSim and then modified by Simon
 * Rink
 */
void ContactDtn::initialize(const int stage) {
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

        // Create (empty) contact history
        this->contactHistory_ = ContactHistory();

        // Schedule local starts contact messages.
        // Only contactTopology start contacts are scheduled.
        const vector<Contact> localContacts1 = contactTopology_.getContactsBySrc(this->eid_);
        for (auto & c : localContacts1) {
            ContactMsg *contactMsgStart;

            if (c.isDiscovered()) {
                contactMsgStart = new ContactMsg("discContactStart", DISC_CONTACT_START_TIMER);
                contactMsgStart->setSchedulingPriority(DISC_CONTACT_START_TIMER);
            } else {
                contactMsgStart = new ContactMsg("ContactStart", CONTACT_START_TIMER);
                contactMsgStart->setSchedulingPriority(CONTACT_START_TIMER);
            }

            contactMsgStart->setId(c.getId());
            contactMsgStart->setStart(c.getStart());
            contactMsgStart->setEnd(c.getEnd());
            contactMsgStart->setDuration(c.getEnd() - c.getStart());
            contactMsgStart->setSourceEid(c.getSourceEid());
            contactMsgStart->setDestinationEid(c.getDestinationEid());
            contactMsgStart->setDataRate(c.getDataRate());

            scheduleAt(c.getStart(), contactMsgStart);

            EV << "node " << eid_ << ": "
               << "a contact +" << c.getStart() << " +" << c.getEnd() << " "
               << c.getSourceEid() << " " << c.getDestinationEid() << " "
               << c.getDataRate() << endl;
        }
        // Schedule local ends contact messages.
        // All ends contacts of the contactTopology are scheduled.
        // to trigger re-routings of bundles queued in contacts that did not happen.
        const vector<Contact> localContacts2 = contactTopology_.getContactsBySrc(this->eid_);
        for (auto & c : localContacts2) {
            ContactMsg *contactMsgEnd;

            if (c.isDiscovered()) {
                contactMsgEnd = new ContactMsg("discContactEnd", DISC_CONTACT_END_TIMER);
                contactMsgEnd->setSchedulingPriority(DISC_CONTACT_END_TIMER);
                contactMsgEnd->setName("discContactEnd");
            } else {
                contactMsgEnd = new ContactMsg("ContactEnd", CONTACT_END_TIMER);
                contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
                contactMsgEnd->setName("ContactEnd");
            }

            contactMsgEnd->setId(c.getId());
            contactMsgEnd->setStart(c.getStart());
            contactMsgEnd->setEnd(c.getEnd());
            contactMsgEnd->setDuration(c.getEnd() - c.getStart());
            contactMsgEnd->setSourceEid(c.getSourceEid());
            contactMsgEnd->setDestinationEid(c.getDestinationEid());
            contactMsgEnd->setDataRate(c.getDataRate());

            scheduleAt(c.getStart() + c.getDuration(), contactMsgEnd);
        }

        const string routeString = par("routing");

        if (routeString == "uncertainUniboCgr") { // only done for (O)CGR-UCoP
            const vector<Contact> localContacts3 = contactPlan_.getContactsBySrc(this->eid_);
            for (auto & it : localContacts3) {
                if (this->checkExistenceOfContact(it.getSourceEid(), it.getDestinationEid(), it.getStart()) == 0) { // identify failed contacts
                    auto *failedMsg = new ContactMsg("contactFailed", CONTACT_FAILED);
                    failedMsg->setSchedulingPriority(CONTACT_FAILED);
                    failedMsg->setName("failedMsg");
                    failedMsg->setId(it.getId());

                    scheduleAt(it.getEnd(), failedMsg);
                }
            }
        }

        // Initialize routing
        this->initializeRouting(routeString);

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
        routeCgrDijkstraCalls = registerSignal("routeCgrDijkstraCalls");
        routeCgrDijkstraLoops = registerSignal("routeCgrDijkstraLoops");
        routeCgrRouteTableEntriesCreated = registerSignal("routeCgrRouteTableEntriesCreated");
        routeCgrRouteTableEntriesExplored = registerSignal("routeCgrRouteTableEntriesExplored");

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

void ContactDtn::initializeRouting(const string& routingString) {
    this->sdr_.setEid(eid_);
    this->sdr_.setSize(par("sdrSize"));
    this->sdr_.setNodesNumber(this->getParentModule()->getParentModule()->par("nodesNumber"));
    this->sdr_.setContactPlan(&contactTopology_);

    if (routingString == "direct")
        routing = new RoutingDirect(eid_, &sdr_, &contactPlan_);
    else if (routingString == "cgrModel350")
        routing = new RoutingCgrModel350(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routingString == "cgrModel350_Hops")
        routing = new RoutingCgrModel350_Hops(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routingString == "cgrModelYen")
        routing = new RoutingCgrModelYen(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routingString == "cgrModelRev17") {
        ContactPlan *globalContactPlan = dynamic_cast<ContactDtn*>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule("dtn")
        )->getContactPlanPointer();

        routing = new RoutingCgrModelRev17(eid_, this->getParentModule()->getVectorSize(), &sdr_,
                                           &contactPlan_, globalContactPlan, par("routingType"),
                                           par("printRoutingDebug"));
    } else if (routingString == "epidemic") {
        routing = new RoutingEpidemic(eid_, &sdr_, this);
    } else if (routingString == "sprayAndWait") {
        int bundlesCopies = par("bundlesCopies");
        routing = new RoutingSprayAndWait(eid_, &sdr_, this, bundlesCopies, false,
                                          this->metricCollector_);
    } else if (routingString == "binarySprayAndWait") {
        int bundlesCopies = par("bundlesCopies");
        routing =
            new RoutingSprayAndWait(eid_, &sdr_, this, bundlesCopies, true, this->metricCollector_);
    } else if (routingString == "PRoPHET") {
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        double p_enc_max = par("pEncouterMax");
        double p_enc_first = par("pEncouterFirst");
        double p_first_thresh = par("pFirstThreshold");
        double forw_tresh = par("ForwThresh");
        double alpha = par("alpha");
        double beta = par("beta");
        double gamma = par("gamma");
        double delta = par("delta");
        routing = new RoutingPRoPHET(eid_, &sdr_, this, p_enc_max, p_enc_first, p_first_thresh,
                                     forw_tresh, alpha, beta, gamma, delta, numOfNodes,
                                     this->metricCollector_);
    } else if (routingString == "cgrModel350_2Copies")
        routing = new RoutingCgrModel350_2Copies(eid_, &sdr_, &contactPlan_,
                                                 par("printRoutingDebug"), this);
    else if (routingString == "cgrModel350_Probabilistic") {
        double sContactProb = par("sContactProb");
        routing = new RoutingCgrModel350_Probabilistic(
            eid_, &sdr_, &contactPlan_, par("printRoutingDebug"), this, sContactProb);
    } else if (routingString == "uncertainUniboCgr") {
        bool useUncertainty =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par(
                "useUncertainty");
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing =
            new RoutingUncertainUniboCgr(eid_, &sdr_, &contactPlan_, this, this->metricCollector_,
                                         -1, useUncertainty, repetition, numOfNodes);
    } else if (routingString == "uniboCgr") {
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing =
            new RoutingUncertainUniboCgr(eid_, &sdr_, &contactPlan_, this, this->metricCollector_,
                                         -1, false, repetition, numOfNodes);
    } else if (routingString == "ORUCOP") {
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing = new RoutingORUCOP(eid_, &sdr_, &contactPlan_, this, this->metricCollector_, 2,
                                    repetition, numOfNodes);
    } // 2 bundles for now.
    else if (routingString == "BRUF1T") {
        string frouting = par("frouting");
        routing = new RoutingBRUF1T(eid_, &sdr_, &contactPlan_, frouting);
    } else if (routingString == "BRUFNCopies") {
        string frouting = par("frouting");
        int bundlesCopies = par("bundlesCopies");
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        double pf = this->getParentModule()->getParentModule()->getSubmodule("central")->par(
            "failureProbability");
        pf = -1.00;

        ostringstream prefix;
        prefix << frouting << "pf=" << fixed << setprecision(2) << pf << "/todtnsim-";
        ostringstream posfix;
        posfix << "-" << fixed << setprecision(2) << pf << ".json";

        routing = new RoutingBRUFNCopies(eid_, &sdr_, &contactPlan_, bundlesCopies, numOfNodes,
                                         prefix.str(), posfix.str());
    } else if (routingString == "CGR_BRUFPowered") {
        string frouting = par("frouting");
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        double pf = this->getParentModule()->getParentModule()->getSubmodule("central")->par(
            "failureProbability");
        int ts_duration = par("ts_duration");

        // Parse parameter ts_start_times
        const char *str_ts_start_times = par("ts_start_times");
        cStringTokenizer ts_start_Tokenizer(str_ts_start_times, ",");
        std::vector<int> ts_start_times;
        while (ts_start_Tokenizer.hasMoreTokens())
            ts_start_times.push_back(atoi(ts_start_Tokenizer.nextToken()));

        ostringstream prefix;
        prefix << frouting << "pf=" << fixed << setprecision(2) << pf << "/todtnsim-";
        ostringstream posfix;
        posfix << "-" << fixed << setprecision(2) << pf << ".json";

        routing = new CGRBRUFPowered(
            eid_, &sdr_, &contactPlan_, par("printRoutingDebug"), pf,
            ts_duration, ts_start_times, numOfNodes, prefix.str(), posfix.str()
        );
    } else {
        cout << "dtnsim error: unknown routing type: " << routingString << endl;
        exit(1);
    }
}

void ContactDtn::finish() {
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

void ContactDtn::handleMessage(cMessage *msg) {
    ///////////////////////////////////////////
    // New Bundle (from App or ContactPlanCom):
    ///////////////////////////////////////////
    if (msg->getKind() == BUNDLE || msg->getKind() == BUNDLE_CUSTODY_REPORT) {
        if (msg->arrivedOn("gateToCom$i"))
            emit(dtnBundleReceivedFromCom, true);
        if (msg->arrivedOn("gateToApp$i"))
            emit(dtnBundleReceivedFromApp, true);

        auto *bundle = check_and_cast<BundlePkt *>(msg);
        dispatchBundle(bundle);
    } else if (msg->getKind() == CONTACT_FAILED) { // A failed contact was noticed!
        const auto *contactMsg = check_and_cast<ContactMsg *>(msg);

        auto *uniboRouting = check_and_cast<RoutingUncertainUniboCgr *>(this->routing);
        uniboRouting->contactFailure(contactMsg->getId()); // reroute all failed bundles!

        this->refreshForwarding();

        delete contactMsg;
    }

    ///////////////////////////////////////////
    // Contact Start and End
    ///////////////////////////////////////////
    else if (msg->getKind() == DISC_CONTACT_START_TIMER) { // Discovered contact was found
        const auto *contactMsg = check_and_cast<ContactMsg *>(msg);

        const auto controller = check_and_cast<ContactDtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        (*controller)
            .syncDiscoveredContact(contactTopology_.getContactById(contactMsg->getId()), true);

        delete contactMsg;

    } else if (msg->getKind() == DISC_CONTACT_END_TIMER) { // Discovered contact ended
        const auto *contactMsg = check_and_cast<ContactMsg *>(msg);
        const auto controller = check_and_cast<ContactDtn *>(this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule("dtn"));

        controller->syncDiscoveredContact(contactTopology_.getContactById(contactMsg->getId()), false);

        delete contactMsg;
    } else if (msg->getKind() == CONTACT_START_TIMER) { // Schedule end of contact
        auto *contactMsg = check_and_cast<ContactMsg *>(msg);

        Contact *contact = contactTopology_.getContactById(contactMsg->getId());

        const auto controller = check_and_cast<ContactDtn *>(this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule("dtn"));

        // For opportunistic extensions
        controller->coordinateContactStart(contact);

        // Visualize contact line on
        graphicsModule->setContactOn(contactMsg);

        // Call to routing algorithm
        routing->contactStart(contact);

        // Schedule start of transmission
        auto *forwardingMsg = new ForwardingMsgStart("forwardingMsgStart", FORWARDING_MSG_START);
        forwardingMsg->setSchedulingPriority(FORWARDING_MSG_START);
        forwardingMsg->setNeighborEid(contactMsg->getDestinationEid());
        forwardingMsg->setContactId(contactMsg->getId());
        forwardingMsgs_[contactMsg->getId()] = forwardingMsg;
        scheduleAt(simTime(), forwardingMsg);

        delete contactMsg;
    } else if (msg->getKind() == CONTACT_END_TIMER) {
        // Finish transmission: If bundles are left in contact re-route them
        auto *contactMsg = check_and_cast<ContactMsg *>(msg);

        Contact *contact = contactTopology_.getContactById(contactMsg->getId());

        const ContactDtn *controller = check_and_cast<ContactDtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        // for opportunistic extensions
        controller->coordinateContactEnd(contact);

        for (int i = 0; i < sdr_.getBundlesCountInIndex(contactMsg->getId()); i++)
            emit(dtnBundleReRouted, true);

        routing->contactEnd(contactTopology_.getContactById(contactMsg->getId()));

        // Visualize contact line off
        graphicsModule->setContactOff(contactMsg);

        // Delete contactMsg
        cancelAndDelete(forwardingMsgs_[contactMsg->getId()]);
        forwardingMsgs_.erase(contactMsg->getId());
        delete contactMsg;
    }

    ///////////////////////////////////////////
    // Forwarding Stage
    ///////////////////////////////////////////
    else if (msg->getKind() == FORWARDING_MSG_START) {
        auto *forwardingMsgStart = check_and_cast<ForwardingMsgStart *>(msg);
        const int neighborEid = forwardingMsgStart->getNeighborEid();
        const int contactId = forwardingMsgStart->getContactId();

        // save freeChannelMsg to cancel event if necessary
        forwardingMsgs_[forwardingMsgStart->getContactId()] = forwardingMsgStart;

        // if there are messages in the queue for this contact
        if (sdr_.isBundleForId(contactId)) {
            // If local/remote node are responsive, then transmit bundle
            const auto neighborContactDtn = check_and_cast<ContactDtn *>(this->getParentModule()
                                                         ->getParentModule()
                                                         ->getSubmodule("node", neighborEid)
                                                         ->getSubmodule("dtn"));
            if ((!neighborContactDtn->onFault) && (!this->onFault)) {
                // Get bundle pointer from sdr
                BundlePkt *bundle = sdr_.getBundle(contactId);

                // Calculate data rate and Tx duration
                double dataRate = contactTopology_.getContactById(contactId)->getDataRate();
                double txDuration = static_cast<double>(bundle->getByteLength()) / dataRate;
                double linkDelay = contactTopology_.getRangeBySrcDst(eid_, neighborEid);

                Contact *contact = contactTopology_.getContactById(contactId);

                // if the message can be fully transmitted before the end of the contact, transmit
                // it
                if ((simTime() + txDuration + linkDelay) <= contact->getEnd()) {
                    // Set bundle metadata (set by intermediate nodes)
                    bundle->setSenderEid(eid_);
                    bundle->setHopCount(bundle->getHopCount() + 1);
                    bundle->getVisitedNodesForUpdate().push_back(eid_);
                    bundle->setXmitCopiesCount(0);

                    // cout<<"-----> sending bundle to node "<bundle->getNextHopEid()<<endl;
                    send(bundle, "gateToCom$o");

                    if (saveBundleMap_)
                        bundleMap_ << simTime() << "," << eid_ << "," << neighborEid << ","
                                   << bundle->getSourceEid() << "," << bundle->getDestinationEid()
                                   << "," << bundle->getBitLength() << "," << txDuration << endl;

                    sdr_.popBundleFromId(contactId);

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

                    // Schedule next transmission
                    scheduleAt(simTime() + txDuration, forwardingMsgStart);

                    // Schedule forwarding message end
                    auto *forwardingMsgEnd = new ForwardingMsgEnd("forwardingMsgEnd", FORWARDING_MSG_END);
                    forwardingMsgEnd->setSchedulingPriority(FORWARDING_MSG_END);
                    forwardingMsgEnd->setNeighborEid(neighborEid);
                    forwardingMsgEnd->setContactId(contactId);
                    forwardingMsgEnd->setBundleId(bundle->getBundleId());
                    forwardingMsgEnd->setSentToDestination(neighborEid == bundle->getDestinationEid());
                    scheduleAt(simTime() + txDuration, forwardingMsgEnd);
                }
            } else {
                // If local/remote node unresponsive, then do nothing.
                // fault recovery will trigger a local and remote refreshForwarding
            }
        } else {
            // There are no messages in the queue for this contact
            // Do nothing, if new data arrives, a refreshForwarding
            // will wake up this forwarding thread
        }
    } else if (msg->getKind() == FORWARDING_MSG_END) {
        // A bundle was successfully forwarded. Notify routing schema in order to it makes proper
        // decisions.
        const auto *forwardingMsgEnd = check_and_cast<ForwardingMsgEnd *>(msg);
        const int bundleId = forwardingMsgEnd->getBundleId();
        const int contactId = forwardingMsgEnd->getContactId();

        Contact *contact = contactTopology_.getContactById(contactId);
        routing->successfulBundleForwarded(bundleId, contact, forwardingMsgEnd->getSentToDestination());

        delete forwardingMsgEnd;
    }
    ///////////////////////////////////////////
    // Custody retransmission timer
    ///////////////////////////////////////////
    else if (msg->getKind() == CUSTODY_TIMEOUT) {
        // Custody timer expired, check if bundle still in custody memory space and retransmit it if positive.
        auto *custodyTimeout = check_and_cast<CustodyTimout *>(msg);

        if (BundlePkt *reSendBundle = this->custodyModel_.custodyTimerExpired(custodyTimeout); reSendBundle != nullptr)
            this->dispatchBundle(reSendBundle);
        delete custodyTimeout;
    }
}

void ContactDtn::dispatchBundle(BundlePkt *bundle) {
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
        emit(dtnBundleSentToAppRevisitedHops,
             bundle->getHopCount() - bundle->getVisitedNodes().size());

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
                    this->dispatchBundle(
                        this->custodyModel_.bundleWithCustodyRequestedArrived(bundle));

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

        // Emit routing specific statistics
        // TODO
        const string routeString = par("routing");
        if (routeString == "cgrModel350") {
            emit(routeCgrDijkstraCalls, dynamic_cast<RoutingCgrModel350 *>(routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, dynamic_cast<RoutingCgrModel350 *>(routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesExplored, dynamic_cast<RoutingCgrModel350 *>(routing)->getRouteTableEntriesExplored());
        }
        if (routeString == "cgrModel350_3") {
            emit(routeCgrDijkstraCalls, dynamic_cast<RoutingCgrModel350_Hops *>(routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, dynamic_cast<RoutingCgrModel350_Hops *>(routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesExplored, dynamic_cast<RoutingCgrModel350_Hops *>(routing)->getRouteTableEntriesExplored());
        }
        if (routeString == "cgrModelRev17") {
            emit(routeCgrDijkstraCalls, dynamic_cast<RoutingCgrModelRev17 *>(routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, dynamic_cast<RoutingCgrModelRev17 *>(routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesCreated, dynamic_cast<RoutingCgrModelRev17 *>(routing)->getRouteTableEntriesCreated());
            emit(routeCgrRouteTableEntriesExplored, dynamic_cast<RoutingCgrModelRev17 *>(routing)->getRouteTableEntriesExplored());
        }
        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());

        // Wake-up sleeping forwarding threads
        this->refreshForwarding();
    }
}

void ContactDtn::refreshForwarding() {
    // Check all ongoing forwardingMsgs threads (contacts) and wake up those not scheduled.
    for (auto &[_, forwardingMsg] : forwardingMsgs_) {
        if (const int cid = forwardingMsg->getContactId(); !sdr_.isBundleForId(cid))
            // Notify routing protocol that it has messages to send and contacts for routing
            routing->refreshForwarding(contactTopology_.getContactById(cid));
        if (!forwardingMsg->isScheduled())
            scheduleAt(simTime(), forwardingMsg);
    }
}

void ContactDtn::setOnFault(bool onFault) {
    this->onFault = onFault;

    // Local and remote forwarding recovery
    if (onFault == false) {
        // Wake-up local un-scheduled forwarding threads
        this->refreshForwarding();

        // Wake-up remote un-scheduled forwarding threads
        for (auto &[_, forwardingMsg] : forwardingMsgs_) {
            const auto remoteContactDtn = dynamic_cast<ContactDtn *>(this->getParentModule()
                                       ->getParentModule()
                                       ->getSubmodule("node", forwardingMsg->getNeighborEid())
                                       ->getSubmodule("dtn"));
            remoteContactDtn->refreshForwarding();
        }
    }
}

ContactPlan *ContactDtn::getContactPlanPointer() {
    return &this->contactPlan_;
}

Routing *ContactDtn::getRouting() {
    return this->routing;
}

/**
 * Implementation of method inherited from observer to update gui according to the number of
 * bundles stored in sdr.
 */
void ContactDtn::update() {
    // Update srd size text
    graphicsModule->setBundlesInSdr(sdr_.getBundlesCountInSdr());
}

// PROCEDURES FOR OPPORTUNISTIC ROUTING!

/**
 * Schedules the contact start or end for a discovered contact
 *
 * @param c: the started/ended discovered contact.
 * @param start: boolean whether the contact started or ended.
 *
 * @author Simon Rink
 */
void ContactDtn::syncDiscoveredContact(Contact *c, bool start) const {
    // only controller node is allowed to decide on final topology
    if (this->eid_ != 0) {
        throw invalid_argument("Illegal controller call");
    }

    if (start) {
        // Schedule start of contact for sender
        const int sourceEid = c->getSourceEid();
        const auto source = check_and_cast<ContactDtn *>(
            this->getParentModule()
                  ->getParentModule()
                  ->getSubmodule("node", sourceEid)
                  ->getSubmodule("dtn")
        );
        source->scheduleDiscoveredContactStart(c);

    } else {
        // Schedule end of contact for sender
        const int sourceEid = (*c).getSourceEid();
        const auto source = check_and_cast<ContactDtn *>(
            this->getParentModule()
                  ->getParentModule()
                  ->getSubmodule("node", sourceEid)
                  ->getSubmodule("dtn")
        );
        source->scheduleDiscoveredContactEnd(c);
    }
}

/**
 * Adds or removes a discovered contact from a neighbor
 *
 * @param c: the contact to be added/removed.
 * @param start: boolean whether the contact started or ended.
 *
 * @author Simon Rink
 */
void ContactDtn::syncDiscoveredContactFromNeighbor(const Contact *c, const bool start, int ownEid, int neighborEid) const {
    if (this->eid_ != 0)
        throw invalid_argument("Illegal controller call");

    const auto neighbor = check_and_cast<ContactDtn *>(
        this->getParentModule()
              ->getParentModule()
              ->getSubmodule("node", neighborEid)
              ->getSubmodule("dtn")
    );

    if (start) // Add discovered contact into the contact plan of the neighbor and inform its neighbors
        neighbor->addDiscoveredContact(*c);
    else // Remove discovered contact from the contact plan of the neighbor and inform its neighbors
        neighbor->removeDiscoveredContact(*c);
}

/**
 * Schedules the start of a discovered contact
 *
 * @param c: The contact to be started
 *
 *
 * @author Simon Rink
 */
void ContactDtn::scheduleDiscoveredContactStart(Contact *c) {
    // schedule a new message for the actual contact start
    auto *contactMsgStart = new ContactMsg("ContactStart", CONTACT_START_TIMER);
    contactMsgStart->setSchedulingPriority(CONTACT_START_TIMER);
    contactMsgStart->setId((*c).getId());
    contactMsgStart->setStart((*c).getStart());
    contactMsgStart->setEnd((*c).getEnd());
    contactMsgStart->setDuration((*c).getEnd() - (*c).getStart());
    contactMsgStart->setSourceEid((*c).getSourceEid());
    contactMsgStart->setDestinationEid((*c).getDestinationEid());
    contactMsgStart->setDataRate((*c).getDataRate());

    scheduleAt(simTime(), contactMsgStart);
}

/**
 * Schedules the end of a discovered contact
 *
 * @param c: The contact to be ended
 *
 *
 * @author Simon Rink
 */
void ContactDtn::scheduleDiscoveredContactEnd(Contact *c) {
    // schedule a new message for the actual contact end
    auto *contactMsgEnd = new ContactMsg("ContactEnd", CONTACT_END_TIMER);
    contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
    contactMsgEnd->setName("ContactEnd");
    contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
    contactMsgEnd->setId(c->getId());
    contactMsgEnd->setStart(c->getStart());
    contactMsgEnd->setEnd(c->getEnd());
    contactMsgEnd->setDuration(c->getEnd() - c->getStart());
    contactMsgEnd->setSourceEid(c->getSourceEid());
    contactMsgEnd->setDestinationEid(c->getDestinationEid());
    contactMsgEnd->setDataRate(c->getDataRate());

    scheduleAt(simTime(), contactMsgEnd);
}

ContactHistory *ContactDtn::getContactHistory() {
    return &this->contactHistory_;
}

/**
 * Adds the given discovered contact to the contact plan, removes any predicted contact for that
 * pair and notifies the routing about it
 *
 * @param c: The contact to be added
 *
 * @author Simon Rink
 */
void ContactDtn::addDiscoveredContact(Contact c) {
    // remove predicted contacts for the source/destination pair
    Contact contact = this->contactPlan_.removePredictedContactForSourceDestination(
        c.getSourceEid(), c.getDestinationEid());
    if (contact.getId() != -1) {
        this->routing->updateContactPlan(&contact);
    }

    // add the discovered contact + range
    const int id = this->contactPlan_.addDiscoveredContact(c.getStart(), 1000000, c.getSourceEid(),
                                                     c.getDestinationEid(), c.getDataRate(),
                                                     c.getConfidence(), 0);
    if (id == -1) {
        return;
    }
    const double range = this->contactTopology_.getRangeBySrcDst(c.getSourceEid(), c.getDestinationEid());
    this->contactPlan_.addRange(c.getStart(), 1000000, c.getSourceEid(), c.getDestinationEid(),
                                range, c.getConfidence());
    this->contactPlan_.getContactById(id)->setRange(range);
    this->routing->updateContactPlan(nullptr);
}

/**
 * Removes the given discovered contact from the contact plan, and notifies the routing about it
 *
 * @param c: the contact to be removed
 *
 * @author Simon Rink
 */
void ContactDtn::removeDiscoveredContact(const Contact& c) {
    Contact contact = this->contactPlan_.removeDiscoveredContact(c.getSourceEid(), c.getDestinationEid());
    if (contact.getId() != -1)
        this->routing->updateContactPlan(&contact);
}

/*
 * Predicts all updated contacts
 *
 * @param currentTime: The current simulation time
 *
 * @author Simon Rink
 */
void ContactDtn::predictAllContacts(double currentTime) {
    this->contactHistory_.predictAndAddAllContacts(currentTime, &this->contactPlan_);
}

/**
 * Coordinates the contact start, thus it exchanges all discovered contacts and combines both
 * contact histories
 *
 * @param c: The started contact
 *
 * @author Simon Rink
 */
void ContactDtn::coordinateContactStart(Contact *c) const {
    if (this->eid_ != 0)
        throw invalid_argument("Illegal controller call");

    map<int, int> alreadyInformed;

    const int sourceEid = c->getSourceEid();
    const int destinationEid = c->getDestinationEid();

    const auto source = check_and_cast<ContactDtn *>(this->getParentModule()
                                            ->getParentModule()
                                            ->getSubmodule("node", sourceEid)
                                            ->getSubmodule("dtn"));
    const auto destination = check_and_cast<ContactDtn *>(this->getParentModule()
                                                 ->getParentModule()
                                                 ->getSubmodule("node", destinationEid)
                                                 ->getSubmodule("dtn"));

    ContactHistory *sourceHistory = source->getContactHistory();
    ContactHistory *destinationHistory = destination->getContactHistory();

    // get all discovered contacts from both source and destination node
    vector<Contact> sourceDiscoveredContacts =
        (*source->getContactPlanPointer()).getDiscoveredContacts();
    vector<Contact> destinationDiscoveredContacts =
        (*destination->getContactPlanPointer()).getDiscoveredContacts();

    // add foreign contacts to the respective contact plans
    for (auto & sourceDiscoveredContact : sourceDiscoveredContacts) {
        destination->addDiscoveredContact(sourceDiscoveredContact);
        destination->notifyNeighborsAboutDiscoveredContact(&sourceDiscoveredContact, true,
                                                           &alreadyInformed);
        alreadyInformed.clear();
    }

    for (auto & destinationDiscoveredContact : destinationDiscoveredContacts) {
        source->addDiscoveredContact(destinationDiscoveredContact);
        source->notifyNeighborsAboutDiscoveredContact(&destinationDiscoveredContact, true,
                                                      &alreadyInformed);
        alreadyInformed.clear();
    }

    // if the new contact is discovered, add it to the contact plans of both nodes and notify your
    // neighbors.
    if (c->isDiscovered()) {
        source->addDiscoveredContact(*c);
        destination->addDiscoveredContact(*c);
        source->notifyNeighborsAboutDiscoveredContact(c, true, &alreadyInformed);
        destination->notifyNeighborsAboutDiscoveredContact(c, true, &alreadyInformed);
    }

    source->addCurrentNeighbor(destinationEid);
    destination->addCurrentNeighbor(sourceEid);

    // combine the two contact histories.
    (*sourceHistory).combineContactHistories(destinationHistory);
    (*destinationHistory).combineContactHistories(sourceHistory);

    // after every new contact information is known, predict the contacts again.
    source->predictAllContacts(simTime().dbl());
    destination->predictAllContacts(simTime().dbl());

    // update the contact plans of the routing algorithms
    source->getRouting()->updateContactPlan(nullptr);
    destination->getRouting()->updateContactPlan(nullptr);
}

/**
 * Coordinates the end of a contact. Thus, the discovered contacts are updated (and the neighbors
 * informed), the contact history updated and the contacts predicted
 *
 * @param c: the contact that just ended.
 *
 * @author Simon Rink
 */
void ContactDtn::coordinateContactEnd(Contact *c) const {
    if (this->eid_ != 0)
        throw invalid_argument("Illegal controller call");

    vector<Contact> removedContacts;
    map<int, int> hasBeenInformed;

    const int sourceEid = c->getSourceEid();
    const int destinationEid = c->getDestinationEid();

    const auto source = check_and_cast<ContactDtn *>(
        this->getParentModule()
              ->getParentModule()
              ->getSubmodule("node", sourceEid)
              ->getSubmodule("dtn")
    );
    const auto destination = check_and_cast<ContactDtn *>(
        this->getParentModule()
               ->getParentModule()
               ->getSubmodule("node", destinationEid)
               ->getSubmodule("dtn")
    );

    ContactHistory *sourceHistory = source->getContactHistory();
    ContactHistory *destinationHistory = destination->getContactHistory();

    // Connection is dropped, they are no longer neighbors
    source->removeCurrentNeighbor(destinationEid);
    destination->removeCurrentNeighbor(sourceEid);

    source->updateDiscoveredContacts(c);
    destination->updateDiscoveredContacts(c);

    // Update the contact plans/histories and notify neighbors about lost contact.
    if (c->isDiscovered()) {
        // Add the contact to the histories
        if (const int sourceAvailable = rand() % 100; sourceAvailable < 1) {
            sourceHistory->addContact(nullptr, c);
            destinationHistory->addContact(nullptr, c);
        } else {
            sourceHistory->addContact(c, c);
            destinationHistory->addContact(c, c);
        }

        // Remove the contact
        source->removeDiscoveredContact(*c);
        destination->removeDiscoveredContact(*c);

        // Predict a new contact for the source/destination pair
        source->predictAllContacts(simTime().dbl());
        destination->predictAllContacts(simTime().dbl());

        // Notify neighbors
        source->notifyNeighborsAboutDiscoveredContact(c, false, &hasBeenInformed);
        destination->notifyNeighborsAboutDiscoveredContact(c, false, &hasBeenInformed);

        source->getRouting()->updateContactPlan(nullptr);
        destination->getRouting()->updateContactPlan(nullptr);
    }
}

/**
 * Notifies all current neighbors about a discovered contact start/end. The function is then
 * recalled by each of them, such that their neighbors are also notified.
 *
 * @param c: The contact that just started/ended.
 * @param start: a boolean whether the contact started or ended.
 * @param alreadyInformed: a pointer to a map that tracks which nodes were already notified.
 *
 * @author Simon Rink
 *
 */
void ContactDtn::notifyNeighborsAboutDiscoveredContact(Contact *c, const bool start, map<int, int> *alreadyInformed) {
    const vector<int> currentNeighbors = this->contactPlan_.getCurrentNeighbors();
    const auto controller = check_and_cast<ContactDtn *>(
        this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule("dtn")
    );

    for (const int currentNeighbor : currentNeighbors) {
        if (alreadyInformed->find(currentNeighbor) == alreadyInformed->end()) {
            controller->syncDiscoveredContactFromNeighbor(c, start, this->eid_, currentNeighbor);
            (*alreadyInformed)[currentNeighbor] = 1;
            const auto neighbor = check_and_cast<ContactDtn *>(this->getParentModule()
                                                      ->getParentModule()
                                                      ->getSubmodule("node", currentNeighbor)
                                                      ->getSubmodule("dtn"));
            neighbor->notifyNeighborsAboutDiscoveredContact(c, start, alreadyInformed);
        }
    }
}

/**
 * Updates the list of discovered contacts that are still reachable
 *
 * @param: the lost contact.
 *
 * @author Simon Rink
 */
void ContactDtn::updateDiscoveredContacts(Contact *c) {
    const vector<Contact> discoveredContacts = this->contactPlan_.getDiscoveredContacts();
    vector<Contact> lostContacts;
    map<int, int> reachableNodes = this->getReachableNodes(); // Obtain the nodes that are still reachable
    map<int, int> alreadyInformed;

    for (const auto& contact : discoveredContacts) {
        if (reachableNodes.find(contact.getSourceEid()) ==
            reachableNodes.end()) // contact is not reachable anymore
        {
            lostContacts.push_back(contact);
        }
    }

    for (auto & lostContact : lostContacts) {
        this->removeDiscoveredContact(lostContact); // remove the discovered contact
        this->notifyNeighborsAboutDiscoveredContact(
            &lostContact, false, &alreadyInformed); // inform your neighbors about the loss
        alreadyInformed.clear();
    }
}

/**
 * Identifies the reachable nodes from the current node
 *
 * @return A HashMap of reachable nodes
 *
 * @author Simon Rink
 */
map<int, int> ContactDtn::getReachableNodes() const {
    map<int, int> alreadyFound;
    map<int, int> stillAvailable;
    stillAvailable[this->eid_] = 1;

    while (!stillAvailable.empty()) { // As long as there are still new nodes available go into the loop
        int newNeighbor = stillAvailable.begin()->first;

        stillAvailable.erase(stillAvailable.begin()); // Remove the first element at it is traversed just right now
        alreadyFound[newNeighbor] = 1;

        const auto neighbor = check_and_cast<ContactDtn *>(
            this->getParentModule()
                  ->getParentModule()
                  ->getSubmodule("node", newNeighbor)
                  ->getSubmodule("dtn"));
        ContactPlan *neighborContactPlan = neighbor->getContactPlanPointer();
        vector<int> newNeighbors = neighborContactPlan->getCurrentNeighbors(); // Identify current connection for the node

        for (int newNeighbour : newNeighbors) {
            if (alreadyFound.find(newNeighbour) != alreadyFound.end()) { // Node is already found
                continue;
            }
            if (stillAvailable.find(newNeighbour) == stillAvailable.end()) { // A new node to be traversed was found
                stillAvailable[newNeighbour] = 1;
            }
        }
    }

    return alreadyFound;
}

/*
 * Adds a new neighbor to the contact plan
 *
 * @param neighborEid: The EID of the new neighbor
 *
 * @author Simon Rink
 */
void ContactDtn::addCurrentNeighbor(const int neighborEid) {
    this->contactPlan_.addCurrentNeighbor(neighborEid);
}

/*
 * Removes a neighbor from the contact plan
 *
 * @param neighbor: The EID of the neighbor to be removed
 *
 * @author Simon Rink
 */
void ContactDtn::removeCurrentNeighbor(const int neighborEid) {
    this->contactPlan_.removeCurrentNeighbor(neighborEid);
}

/**
 * Checks whether a contact exists with the given parameters
 *
 * @param sourceEid: The source of the contact
 * @param destinationEid: destination of the contact.
 * @param start: start time of the contact
 *
 * @return The ID of the contact if it exists, or 0, if none exists
 *
 * @author Simon Rink
 */
int ContactDtn::checkExistenceOfContact(const int sourceEid, const int destinationEid, const int start) {
    const Contact *contact = this->contactTopology_.getContactBySrcDstStart(sourceEid, destinationEid, start);

    if (contact == nullptr || contact->getEnd() <= simTime().dbl()) {
        return 0; // No contact found or contact already over
    }
    return contact->getId();
}
