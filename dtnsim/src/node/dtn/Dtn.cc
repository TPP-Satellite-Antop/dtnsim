#include "src/node/dtn/Dtn.h"
#include "src/node/app/App.h"
#include "src/node/dtn/routing/RoutingAntop.h"

Define_Module(Dtn);

Dtn::Dtn() {}

Dtn::~Dtn() {}

void Dtn::setContactPlan(ContactPlan &contactPlan) {
    this->contactPlan_ = contactPlan;
}

void Dtn::setContactTopology(ContactPlan &contactTopology) {
    this->contactTopology_ = contactTopology;
}

void Dtn::setMetricCollector(MetricCollector *metricCollector) {
    this->metricCollector_ = metricCollector;
}

int Dtn::numInitStages() const {
    int stages = 2;
    return stages;
}

/**
 * Initializes the Dtn object
 *
 * @param stage: the stage for the dtn object
 *
 * @authors The original implementation was done by the authors of DTNSim and then modified by Simon
 * Rink
 */
void Dtn::initialize(int stage) {
    if (stage == 1) {
        // Store this node eid
        this->eid_ = this->getParentModule()->getIndex();

        this->custodyTimeout_ = par("custodyTimeout");
        this->custodyModel_.setEid(eid_);
        this->custodyModel_.setSdr(&sdr_);
        this->custodyModel_.setCustodyReportByteSize(par("custodyReportByteSize"));

        // Get a pointer to graphics module
        graphicsModule = (Graphics *)this->getParentModule()->getSubmodule("graphics");
        // Register this object as sdr obsever, in order to display bundles stored in sdr properly.
        sdr_.addObserver(this);
        update();

        // Create (empty) contact history
        this->contactHistory_ = ContactHistory();

        // Schedule local starts contact messages.
        // Only contactTopology start contacts are scheduled.
        vector<Contact> localContacts1 = contactTopology_.getContactsBySrc(this->eid_);
        for (vector<Contact>::iterator it = localContacts1.begin(); it != localContacts1.end();
             ++it) {
            ContactMsg *contactMsgStart;

            if ((*it).isDiscovered()) {
                contactMsgStart = new ContactMsg("discContactStart", DISC_CONTACT_START_TIMER);
                contactMsgStart->setSchedulingPriority(DISC_CONTACT_START_TIMER);
            } else {
                contactMsgStart = new ContactMsg("ContactStart", CONTACT_START_TIMER);
                contactMsgStart->setSchedulingPriority(CONTACT_START_TIMER);
            }

            contactMsgStart->setId((*it).getId());
            contactMsgStart->setStart((*it).getStart());
            contactMsgStart->setEnd((*it).getEnd());
            contactMsgStart->setDuration((*it).getEnd() - (*it).getStart());
            contactMsgStart->setSourceEid((*it).getSourceEid());
            contactMsgStart->setDestinationEid((*it).getDestinationEid());
            contactMsgStart->setDataRate((*it).getDataRate());

            scheduleAt((*it).getStart(), contactMsgStart);

            EV << "node " << eid_ << ": "
               << "a contact +" << (*it).getStart() << " +" << (*it).getEnd() << " "
               << (*it).getSourceEid() << " " << (*it).getDestinationEid() << " "
               << (*it).getDataRate() << endl;
        }
        // Schedule local ends contact messages.
        // All ends contacts of the contactTopology are scheduled.
        // to trigger re-routings of bundles queued in contacts that did not happen.
        vector<Contact> localContacts2 = contactTopology_.getContactsBySrc(this->eid_);
        for (vector<Contact>::iterator it = localContacts2.begin(); it != localContacts2.end();
             ++it) {

            ContactMsg *contactMsgEnd;

            if ((*it).isDiscovered()) {
                contactMsgEnd = new ContactMsg("discContactEnd", DISC_CONTACT_END_TIMER);
                contactMsgEnd->setSchedulingPriority(DISC_CONTACT_END_TIMER);
                contactMsgEnd->setName("discContactEnd");
            } else {
                contactMsgEnd = new ContactMsg("ContactEnd", CONTACT_END_TIMER);
                contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
                contactMsgEnd->setName("ContactEnd");
            }

            contactMsgEnd->setId((*it).getId());
            contactMsgEnd->setStart((*it).getStart());
            contactMsgEnd->setEnd((*it).getEnd());
            contactMsgEnd->setDuration((*it).getEnd() - (*it).getStart());
            contactMsgEnd->setSourceEid((*it).getSourceEid());
            contactMsgEnd->setDestinationEid((*it).getDestinationEid());
            contactMsgEnd->setDataRate((*it).getDataRate());

            scheduleAt((*it).getStart() + (*it).getDuration(), contactMsgEnd);
        }

        string routeString = par("routing");

        if (routeString.compare("uncertainUniboCgr") == 0) // only done for (O)CGR-UCoP
        {
            vector<Contact> localContacts3 = contactPlan_.getContactsBySrc(this->eid_);
            for (auto it = localContacts3.begin(); it != localContacts3.end(); it++) {
                if (this->checkExistenceOfContact(it->getSourceEid(), it->getDestinationEid(),
                                                  it->getStart()) == 0) // identify failed contacts
                {
                    ContactMsg *failedMsg;
                    failedMsg = new ContactMsg("contactFailed", CONTACT_FAILED);
                    failedMsg->setSchedulingPriority(CONTACT_FAILED);
                    failedMsg->setName("failedMsg");
                    failedMsg->setId(it->getId());

                    scheduleAt(it->getEnd(), failedMsg);
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

void Dtn::initializeRouting(string routeString) {
    this->sdr_.setEid(eid_);
    this->sdr_.setSize(par("sdrSize"));
    this->sdr_.setNodesNumber(this->getParentModule()->getParentModule()->par("nodesNumber"));
    this->sdr_.setContactPlan(&contactTopology_);

    if (routeString.compare("direct") == 0)
        routing = new RoutingDirect(eid_, &sdr_, &contactPlan_);
    else if (routeString.compare("antop") == 0)
        routing = new RoutingAntop(eid_, &sdr_, &contactPlan_);
    else if (routeString.compare("cgrModel350") == 0)
        routing = new RoutingCgrModel350(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routeString.compare("cgrModel350_Hops") == 0)
        routing = new RoutingCgrModel350_Hops(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routeString.compare("cgrModelYen") == 0)
        routing = new RoutingCgrModelYen(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"));
    else if (routeString.compare("cgrModelRev17") == 0) {
        ContactPlan *globalContactPlan = ((Dtn *)this->getParentModule()
                                              ->getParentModule()
                                              ->getSubmodule("node", 0)
                                              ->getSubmodule("dtn"))
                                             ->getContactPlanPointer();
        routing = new RoutingCgrModelRev17(eid_, this->getParentModule()->getVectorSize(), &sdr_,
                                           &contactPlan_, globalContactPlan, par("routingType"),
                                           par("printRoutingDebug"));
    } else if (routeString.compare("epidemic") == 0) {
        routing = new RoutingEpidemic(eid_, &sdr_, this);
    } else if (routeString.compare("sprayAndWait") == 0) {
        int bundlesCopies = par("bundlesCopies");
        routing = new RoutingSprayAndWait(eid_, &sdr_, this, bundlesCopies, false,
                                          this->metricCollector_);
    } else if (routeString.compare("binarySprayAndWait") == 0) {
        int bundlesCopies = par("bundlesCopies");
        routing =
            new RoutingSprayAndWait(eid_, &sdr_, this, bundlesCopies, true, this->metricCollector_);
    } else if (routeString.compare("PRoPHET") == 0) {
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
    } else if (routeString.compare("cgrModel350_2Copies") == 0)
        routing = new RoutingCgrModel350_2Copies(eid_, &sdr_, &contactPlan_,
                                                 par("printRoutingDebug"), this);
    else if (routeString.compare("cgrModel350_Probabilistic") == 0) {
        double sContactProb = par("sContactProb");
        routing = new RoutingCgrModel350_Probabilistic(
            eid_, &sdr_, &contactPlan_, par("printRoutingDebug"), this, sContactProb);
    } else if (routeString.compare("uncertainUniboCgr") == 0) {
        bool useUncertainty =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par(
                "useUncertainty");
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing =
            new RoutingUncertainUniboCgr(eid_, &sdr_, &contactPlan_, this, this->metricCollector_,
                                         -1, useUncertainty, repetition, numOfNodes);
    } else if (routeString.compare("uniboCgr") == 0) {
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing =
            new RoutingUncertainUniboCgr(eid_, &sdr_, &contactPlan_, this, this->metricCollector_,
                                         -1, false, repetition, numOfNodes);
    } else if (routeString.compare("ORUCOP") == 0) {
        int numOfNodes = this->getParentModule()->getParentModule()->par("nodesNumber");
        int repetition =
            this->getParentModule()->getParentModule()->getSubmodule("central")->par("repetition");
        routing = new RoutingORUCOP(eid_, &sdr_, &contactPlan_, this, this->metricCollector_, 2,
                                    repetition, numOfNodes);
    } // 2 bundles for now.
    else if (routeString.compare("BRUF1T") == 0) {
        string frouting = par("frouting");
        routing = new RoutingBRUF1T(eid_, &sdr_, &contactPlan_, frouting);
    } else if (routeString.compare("BRUFNCopies") == 0) {
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
    } else if (routeString.compare("CGR_BRUFPowered") == 0) {
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

        routing =
            new CGRBRUFPowered(eid_, &sdr_, &contactPlan_, par("printRoutingDebug"), pf,
                               ts_duration, ts_start_times, numOfNodes, prefix.str(), posfix.str());
    } else {
        cout << "dtnsim error: unknown routing type: " << routeString << endl;
        exit(1);
    }
}

void Dtn::finish() {
    // Last call to sample-hold type metrics
    if (eid_ != 0) {
        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());
    }

    // Delete scheduled forwardingMsg
    std::map<int, ForwardingMsgStart *>::iterator it;
    for (it = forwardingMsgs_.begin(); it != forwardingMsgs_.end(); ++it) {
        ForwardingMsgStart *forwardingMsg = it->second;
        cancelAndDelete(forwardingMsg);
    }

    // Delete all stored bundles
    sdr_.freeSdr(eid_);

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

void Dtn::handleMessage(cMessage *msg) {
    ///////////////////////////////////////////
    // New Bundle (from App or Com):
    ///////////////////////////////////////////
    if (msg->getKind() == BUNDLE || msg->getKind() == BUNDLE_CUSTODY_REPORT) {
        if (msg->arrivedOn("gateToCom$i"))
            emit(dtnBundleReceivedFromCom, true);
        if (msg->arrivedOn("gateToApp$i"))
            emit(dtnBundleReceivedFromApp, true);

        BundlePkt *bundle = check_and_cast<BundlePkt *>(msg);
        dispatchBundle(bundle);
    } else if (msg->getKind() == CONTACT_FAILED) // A failed contact was noticed!
    {
        ContactMsg *contactMsg = check_and_cast<ContactMsg *>(msg);

        RoutingUncertainUniboCgr *uniboRouting =
            check_and_cast<RoutingUncertainUniboCgr *>(this->routing);
        uniboRouting->contactFailure(contactMsg->getId()); // reroute all failed bundles!

        this->refreshForwarding();

        delete contactMsg;
    }

    ///////////////////////////////////////////
    // Contact Start and End
    ///////////////////////////////////////////
    else if (msg->getKind() == DISC_CONTACT_START_TIMER) // Discovered contact was found
    {
        ContactMsg *contactMsg = check_and_cast<ContactMsg *>(msg);

        Dtn *controller = check_and_cast<Dtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        (*controller)
            .syncDiscoveredContact(contactTopology_.getContactById(contactMsg->getId()), true);

        delete contactMsg;

    } else if (msg->getKind() == DISC_CONTACT_END_TIMER) // Discovered contact ended
    {
        ContactMsg *contactMsg = check_and_cast<ContactMsg *>(msg);

        Dtn *controller = check_and_cast<Dtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        (*controller)
            .syncDiscoveredContact(contactTopology_.getContactById(contactMsg->getId()), false);

        delete contactMsg;
    }

    else if (msg->getKind() == CONTACT_START_TIMER) {

        // Schedule end of contact
        ContactMsg *contactMsg = check_and_cast<ContactMsg *>(msg);

        Contact *contact = contactTopology_.getContactById(contactMsg->getId());

        Dtn *controller = check_and_cast<Dtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        // for opportunistic extensions
        controller->coordinateContactStart(contact);

        // Visualize contact line on
        graphicsModule->setContactOn(contactMsg);

        // Call to routing algorithm
        routing->contactStart(contact);

        // Schedule start of transmission
        ForwardingMsgStart *forwardingMsg =
            new ForwardingMsgStart("forwardingMsgStart", FORWARDING_MSG_START);
        forwardingMsg->setSchedulingPriority(FORWARDING_MSG_START);
        forwardingMsg->setNeighborEid(contactMsg->getDestinationEid());
        forwardingMsg->setContactId(contactMsg->getId());
        forwardingMsgs_[contactMsg->getId()] = forwardingMsg;
        scheduleAt(simTime(), forwardingMsg);

        delete contactMsg;
    } else if (msg->getKind() == CONTACT_END_TIMER) {
        // Finish transmission: If bundles are left in contact re-route them
        ContactMsg *contactMsg = check_and_cast<ContactMsg *>(msg);

        Contact *contact = contactTopology_.getContactById(contactMsg->getId());

        Dtn *controller = check_and_cast<Dtn *>(
            this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule(
                "dtn"));

        // for opportunistic extensions
        controller->coordinateContactEnd(contact);

        for (int i = 0; i < sdr_.getBundlesCountInContact(contactMsg->getId()); i++)
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
        ForwardingMsgStart *forwardingMsgStart = check_and_cast<ForwardingMsgStart *>(msg);
        int neighborEid = forwardingMsgStart->getNeighborEid();
        int contactId = forwardingMsgStart->getContactId();

        // save freeChannelMsg to cancel event if necessary
        forwardingMsgs_[forwardingMsgStart->getContactId()] = forwardingMsgStart;

        // if there are messages in the queue for this contact
        if (sdr_.isBundleForContact(contactId)) {
            // If local/remote node are responsive, then transmit bundle
            Dtn *neighborDtn = check_and_cast<Dtn *>(this->getParentModule()
                                                         ->getParentModule()
                                                         ->getSubmodule("node", neighborEid)
                                                         ->getSubmodule("dtn"));
            if ((!neighborDtn->onFault) && (!this->onFault)) {
                // Get bundle pointer from sdr
                BundlePkt *bundle = sdr_.getNextBundleForContact(contactId);

                // Calculate data rate and Tx duration
                double dataRate = contactTopology_.getContactById(contactId)->getDataRate();
                double txDuration = (double)bundle->getByteLength() / dataRate;
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

                    sdr_.popNextBundleForContact(contactId);

                    // If custody requested, store a copy of the bundle until report received
                    if (bundle->getCustodyTransferRequested()) {
                        sdr_.enqueueTransmittedBundleInCustody(bundle->dup());
                        this->custodyModel_.printBundlesInCustody();

                        // Enqueue a retransmission event in case custody acceptance not received
                        CustodyTimout *custodyTimeout =
                            new CustodyTimout("custodyTimeout", CUSTODY_TIMEOUT);
                        custodyTimeout->setBundleId(bundle->getBundleId());
                        scheduleAt(simTime() + this->custodyTimeout_, custodyTimeout);
                    }

                    emit(dtnBundleSentToCom, true);
                    emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
                    emit(sdrBytesStored, sdr_.getBytesStoredInSdr());

                    // Schedule next transmission
                    scheduleAt(simTime() + txDuration, forwardingMsgStart);

                    // Schedule forwarding message end
                    ForwardingMsgEnd *forwardingMsgEnd =
                        new ForwardingMsgEnd("forwardingMsgEnd", FORWARDING_MSG_END);
                    forwardingMsgEnd->setSchedulingPriority(FORWARDING_MSG_END);
                    forwardingMsgEnd->setNeighborEid(neighborEid);
                    forwardingMsgEnd->setContactId(contactId);
                    forwardingMsgEnd->setBundleId(bundle->getBundleId());
                    forwardingMsgEnd->setSentToDestination(neighborEid ==
                                                           bundle->getDestinationEid());
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
        ForwardingMsgEnd *forwardingMsgEnd = check_and_cast<ForwardingMsgEnd *>(msg);
        int bundleId = forwardingMsgEnd->getBundleId();
        int contactId = forwardingMsgEnd->getContactId();
        Contact *contact = contactTopology_.getContactById(contactId);

        routing->successfulBundleForwarded(bundleId, contact,
                                           forwardingMsgEnd->getSentToDestination());
        delete forwardingMsgEnd;
    }
    ///////////////////////////////////////////
    // Custody retransmission timer
    ///////////////////////////////////////////
    else if (msg->getKind() == CUSTODY_TIMEOUT) {
        // Custody timer expired, check if bundle still in custody memory space and retransmit it if
        // positive
        CustodyTimout *custodyTimout = check_and_cast<CustodyTimout *>(msg);
        BundlePkt *reSendBundle = this->custodyModel_.custodyTimerExpired(custodyTimout);

        if (reSendBundle != NULL)
            this->dispatchBundle(reSendBundle);

        delete custodyTimout;
    }
}

void Dtn::dispatchBundle(BundlePkt *bundle) {
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
                BundlePkt *reSendBundle = this->custodyModel_.custodyReportArrived(bundle);

                // If custody was rejected, reroute
                if (reSendBundle != NULL)
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
        string routeString = par("routing");
        if (routeString.compare("cgrModel350") == 0) {
            emit(routeCgrDijkstraCalls, ((RoutingCgrModel350 *)routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, ((RoutingCgrModel350 *)routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesExplored,
                 ((RoutingCgrModel350 *)routing)->getRouteTableEntriesExplored());
        }
        if (routeString.compare("cgrModel350_3") == 0) {
            emit(routeCgrDijkstraCalls, ((RoutingCgrModel350_Hops *)routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, ((RoutingCgrModel350_Hops *)routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesExplored,
                 ((RoutingCgrModel350_Hops *)routing)->getRouteTableEntriesExplored());
        }
        if (routeString.compare("cgrModelRev17") == 0) {
            emit(routeCgrDijkstraCalls, ((RoutingCgrModelRev17 *)routing)->getDijkstraCalls());
            emit(routeCgrDijkstraLoops, ((RoutingCgrModelRev17 *)routing)->getDijkstraLoops());
            emit(routeCgrRouteTableEntriesCreated,
                 ((RoutingCgrModelRev17 *)routing)->getRouteTableEntriesCreated());
            emit(routeCgrRouteTableEntriesExplored,
                 ((RoutingCgrModelRev17 *)routing)->getRouteTableEntriesExplored());
        }
        emit(sdrBundleStored, sdr_.getBundlesCountInSdr());
        emit(sdrBytesStored, sdr_.getBytesStoredInSdr());

        // Wake-up sleeping forwarding threads
        this->refreshForwarding();
    }
}

void Dtn::refreshForwarding() {
    // Check all on-going forwardingMsgs threads
    // (contacts) and wake up those not scheduled.
    std::map<int, ForwardingMsgStart *>::iterator it;
    for (it = forwardingMsgs_.begin(); it != forwardingMsgs_.end(); ++it) {
        ForwardingMsgStart *forwardingMsg = it->second;
        int cid = forwardingMsg->getContactId();
        if (!sdr_.isBundleForContact(cid))
            // notify routing protocol that it has messages to send and contacts for routing
            routing->refreshForwarding(contactTopology_.getContactById(cid));
        if (!forwardingMsg->isScheduled()) {
            scheduleAt(simTime(), forwardingMsg);
        }
    }
}

void Dtn::setOnFault(bool onFault) {
    this->onFault = onFault;

    // Local and remote forwarding recovery
    if (onFault == false) {
        // Wake-up local un-scheduled forwarding threads
        this->refreshForwarding();

        // Wake-up remote un-scheduled forwarding threads
        std::map<int, ForwardingMsgStart *>::iterator it;
        for (it = forwardingMsgs_.begin(); it != forwardingMsgs_.end(); ++it) {
            ForwardingMsgStart *forwardingMsg = it->second;
            Dtn *remoteDtn = (Dtn *)this->getParentModule()
                                 ->getParentModule()
                                 ->getSubmodule("node", forwardingMsg->getNeighborEid())
                                 ->getSubmodule("dtn");
            remoteDtn->refreshForwarding();
        }
    }
}

ContactPlan *Dtn::getContactPlanPointer(void) {
    return &this->contactPlan_;
}

Routing *Dtn::getRouting(void) {
    return this->routing;
}

/**
 * Implementation of method inherited from observer to update gui according to the number of
 * bundles stored in sdr.
 */
void Dtn::update(void) {
    // update srd size text
    graphicsModule->setBundlesInSdr(sdr_.getBundlesCountInSdr());
}

// PROCEDURES FOR OPPORTUNISTIC ROUTING!

/**
 * Schedules the contact start or end for a discovered contact
 *
 * @param c: The started/ended discovered contact
 * 	      start: Boolean whether the contact started or ended
 *
 * @author Simon Rink
 */
void Dtn::syncDiscoveredContact(Contact *c, bool start) {
    // only controller node is allowed to decide on final topology
    if (!this->eid_ == 0) {
        throw invalid_argument("Illegal controller call");
    }

    if (start) {

        // schedule start of contact for sender
        int sourceEid = (*c).getSourceEid();
        Dtn *source = check_and_cast<Dtn *>(this->getParentModule()
                                                ->getParentModule()
                                                ->getSubmodule("node", sourceEid)
                                                ->getSubmodule("dtn"));
        source->scheduleDiscoveredContactStart(c);

    } else {

        // schedule end of contact for sender
        int sourceEid = (*c).getSourceEid();
        Dtn *source = check_and_cast<Dtn *>(this->getParentModule()
                                                ->getParentModule()
                                                ->getSubmodule("node", sourceEid)
                                                ->getSubmodule("dtn"));
        source->scheduleDiscoveredContactEnd(c);
    }
}

/**
 * Adds or removes a discovered contact from a neighbor
 *
 * @param c: The contact to be added/removed
 * 	      start: Boolean whether the contact started or ended
 *
 * @author Simon Rink
 */
void Dtn::syncDiscoveredContactFromNeighbor(Contact *c, bool start, int ownEid, int neighborEid) {
    if (!this->eid_ == 0) {
        throw invalid_argument("Illegal controller call");
    }

    Dtn *neighbor = check_and_cast<Dtn *>(this->getParentModule()
                                              ->getParentModule()
                                              ->getSubmodule("node", neighborEid)
                                              ->getSubmodule("dtn"));

    if (start) {

        // add discovered contact into the contact plan of the neighbor and inform its neighbors
        neighbor->addDiscoveredContact(*c);

    } else {

        // remove discovered contact from the contact plan of the neighbor and inform its neighbors
        neighbor->removeDiscoveredContact(*c);
    }
}

/**
 * Schedules the start of a discovered contact
 *
 * @param c: The contact to be started
 *
 *
 * @author Simon Rink
 */
void Dtn::scheduleDiscoveredContactStart(Contact *c) {
    // schedule a new message for the actual contact start
    ContactMsg *contactMsgStart = new ContactMsg("ContactStart", CONTACT_START_TIMER);
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
void Dtn::scheduleDiscoveredContactEnd(Contact *c) {
    // schedule a new message for the actual contact end
    ContactMsg *contactMsgEnd = new ContactMsg("ContactEnd", CONTACT_END_TIMER);
    contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
    contactMsgEnd->setName("ContactEnd");
    contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
    contactMsgEnd->setId((*c).getId());
    contactMsgEnd->setStart((*c).getStart());
    contactMsgEnd->setEnd((*c).getEnd());
    contactMsgEnd->setDuration((*c).getEnd() - (*c).getStart());
    contactMsgEnd->setSourceEid((*c).getSourceEid());
    contactMsgEnd->setDestinationEid((*c).getDestinationEid());
    contactMsgEnd->setDataRate((*c).getDataRate());

    scheduleAt(simTime(), contactMsgEnd);
}

ContactHistory *Dtn::getContactHistory() {
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
void Dtn::addDiscoveredContact(Contact c) {

    // remove predicted contacts for the source/destination pair
    Contact contact = this->contactPlan_.removePredictedContactForSourceDestination(
        c.getSourceEid(), c.getDestinationEid());
    if (contact.getId() != -1) {
        this->routing->updateContactPlan(&contact);
    }

    // add the discovered contact + range
    int id = this->contactPlan_.addDiscoveredContact(c.getStart(), 1000000, c.getSourceEid(),
                                                     c.getDestinationEid(), c.getDataRate(),
                                                     c.getConfidence(), 0);
    if (id == -1) {
        return;
    }
    double range = this->contactTopology_.getRangeBySrcDst(c.getSourceEid(), c.getDestinationEid());
    this->contactPlan_.addRange(c.getStart(), 1000000, c.getSourceEid(), c.getDestinationEid(),
                                range, c.getConfidence());
    this->contactPlan_.getContactById(id)->setRange(range);
    this->routing->updateContactPlan(NULL);
}

/**
 * Removes the given discovered contact from the contact plan, and notifies the routing about it
 *
 * @param c: The contact to be removed
 *
 * @author Simon Rink
 */
void Dtn::removeDiscoveredContact(Contact c) {
    Contact contact =
        this->contactPlan_.removeDiscoveredContact(c.getSourceEid(), c.getDestinationEid());

    if (contact.getId() != -1) {
        this->routing->updateContactPlan(&contact);
    }
}

/*
 * Predicts all updated contacts
 *
 * @param currentTime: The current simulation time
 *
 * @author Simon Rink
 */
void Dtn::predictAllContacts(double currentTime) {
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
void Dtn::coordinateContactStart(Contact *c) {
    if (!this->eid_ == 0) {
        throw invalid_argument("Illegal controller call");
    }

    map<int, int> alreadyInformed;

    int sourceEid = (*c).getSourceEid();
    int destinationEid = (*c).getDestinationEid();

    Dtn *source = check_and_cast<Dtn *>(this->getParentModule()
                                            ->getParentModule()
                                            ->getSubmodule("node", sourceEid)
                                            ->getSubmodule("dtn"));
    Dtn *destination = check_and_cast<Dtn *>(this->getParentModule()
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
    for (size_t i = 0; i < sourceDiscoveredContacts.size(); i++) {
        destination->addDiscoveredContact(sourceDiscoveredContacts.at(i));
        destination->notifyNeighborsAboutDiscoveredContact(&sourceDiscoveredContacts.at(i), true,
                                                           &alreadyInformed);
        alreadyInformed.clear();
    }

    for (size_t i = 0; i < destinationDiscoveredContacts.size(); i++) {
        source->addDiscoveredContact(destinationDiscoveredContacts.at(i));
        source->notifyNeighborsAboutDiscoveredContact(&destinationDiscoveredContacts.at(i), true,
                                                      &alreadyInformed);
        alreadyInformed.clear();
    }

    // if the new contact is discovered, add it to the contact plans of both nodes and notify your
    // neighbors.
    if ((*c).isDiscovered()) {
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
    source->getRouting()->updateContactPlan(NULL);
    destination->getRouting()->updateContactPlan(NULL);
}

/**
 * Coordinates the end of a contact. Thus the discovered contacts are updated (and the neighbors
 * informed), the contact history updated and the contacts predicted
 *
 * @param c: The contact that just ended
 *
 * @author Simon Rink
 */
void Dtn::coordinateContactEnd(Contact *c) {
    if (!this->eid_ == 0) {
        throw invalid_argument("Illegal controller call");
    }

    vector<Contact> removedContacts;
    map<int, int> hasBeenInformed;

    int sourceEid = (*c).getSourceEid();
    int destinationEid = (*c).getDestinationEid();

    Dtn *source = check_and_cast<Dtn *>(this->getParentModule()
                                            ->getParentModule()
                                            ->getSubmodule("node", sourceEid)
                                            ->getSubmodule("dtn"));
    Dtn *destination = check_and_cast<Dtn *>(this->getParentModule()
                                                 ->getParentModule()
                                                 ->getSubmodule("node", destinationEid)
                                                 ->getSubmodule("dtn"));

    ContactHistory *sourceHistory = source->getContactHistory();
    ContactHistory *destinationHistory = destination->getContactHistory();

    // connection is dropped, they are no longer neighbors
    source->removeCurrentNeighbor(destinationEid);
    destination->removeCurrentNeighbor(sourceEid);

    source->updateDiscoveredContacts(c);
    destination->updateDiscoveredContacts(c);

    // update the contact plans/histories and notify neighbors about lost contact.
    if (c->isDiscovered()) {
        int sourceAvailable = rand() % 100;

        // add the contact to the histories
        if (sourceAvailable < 1) {
            (*sourceHistory).addContact(NULL, c);
            (*destinationHistory).addContact(NULL, c);
        } else {
            (*sourceHistory).addContact(c, c);
            (*destinationHistory).addContact(c, c);
        }

        // remove the contact
        source->removeDiscoveredContact(*c);
        destination->removeDiscoveredContact(*c);

        // predict a new contact for the source/destination pair
        source->predictAllContacts(simTime().dbl());
        destination->predictAllContacts(simTime().dbl());

        // notify neighbors
        source->notifyNeighborsAboutDiscoveredContact(c, false, &hasBeenInformed);
        destination->notifyNeighborsAboutDiscoveredContact(c, false, &hasBeenInformed);

        source->getRouting()->updateContactPlan(NULL);
        destination->getRouting()->updateContactPlan(NULL);
    }
}

/**
 * Notifies all current neighbors about a discovered contact start/end. The function is then
 * recalled by each of them, such that their neighbors are also notified.
 *
 * @param c: The contact that just started/ended
 *        start: A boolean whether the contact started or ended
 *        alreadyInformed: A pointer to a map that tracks which nodes were already notified
 *
 * @author Simon Rink
 *
 */
void Dtn::notifyNeighborsAboutDiscoveredContact(Contact *c, bool start,
                                                map<int, int> *alreadyInformed) {
    vector<int> currentNeighbors = this->contactPlan_.getCurrentNeighbors();
    int currentNeighbor = 0;
    Dtn *controller = check_and_cast<Dtn *>(
        this->getParentModule()->getParentModule()->getSubmodule("node", 0)->getSubmodule("dtn"));

    for (size_t i = 0; i < currentNeighbors.size(); i++) {
        currentNeighbor = currentNeighbors[i];
        if (alreadyInformed->find(currentNeighbor) == alreadyInformed->end()) {
            controller->syncDiscoveredContactFromNeighbor(c, start, this->eid_, currentNeighbor);
            (*alreadyInformed)[currentNeighbor] = 1;
            Dtn *neighbor = check_and_cast<Dtn *>(this->getParentModule()
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
 * @param: The lost contact
 *
 * @author Simon Rink
 */
void Dtn::updateDiscoveredContacts(Contact *c) {
    vector<Contact> discoveredContacts = this->contactPlan_.getDiscoveredContacts();
    vector<Contact> lostContacts;
    map<int, int> reachableNodes =
        this->getReachableNodes(); // obtain the nodes that are still reachable
    map<int, int> alreadyInformed;

    for (size_t i = 0; i < discoveredContacts.size(); i++) {
        Contact contact = discoveredContacts.at(i);

        if (reachableNodes.find(contact.getSourceEid()) ==
            reachableNodes.end()) // contact is not reachable anymore
        {
            lostContacts.push_back(contact);
        }
    }

    for (size_t i = 0; i < lostContacts.size(); i++) {
        this->removeDiscoveredContact(lostContacts.at(i)); // remove the discovered contact
        this->notifyNeighborsAboutDiscoveredContact(
            &lostContacts.at(i), false, &alreadyInformed); // inform your neighbors about the loss
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
map<int, int> Dtn::getReachableNodes() {
    map<int, int> alreadyFound;
    map<int, int> stillAvailable;
    stillAvailable[this->eid_] = 1;

    while (stillAvailable.size() !=
           0) // as long as there are still new nodes available go into the loop
    {
        int newNeighbor = stillAvailable.begin()->first;
        stillAvailable.erase(
            stillAvailable.begin()); // remove the first element at it is traversed just right now
        alreadyFound[newNeighbor] = 1;

        Dtn *neighbor = check_and_cast<Dtn *>(this->getParentModule()
                                                  ->getParentModule()
                                                  ->getSubmodule("node", newNeighbor)
                                                  ->getSubmodule("dtn"));
        ContactPlan *neighborContactPlan = neighbor->getContactPlanPointer();
        vector<int> newNeighbors =
            neighborContactPlan->getCurrentNeighbors(); // identify current connection for the node

        for (size_t i = 0; i < newNeighbors.size(); i++) {
            if (alreadyFound.find(newNeighbors[i]) != alreadyFound.end()) // node is already found
            {
                continue;
            } else if (stillAvailable.find(newNeighbors[i]) ==
                       stillAvailable.end()) // a new node to be traversed was found
            {
                stillAvailable[newNeighbors[i]] = 1;
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
void Dtn::addCurrentNeighbor(int neighborEid) {
    this->contactPlan_.addCurrentNeighbor(neighborEid);
}

/*
 * Removes a neighbor from the contact plan
 *
 * @param neighbor: The EID of the neighbor to be removed
 *
 * @author Simon Rink
 */
void Dtn::removeCurrentNeighbor(int neighborEid) {
    this->contactPlan_.removeCurrentNeighbor(neighborEid);
}

/**
 * Checks whether a contact exists with the given parameters
 *
 * @param  sourceEid: The source of the contact
 * 		   destinationEid: The destination of the contact
 * 		   start: The start time of the contact
 *
 * @return The ID of the contact if it exists, or 0, if none exists
 *
 * @author Simon Rink
 */
int Dtn::checkExistenceOfContact(int sourceEid, int destinationEid, int start) {
    Contact *contact =
        this->contactTopology_.getContactBySrcDstStart(sourceEid, destinationEid, start);

    if (contact == NULL) {
        return 0; // no contact found
    } else if (contact->getEnd() <= simTime().dbl()) {
        return 0; // contact already over
    } else {
        return contact->getId();
    }
}
