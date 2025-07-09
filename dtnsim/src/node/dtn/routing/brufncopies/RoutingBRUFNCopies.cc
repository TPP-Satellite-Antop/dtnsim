/*
 * RoutingBRUFNCopies.cpp
 *
 *  Created on: December 12, 2018
 *  Author: FRaverta
 */

#include <src/node/dtn/routing/brufncopies/RoutingBRUFNCopies.h>

RoutingBRUFNCopies::RoutingBRUFNCopies(int eid, SdrModel *sdr, ContactPlan *contactPlan,
                                       int bundlesCopies, int numOfNodes, string pathPrefix,
                                       string pathPosfix)
    : RoutingDeterministic(eid, sdr, contactPlan) {
    oracle_ = BRUFNCopiesOracle::getInstance(numOfNodes, bundlesCopies, pathPrefix, pathPosfix);
    bundlesCopies_ = bundlesCopies;
    oracle_->addObserver(this);
}

RoutingBRUFNCopies::~RoutingBRUFNCopies() {
    auto it1 = copiesList_.begin();
    auto it2 = copiesList_.end();
    while (it1 != it2) {
        list<BundlePkt *> *bundles = &(it1->second);

        while (!bundles->empty()) {
            delete (bundles->back());
            bundles->pop_back();
        }
        copiesList_.erase(it1++);
    }
    BRUFNCopiesOracle::finish();
}

void RoutingBRUFNCopies::msgToOtherArrive(BundlePkt *bundle, double simTime) {
    // Check if it is a new message which was created in this node
    if (bundle->getHopCount() == 0 and bundle->getSourceEid() == this->eid_) {
        cout << "RoutingBRUFNCopies::msgToOtherArrive Node " << eid_ << ": Bundle " << bundle
             << "( " << bundlesCopies_ << " copies) was generated." << endl;

        int target = bundle->getDestinationEid();
        bundle->setBundlesCopies(bundlesCopies_); // Set the number of copies to current bundle
        enqueueToCarryingBundles(bundle);

        oracle_->createBundle(eid_, target, bundlesCopies_); // Notify oracle of this event
    } else {
        int source = bundle->getSourceEid();
        int target = bundle->getDestinationEid();
        int sender = bundle->getSenderEid();
        int copies = bundle->getBundlesCopies();

        // The bundle was transmitted from other node. So, it checks if the bundle has to be sent
        // now or if it has to be stored.
        CgrRoute *route = &(bundle->getCgrRouteForUpdate());
        if (route->terminusNode == eid_) {
            // Bundle has to be stored
            enqueueToCarryingBundles(bundle);
            oracle_->succesfulBundleForwarded(source, target, sender, eid_, copies,
                                              true); // Notify oracle of this event
        } else {
            // Send bundle throw the next hop in its route
            route->nextHop++; // Increment route next hop. Notice that this field is used as an
                              // index to enter in route->hops
            Contact *next_contact = route->hops.at(route->nextHop);
            if (next_contact->getSourceEid() != eid_) // Sanity check
                reportErrorAndExit("RoutingBRUFNCopies::msgToOtherArrive",
                                   "Error next hop's sender it not the current bundle carrier");

            bundle->setNextHopEid(next_contact->getDestinationEid());
            sdr_->enqueueBundleToContact(bundle, next_contact->getId());
            oracle_->succesfulBundleForwarded(source, target, sender, eid_, copies,
                                              false); // Notify oracle of this event
        }
    }
}

bool RoutingBRUFNCopies::msgToMeArrive(BundlePkt *bundle) {
    int source = bundle->getSourceEid();
    int sender = bundle->getSenderEid();
    int copies = bundle->getBundlesCopies();

    oracle_->succesfulBundleForwarded(source, eid_, sender, eid_, copies,
                                      true); // Notify oracle of this event

    /*
     *Check if current bundle has already sent to app, if not
     *it tells to dtn that bundle should be delivered to app and
     *save it as delivered bundle.
     */
    if (!isDeliveredBundle(bundle->getBundleId())) {
        deliveredBundles_.push_back(bundle->getBundleId());
        return true;
    }
    return false;
}

