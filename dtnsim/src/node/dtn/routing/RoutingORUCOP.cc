/*
 * RoutingORUCOP.cc
 *
 *  Created on: Jan 24, 2022
 *      Author: Simon Rink
 */

#include "RoutingORUCOP.h"

namespace fs = std::filesystem;

static map<long, long> originalBundleIds;

long getOriginalBundleId(long bundleId);

/*
 * Initializes the routing object for ORUCoP
 *
 * @param eid: The EID of the local node
 * 	      sdr: The pointer to local SDR model
 * 	      contactPlan: The pointer to the contact plan of the node
 * 	      dtn: A pointer to the local dtn module
 * 	      metricCollector: A pointer to the metricCollector
 * 	      bundleCopies: The amount of copies to be simulated
 * 	      repetition: The run number in the simulations
 * 	      numOfNodes: The amount of nodes in the network
 *
 * @author Simon Rink
 */
RoutingORUCOP::RoutingORUCOP(int eid, SdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                             MetricCollector *metricCollector, int bundleCopies, int repetition,
                             int numOfNodes)
    : RoutingOpportunistic(eid, sdr, contactPlan, dtn, metricCollector) {
    this->bundleCopies_ = bundleCopies;
    this->metricCollector_->setAlgorithm("ORUCoP");
    this->opportunistic = (this->metricCollector_->getMode() == 1);

    if (repetition > 0 && !opportunistic) {
        string pathToFiles = "sharedFolder/";
        for (int j = 1; j <= numOfNodes; j++) {
            for (int i = 1; i <= this->bundleCopies_; i++) {
                string filePath = pathToFiles + to_string(j) + "-" + to_string(i) + ".txt";
                ifstream file(filePath);

                if (file.good()) {
                    this->jsonFunction_[j][i] = json::parse(file);
                } else {
                    // cout << "An error happened trying to read the routing decisions for file " +
                    // filePath;
                }
            }
            this->updateStartTimes(this->contactPlan_->getContacts(), j);

            this->convertContactPlanIntoNet(j);
        }
    }
}

RoutingORUCOP::~RoutingORUCOP() {
    // TODO Auto-generated destructor stub
}

/*
 * Routes a newly received (or created) bundle. In case it is created, it is also copied for the
 * wanted amount.
 *
 * @param bundle: A pointer to the bundle
 *        simTime: The current simulation time
 *
 * @author Simon Rink
 */
void RoutingORUCOP::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    if (this->opportunistic) {
        this->contactPlan_->deleteOldContacts();
    }
    long originalBundleId = getOriginalBundleId(bundle->getBundleId());
    this->storedBundles_[originalBundleId].push(bundle);
    int destination = bundle->getDestinationEid();

    if (bundle->getSenderEid() == 0) {
        this->metricCollector_->updateStartedBundles(this->eid_, bundle->getBundleId(),
                                                     bundle->getSourceEid(),
                                                     bundle->getDestinationEid(), simTime);

        for (int i = 1; i < this->bundleCopies_;
             i++) { // we need to copy the bundle such that we have multicopy bundle support
            BundlePkt *copy = bundle->dup();
            copy->setBundlesCopies(1);
            originalBundleIds[copy->getBundleId()] = originalBundleId;
            this->storedBundles_[originalBundleId].push(copy);
        }
    }

    if (this->jsonFunction_.find(destination) ==
        this->jsonFunction_.end()) { // no routing has happened for this bundle so far
        this->callToPython(bundle);
        int bundlesToRoute = this->storedBundles_[originalBundleId].size();
        for (int i = 0; i < bundlesToRoute; i++) {
            this->routeNextBundle(originalBundleId, destination,
                                  this->getTsForStartOrCurrentTime(simTime, destination));
        }
    } else {
        if (this->contactPlan_->getLastEditTime().dbl() > this->lastUpdateTime_[destination]) {
            this->callToPython(bundle);
        }
        this->routeNextBundle(originalBundleId, destination,
                              this->getTsForStartOrCurrentTime(simTime, destination));
    }

    this->queueBundlesForCurrentContact();
}

/*
 * When a contact starts, all bundles are routes and queued, if they are supposed to be
 *
 * @param c: The started contact
 *
 * @author Simon Rink
 */
void RoutingORUCOP::contactStart(Contact *c) {
    if (c->isDiscovered()) {
        this->reRouteBundles();
    }

    this->updateRoutingDecisions();

    this->queueBundlesForCurrentContact();
}

/*
 * All bundles are queued to the current contact(s) if they're supposed to be, and rerouted if they
 * fail
 *
 * @author Simon Rink
 */
void RoutingORUCOP::queueBundlesForCurrentContact() {
    for (auto it = this->routingDecisions_.begin(); it != this->routingDecisions_.end(); it++) {

        int failedBundles = this->checkAndQueueBundles(it->first);

        if (failedBundles > 0) {
            if (this->storedBundles_[it->first].size() > 0) {

                this->reRouteFailedBundles(
                    it->first, this->storedBundles_[it->first].front()->getDestinationEid(),
                    failedBundles);
            }
        }
    }
}

/*
 * The receiver node is notified about the routing decisions
 *
 * @param jsonFunction: The routing table
 * 	      destination: The destination of the bundle
 * 	      idMap: The ID map of the sender
 * 	      tsStartTimes: The start times of the sender
 * 	      numOfTs: The number of timestamps of the sender
 *
 * @author Simon Rink
 */
void RoutingORUCOP::notifyAboutRouting(map<int, json> jsonFunction, int destination,
                                       map<int, int> idMap, vector<int> tsStartTimes, int numOfTs) {
    this->jsonFunction_[destination] = jsonFunction;
    if (this->bundleSolvedCopies_.find(destination) == this->bundleSolvedCopies_.end()) {
        this->bundleSolvedCopies_[destination] = 0;
    }
    this->tsStartTimes_[destination] = tsStartTimes;
    this->numOfTs_[destination] = numOfTs;
    this->idMap_[destination] = idMap;
    this->lastUpdateTime_[destination] = simTime().dbl();
}

/*
 * For each decision it is checked, whether it accounts to a contact currently active and/or whether
 * contact failed
 *
 * @param bundleId: The Id of the bundle
 *
 * @author Simon Rink
 */
int RoutingORUCOP::checkAndQueueBundles(long bundleId) {
    int failedBundles = 0;
    vector<queue<Contact>> routingDecisions = this->routingDecisions_[bundleId];
    for (size_t i = 0; i < routingDecisions.size(); i++) {
        if (this->storedBundles_[bundleId].size() == 0) {
            break;
        }
        queue<Contact> decision = routingDecisions.at(i);
        // check decisions
        if (decision.size() == 0) {
            continue;
        }

        Contact contact = decision.front();
        if (contact.getId() == -1) {
            continue;
        }
        Dtn *dtn;
        int destination = contact.getDestinationEid();
        int endDestination = this->storedBundles_[bundleId].front()->getDestinationEid();
        try {
            dtn = check_and_cast<Dtn *>(dtn_->getParentModule()
                                            ->getParentModule()
                                            ->getSubmodule("node", destination)
                                            ->getSubmodule("dtn"));
        } catch (...) {
            continue;
        }

        int contactId = dtn->checkExistenceOfContact(
            contact.getSourceEid(), contact.getDestinationEid(), contact.getStart());
        if (simTime().dbl() < contact.getEnd() && simTime().dbl() >= contact.getStart() &&
            destination != this->storedBundles_[bundleId].front()->getSenderEid()) {
            if (contactId != 0) // contact will happen now!
            {
                RoutingORUCOP *neighborRouting = check_and_cast<RoutingORUCOP *>(dtn->getRouting());
                if (neighborRouting->isDeliveredBundle(bundleId)) {
                    BundlePkt *bundle = this->storedBundles_[bundleId].front();
                    this->storedBundles_[bundleId].pop();
                    delete bundle;
                    continue;
                }
                if (decision.size() > 1) // multihop
                {
                    decision.pop();

                    neighborRouting->notifyAboutMultiHop(bundleId, decision);
                }
                neighborRouting->notifyAboutRouting(this->jsonFunction_[endDestination],
                                                    endDestination, this->idMap_[endDestination],
                                                    this->tsStartTimes_[endDestination],
                                                    this->numOfTs_[endDestination]);
                this->queueBundle(this->storedBundles_[bundleId].front(), contactId,
                                  contact.getDestinationEid());
                this->metricCollector_->updateSentBundles(this->eid_, contact.getDestinationEid(),
                                                          simTime().dbl(), bundleId);
                this->storedBundles_[bundleId].pop();
            } else // contact does not happen (failed, or mispredicted)
            {
                failedBundles++;
            }
        }
    }

    return failedBundles;
}