void RoutingBRUFNCopies::contactEnd(Contact *c) {
    while (sdr_->isBundleForContact(c->getId())) {
        BundlePkt *bundle = sdr_->getNextBundleForContact(c->getId());
        int source = bundle->getSourceEid();
        int target = bundle->getDestinationEid();
        int copies = bundle->getBundlesCopies();

        sdr_->popNextBundleForContact(c->getId());
        enqueueToCarryingBundles(bundle);
        cout << "RoutingBRUFNCopies::contactEnd - Node " << eid_ << ": Enqueue to carrying "
             << source << " " << target << endl;
        oracle_->failBundleForwarded(source, target, copies); // Notify oracle
    }
}

/**
 * This method enqueue the bundle in node's sdr_ and make bundle copies,
 * which could be send in the future.
 *
 * Note: The bundle must not be used after calling this method because
 * it might be deleted.
 */
void RoutingBRUFNCopies::enqueueToCarryingBundles(BundlePkt *bundle) {
    BundlePkt *storedBundle = sdr_->getEnqueuedBundle(bundle->getBundleId());
    // Check if node is already carrying copies of this bundle
    if (storedBundle != nullptr) {
        cout << storedBundle << endl;
        generateCopies(bundle);
        storedBundle->setBundlesCopies(storedBundle->getBundlesCopies() +
                                       bundle->getBundlesCopies()); // update copies number
        delete bundle;
    } else {
        generateCopies(bundle);
        sdr_->enqueueBundle(bundle);
    }
}

/**
 * Observer update method. It is called by oracle after a change in a Markov Chain status.
 * If there are any route decision to make, it will perform the corresponding routing.
 */
void RoutingBRUFNCopies::update() {
    int source;
    int target;
    cout << " RoutingBRUFNCopies::update() - Node " << eid_ << endl;
    vector<tuple<const int, const vector<int>>> routing_decisions =
        this->oracle_->getRoutingDecision(eid_, source, target);
    if (routing_decisions.size() > 0) {
        BundlePkt *bundle = getCarryingBundle(source, target);
        if (bundle == nullptr) {
            ostringstream stream;
            stream << "There not exist bundle from " << source << " to " << target;
            reportErrorAndExit("RoutingBRUFNCopies::update()", stream.str());
        }
        for (auto it = routing_decisions.begin(); it != routing_decisions.end(); ++it) {
            tuple<const int, const vector<int>> action = *it;
            routeBundle(bundle, get<0>(action), get<1>(action));
        }
        if (bundle->getBundlesCopies() == 0)
            sdr_->removeBundle(bundle->getBundleId());
    }
}

/**
 * Send bundle throw the selected route
 */
void RoutingBRUFNCopies::routeBundle(BundlePkt *bundle, int copies, vector<int> route) {
    Contact *first_hop_contact = contactPlan_->getContactById(route.at(0));
    Contact *last_hop_contact = contactPlan_->getContactById(route.back());

    // generate the route
    CgrRoute cgrRoute;
    cgrRoute.nextHop = 0;
    cgrRoute.terminusNode = last_hop_contact->getDestinationEid();
    for (auto contact_it = route.begin(); contact_it != route.end(); ++contact_it)
        cgrRoute.hops.push_back(contactPlan_->getContactById(*contact_it));

    // Sanity check, node must have enough copies of bundle to be sent
    if (copies <= bundle->getBundlesCopies()) {
        bundle->setBundlesCopies(bundle->getBundlesCopies() - copies);
        BundlePkt *bundleCopy =
            getCopiesToSend(bundle->getBundleId(), copies); // Get corresponding bundles copies
        bundleCopy->setNextHopEid(first_hop_contact->getDestinationEid());
        bundleCopy->setCgrRoute(cgrRoute);
        if (first_hop_contact->getSourceEid() != eid_) {
            ostringstream stream;
            stream << "Error routing bundle " << bundle
                   << " . First hop contact sender is not who is performing this routing decision.";
            reportErrorAndExit("RoutingBRUFNCopies::routeBundle", stream.str());
        }
        sdr_->enqueueBundleToContact(bundleCopy, first_hop_contact->getId());

    } else
        reportErrorAndExit("RoutingBRUFNCopies::routeBundle",
                           "bundles has least copies than the required");
}