/*
 * All bundles with the passed ID are rerouted
 *
 * @param bundleId: The ID of the bundle
 *        destination: The destination of the bundle
 *        failedBundles: The number of failed bundles
 *
 * @author Simon Rink
 */
void RoutingORUCOP::reRouteFailedBundles(long bundleId, int destination, int failedBundles) {
    int nextTs = this->getTsForStartOrCurrentTime(simTime().dbl(), destination) + 1;

    while (failedBundles !=
           0) // as long as there are still failed routes continue to search for new opportunities
    {
        this->routingDecisions_[bundleId].clear();
        this->bundleSolvedCopies_[bundleId] = this->storedBundles_[bundleId].size() - failedBundles;

        for (int i = 0; i < failedBundles; i++) {
            this->routeNextBundle(bundleId, destination, nextTs);
        }

        failedBundles =
            this->checkAndQueueBundles(bundleId); // check whether there are still failed ones

        nextTs++; // check next timestamp
    }
}

/*
 * Notifies the receiver about a multi hop
 *
 * @param bundleId: The ID of the bundle
 * 	      hops: The queue of the hops, containing the respective contacts
 *
 * @author Simon Rink
 */
void RoutingORUCOP::notifyAboutMultiHop(long bundleId, queue<Contact> hops) {
    this->bundleSolvedCopies_[bundleId]++;
    this->routingDecisions_[bundleId].push_back(hops);
}

/**
 * Routes the next bundle from the unsolved ones (if any)
 *
 * @param bundleId: The ID of the bundle
 * 	      destination: The destination of the bundle
 * 	      ts: The current timestamp
 *
 * @author Simon Rink
 */
void RoutingORUCOP::routeNextBundle(long bundleId, int destination, int ts) {
    if (this->bundleSolvedCopies_[bundleId] >=
        this->storedBundles_[bundleId].size()) // do return if all decisions are already made!
    {
        return;
    }
    queue<Contact> nextContactDecision =
        this->getNextRoutingDecisionForBundle(bundleId, destination, ts);
    this->routingDecisions_[bundleId].push_back(nextContactDecision);

    this->bundleSolvedCopies_[bundleId]++;
}

/*
 * Reroutes all bundles that could not delivered using this contact
 *
 * @param c: The ended contact
 *
 * @authors The original authors of DTNSim, then ported to this class by Simon Rink
 */
void RoutingORUCOP::contactEnd(Contact *c) {
    while (sdr_->isBundleForContact(c->getId())) {
        BundlePkt *bundle = sdr_->getNextBundleForContact(c->getId());
        sdr_->popNextBundleForContact(c->getId());
        routeAndQueueBundle(bundle, simTime().dbl());
    }
}

/*
 * Checks whether a bundle was already delivered successfully
 *
 * @param bundleId: The ID of the bundle
 *
 * @authors The original authors of DTNSim, then ported to this class by Simon Rink
 */
bool RoutingORUCOP::isDeliveredBundle(long bundleId) {
    for (size_t i = 0; i < this->deliveredBundles_.size(); i++) {
        if (bundleId == this->deliveredBundles_[i]) {
            return true;
        }
    }

    return false;
}

/*
 * Accepts or rejects a delivered bundle with this node as destination
 *
 * @param bundle: The pointer to the received bundle
 *
 * @authors The original authors of DTNSim, then ported to this class by Simon Rink
 */
bool RoutingORUCOP::msgToMeArrive(BundlePkt *bundle) {
    if (!this->isDeliveredBundle(bundle->getBundleId())) {

        this->deliveredBundles_.push_back(bundle->getBundleId());
        this->metricCollector_->updateReceivedBundles(this->eid_, bundle->getBundleId(),
                                                      simTime().dbl());
        return true;
    }

    return false;
}

/*
 * The node is notified about the fact, that a bundle was successfully delivered
 *
 * @param bundleId: The ID of the bundle
 *        contact: The used contact
 *        sentToDestination: Identifier whether the delivery was towards to the bundle destination
 *
 * @authors The original authors of DTNSim, then ported to this class by Simon Rink
 */
void RoutingORUCOP::successfulBundleForwarded(long bundleId, Contact *contact,
                                              bool sentToDestination) {
    if (sentToDestination) {
        this->deliveredBundles_.push_back(bundleId);
    }
}

/*
 * Updates the routing tables for all bundles that currently reside at the node
 *
 * @author Simon Rink
 */
void RoutingORUCOP::reRouteBundles() {
    this->contactPlan_->deleteOldContacts();

    for (auto it = this->storedBundles_.begin(); it != this->storedBundles_.end(); it++) {
        if (it->second.size() != 0) // don't enter of no bundles are present!
        {
            this->callToPython(it->second.front());
        }
    }
}

/*
 * For all stored bundles, the respective decisions are obtained
 *
 * @author Simon Rink
 */
void RoutingORUCOP::updateRoutingDecisions() {

    for (auto it = this->storedBundles_.begin(); it != this->storedBundles_.end(); it++)

    {
        if (it->second.size() == 0) {
            continue;
        }

        // reset all values
        this->bundleSolvedCopies_[it->first] = 0;

        this->routingDecisions_[it->first].clear();

        int destination = it->second.front()->getDestinationEid();

        for (size_t i = 0; i < this->storedBundles_[it->first].size(); i++) {
            this->routeNextBundle(
                it->first, destination,
                this->getTsForStartOrCurrentTime(simTime().dbl(), destination)); // obtain decisions
        }
    }
}

/*
 * A bundle is queued to the given contact
 *
 * @param bundle: The pointer to the bundle
 * 	      contactId: The ID of the contact
 * 	      destinationEid: The EID of the next node
 *
 * @author Simon Rink
 */
void RoutingORUCOP::queueBundle(BundlePkt *bundle, int contactId, int destinationEid) {
    if (contactId != 0) {
        bundle->setNextHopEid(destinationEid);
    }

    this->sdr_->enqueueBundleToContact(bundle, contactId); // enqueue bundle
}

/*
 * The file for the RUCoP computation is created, thus all contact are splitted in case it is
 * necessary and then added
 *
 * @param endDestination: The destination of the bundle
 *
 * @author Simon Rink
 */
void RoutingORUCOP::convertContactPlanIntoNet(int endDestination) {
    ofstream networkFile("net.py");
    this->idMap_[endDestination].clear();
    int brufId = 1;
    int numOfNodes = this->dtn_->getParentModule()->getParentModule()->par("nodesNumber");

    networkFile << "NUM_OF_NODES = " << to_string(numOfNodes) << endl;

    string contactString = "CONTACTS = [";
    vector<Contact> *contacts = this->contactPlan_->getContacts();

    for (size_t i = 0; i < contacts->size(); i++) {
        int source = contacts->at(i).getSourceEid() - 1;
        int contactId = contacts->at(i).getId();
        int destination = contacts->at(i).getDestinationEid() - 1;
        vector<int> tsValues = getTsForContact(&contacts->at(i), endDestination);
        double pf = contacts->at(i).getFailureProbability();
        for (size_t i = 0; i < tsValues.size(); i++) // go through each splitted interval
        {
            contactString = contactString + "{'from': " + to_string(source) + ", ";
            contactString = contactString + "'to': " + to_string(destination) + ", ";
            contactString = contactString + "'ts': " + to_string(tsValues.at(i)) + ", ";
            contactString = contactString + "'pf': " + to_string(pf) + "}, ";
            this->idMap_[endDestination][brufId] = contactId;
            brufId++;
        }
    }

    contactString = contactString + "]";

    networkFile << contactString << endl;

    networkFile.close();
}

/*
 * Creates the initialization file for L-RUCoP
 *
 * @param source: The source of the bundle
 * 		  destination: The destination of the bundle
 *
 * @author Simon Rink
 */
void RoutingORUCOP::createIniFile(int source, int destination) {
    ofstream iniFile("ini.txt");
    iniFile << source - 1 << "," << destination - 1 << "," << this->bundleCopies_ << endl;
    iniFile.close();
}

/*
 * The call to L-RUCoP is performed here, thus the initialiation and the contact plan files are
 * created, L-RUCoP called and the results interpreted
 *
 * @param bundle: A pointer to the bundle in question
 *
 * @author Simon Rink
 */