/**
 * This method makes bundle copies and stores these in copiesList_
 *
 * Bundles need to be copied beforehand a cause of an intrinsic aspect of OMNET: when call dup()
 * method it writes inside of the bundle copy information about which module it was updating. But,
 * sometimes we need to copy bundles while OMNET is updating other node, since that node trigger a
 * change in Markov Chain.
 */
void RoutingBRUFNCopies::generateCopies(BundlePkt *bundle) {
    map<long, list<BundlePkt *>>::iterator it = copiesList_.find(bundle->getBundleId());
    list<BundlePkt *> *lista = new list<BundlePkt *>;
    if (it == copiesList_.end()) {
        copiesList_.insert(std::pair<long, list<BundlePkt *>>(bundle->getBundleId(), *lista));
        lista = &(copiesList_.find(bundle->getBundleId())->second);
    } else
        lista = &(it->second);

    for (int i = 0; i < bundle->getBundlesCopies(); i++) {
        BundlePkt *bundleCopy = bundle->dup();
        bundleCopy->setBundlesCopies(1);
        lista->push_back(bundleCopy);
    }
}

BundlePkt *RoutingBRUFNCopies::getCopiesToSend(long bundle_id, int copies) {
    // Sanity check
    if (copies <= 0)
        reportErrorAndExit("RoutingBRUFNCopies::getCopiesToSend", "copies must be greater than 0");

    map<long, list<BundlePkt *>>::iterator it = copiesList_.find(bundle_id);
    if (it == copiesList_.end())
        reportErrorAndExit("RoutingBRUFNCopies::getCopiesToSend",
                           "Error trying to send a bundle without having copies");

    list<BundlePkt *> *list = &(it->second);
    if (list->size() < (unsigned)copies) // sanity check
        reportErrorAndExit("RoutingBRUFNCopies::getCopiesToSend",
                           "Error trying to send more copies than those it is carrying");

    BundlePkt *bundleCopy = list->front();
    list->pop_front();
    for (int i = 0; i < copies - 1; i++) {
        delete list->front();
        list->pop_front();
    }

    if (list->size() == 0)
        copiesList_.erase(bundle_id);

    bundleCopy->setBundlesCopies(copies);
    return bundleCopy;
}

/**
 * Get carrying bundle by source and destination.
 * It check if node is carrying a bundle generated by source and destined to target.
 * If it finds one the bundle is returned otherwise it returns nullptr.
 */
BundlePkt *RoutingBRUFNCopies::getCarryingBundle(int source, int target) {
    list<BundlePkt *> carryingBundles = sdr_->getCarryingBundles();
    for (list<BundlePkt *>::iterator it = carryingBundles.begin(); it != carryingBundles.end();
         ++it) {
        if ((*it)->getSourceEid() == source && (*it)->getDestinationEid() == target)
            return *(it);
    }
    return nullptr;
}

void RoutingBRUFNCopies::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    cout << "RoutingBRUFNCopies::routeAndQueueBundle it should not happen";
    exit(1);
}

string RoutingBRUFNCopies::reportErrorAndExit(string method, string msg) {
    cout << method << " - node " << eid_ << ": " << msg << endl;
    exit(1);
}

/**
 * Check if bundleId has been delivered to App.
 */
bool RoutingBRUFNCopies::isDeliveredBundle(long bundleId) {
    for (list<int>::iterator it = deliveredBundles_.begin(); it != deliveredBundles_.end(); ++it)
        if ((*it) == bundleId)
            return true;
    return false;
}