void RoutingORUCOP::callToPython(BundlePkt *bundle) {
    char currDirectory[128];
    if (getcwd(currDirectory, sizeof(currDirectory)) == NULL) {
        perror("getcwd failed");
        return;
    }

    if (chdir("../../../") != 0) {
        perror("chdir failed");
        return;
    }

    if (opportunistic) {
        this->contactPlan_->deleteOldContacts();
    }

    this->updateStartTimes(this->contactPlan_->getContacts(), bundle->getDestinationEid());

    this->convertContactPlanIntoNet(bundle->getDestinationEid());
    this->createIniFile(this->eid_, bundle->getDestinationEid());

    auto start = high_resolution_clock::now();

    if (system("./run_ibruf.py") != 0) { //"venv/bin/python run_bruf.py"
        perror("run_ibruf failed");
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(stop - start);
    this->metricCollector_->updateRUCoPComputationTime(duration.count());
    this->readJsonFromFile(bundle->getDestinationEid(), bundle->getId());
    this->updateRoutingDecisions();

    try {
        fs::remove_all("working_dir");
        fs::remove("net.py");
        fs::remove("ini.txt");
    } catch (const fs::filesystem_error &e) {
        perror("remove files failed");
    }

    this->lastUpdateTime_[originalBundleIds[bundle->getId()]] = simTime().dbl();
    for (size_t i = 0; i < this->bundleCopies_; i++) {
        this->metricCollector_->updateRUCoPCalls(this->eid_); // one RUCoP call for each bundle
    }

    if (chdir(currDirectory) != 0) {
        perror("chdir back failed");
        return;
    }

    for (int i = 1; i <= this->bundleCopies_; i++) {
        ofstream jsonFile("sharedFolder/" + to_string(bundle->getDestinationEid()) + "-" +
                          to_string(i) + ".txt");
        jsonFile << this->jsonFunction_[bundle->getDestinationEid()][i] << endl;
    }
}

/*
 * Reads the json results from a call to L-RUCoP
 *
 * @param destination: The destination of the bundle
 * 	      bundleId: The ID of the bundle
 *
 * @author Simon Rink
 */
void RoutingORUCOP::readJsonFromFile(int destination, long bundleId) {
    string pathToFiles =
        "working_dir/IRUCoPn-" + to_string(this->bundleCopies_) + "/routing_files/pf=-1/todtnsim-";

    for (int i = 1; i <= this->bundleCopies_; i++) {
        string filePath =
            pathToFiles + to_string(destination - 1) + "-" + to_string(i) + "--1.json";
        ifstream file(filePath);

        if (file.good()) {
            this->jsonFunction_[destination][i] =
                json::parse(file); // save json for corresponding destination
        } else {
            cout << "An error happened trying to read the routing decisions for file " + filePath;
        }
    }
}

/*
 * Returns the timestamp for the given time, interpreted as contact start or current simulation time
 *
 * @param startOrCurrent: The given simulation or contact start time
 *        destination: The destination of the bundle
 *
 * @return The respective timestamp
 *
 * @author Simon Rink
 */
int RoutingORUCOP::getTsForStartOrCurrentTime(int startOrCurrent, int destination) {
    // go through all time stamps
    for (size_t i = 0; i < this->tsStartTimes_[destination].size() - 1; i++) {
        // We win if we either find the correct start time, or the simulation is in between two
        // timestamps, thus the lower one is important
        if (startOrCurrent == this->tsStartTimes_[destination].at(i) ||
            startOrCurrent < this->tsStartTimes_[destination].at(i + 1)) {
            return i;
        }
    }

    std::cerr << "Error: No timestamp found for start time " << startOrCurrent << " at destination "
              << destination << endl;
    return -1;
}

/*
 * Returns the timestamp for the given time, interpreted as contact end
 *
 * @param end: The end time of a contact
 *        destination: The destination of the bundle
 *
 * @return The respective time stamp
 *
 * @author Simon Rink
 */
int RoutingORUCOP::getTsForEndTime(int end, int destination) {
    // go through each element
    for (size_t i = 1; i < this->tsStartTimes_[destination].size(); i++) {
        // if we found a time stamp, it has to be the lower as we want the interval ended by this
        // contact
        if (end == this->tsStartTimes_[destination].at(i)) {
            return i - 1;
        }
    }

    std::cerr << "Error: No timestamp found for end time " << end << " at destination "
              << destination << endl;
    return -1;
}

/**
 * Updates the start times according to the contacts for RUCoP
 *
 * @param contacts: The contacts that are considered for the time stamps
 * 	      destination: The destination of the bundle
 *
 * @author Simon Rink
 */
void RoutingORUCOP::updateStartTimes(vector<Contact> *contacts, int destination) {
    map<int, int> alreadySeen;
    this->tsStartTimes_[destination].clear();

    for (size_t i = 0; i < contacts->size(); i++) {
        int start = contacts->at(i).getStart();
        int end = contacts->at(i).getEnd();
        if (alreadySeen.find(start) ==
            alreadySeen.end()) // timer must not be included several times
        {
            alreadySeen[start] = 1;
            this->tsStartTimes_[destination].push_back(start);
        }

        if (alreadySeen.find(end) == alreadySeen.end()) {
            alreadySeen[end] = 1;
            this->tsStartTimes_[destination].push_back(end);
        }
    }

    sort(this->tsStartTimes_[destination].begin(),
         this->tsStartTimes_[destination].begin() +
             this->tsStartTimes_[destination].size()); // must be sorted!
    this->numOfTs_[destination] = this->findMaxTs(destination);
}

/*
 * Returns the time intervals for a given contact
 *
 * @param contact: The respective contact pointer
 * 	      destination: The destination of the bundle
 *
 * @return A vector of time stamps
 *
 * @author Simon Rink
 */
vector<int> RoutingORUCOP::getTsForContact(Contact *contact, int destination) {
    int startTs = this->getTsForStartOrCurrentTime(contact->getStart(), destination);
    int endTs = this->getTsForEndTime(contact->getEnd(), destination);
    vector<int> tsValues;

    for (int i = startTs; i <= endTs; i++) // take each timestamp between start and end
    {
        tsValues.push_back(i);
    }

    return tsValues;
}

/*
 * Returns the maximum time stamps
 *
 * @param destination: The destination of the bundle
 *
 * @return The maximum time stamp
 *
 * @author Simon Rink
 */
int RoutingORUCOP::findMaxTs(int destination) {

    return this->tsStartTimes_[destination].size() - 1;
}

/*
 * Identifies the correct decisions for a given bundle copy
 *
 * @param bundleId: The ID of the bundle
 * 	      destination: The destination of the bundle
 *
 * @return The queue of chosen contacts
 *
 * @author Simon Rink
 */
queue<Contact> RoutingORUCOP::getNextRoutingDecisionForBundle(long bundleId, int destination,
                                                              int ts) {
    int solvedCopies = this->bundleSolvedCopies_[bundleId];
    queue<Contact> noDecisionPossible;

    try {

        json possibleActions =
            this->jsonFunction_[destination][this->storedBundles_[bundleId].size()];

        json currJson = possibleActions.at(to_string(ts));

        vector<json> action = currJson.at(to_string(this->eid_) + ":" +
                                          to_string(this->storedBundles_[bundleId].size()));

        for (auto it = action.begin(); it != action.end(); it++) {
            json curr = *it;
            int copies = curr.at("copies");
            solvedCopies = solvedCopies - copies;

            if (solvedCopies < 0) // we have found the correct position
            {
                vector<int> route = curr.at("route");
                return this->getDecisionFromRoute(route, destination);
            }
        }
    } catch (...) {
        return noDecisionPossible;
    }

    return noDecisionPossible;
}

/*
 * Returns for a given route from RUCoP the corresponding queue
 *
 * @param route: The route from RUCoP
 * 	      destination: The destination of the bundle
 *
 * @return The resulting decision queue
 *
 * @author Simon Rink
 */
queue<Contact> RoutingORUCOP::getDecisionFromRoute(vector<int> route, int destination) {
    queue<Contact> decisionQueue;
    for (size_t i = 0; i < route.size(); i++) {
        int actualId = this->idMap_[destination][route.at(i)];
        Contact *contact =
            this->contactPlan_->getContactById(actualId); // get corresponding ID from DTNSim
        if (contact == NULL) {
            continue;
        }
        decisionQueue.push(*contact);
    }

    return decisionQueue;
}

long getOriginalBundleId(long bundleId) {
    if (originalBundleIds.find(bundleId) == originalBundleIds.end()) {
        return bundleId;
    } else {
        return originalBundleIds[bundleId];
    }
}
