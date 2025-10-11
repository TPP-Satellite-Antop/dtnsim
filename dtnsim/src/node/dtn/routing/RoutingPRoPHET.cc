/*
 * RoutingPRoPHET.cc
 *
 *  Created on: Sep 13, 2022
 *      Author: simon
 */

#include "RoutingPRoPHET.h"
#include "src/node/dtn/contactplan/ContactDtn.h"

RoutingPRoPHET::RoutingPRoPHET(int eid, SdrModel *sdr, cModule *dtn, float p_enc_max,
                               float p_enc_first, float p_first_thresh, float forw_thresh,
                               float alpha, float beta, float gamma, float delta, int numOfNodes,
                               MetricCollector *metricCollector)
    : Routing(eid, sdr) {
    this->dtn_ = dtn;
    this->alpha_ = alpha;
    this->beta_ = beta;
    this->gamma_ = gamma;
    this->delta_ = delta;
    this->p_encouter_max_ = p_enc_max;
    this->p_encouter_first_ = p_enc_first;
    this->p_first_threshold_ = p_first_thresh;
    this->forw_thresh_ = forw_thresh;
    this->pred_table_.assign(numOfNodes + 1, 0);
    this->pred_table_[this->eid_] = 1; // trivial case
    this->inactive_intervals_.assign(numOfNodes + 1, vector<double>(0));
    this->last_seen_.assign(numOfNodes + 1, 0);
    this->last_updated_.assign(numOfNodes + 1, 0);
    this->metricCollector_ = metricCollector;
    this->metricCollector_->setAlgorithm("PRoPHET");
}

RoutingPRoPHET::~RoutingPRoPHET() {
    // TODO Auto-generated destructor stub
}

void RoutingPRoPHET::contactStart(Contact *c) {
    this->updateAging(c->getDestinationEid(), c->getStart());
    this->currently_active_[c->getDestinationEid()] = c->getId();
    this->updatePredToNode(c->getDestinationEid(), c->getStart());
    this->updateTransitivity(c->getDestinationEid(), c->getStart());
    this->routeAndQueueBundle(c);
}

void RoutingPRoPHET::contactEnd(Contact *c) {
    // unsent bundles are supposed to be not needed anymore
    while (sdr_->isBundleForId(c->getId())) {
        BundlePkt *bundle = sdr_->getBundle(c->getId());
        sdr_->popBundleFromId(c->getId());
        this->metricCollector_->updateSentBundles(eid_, c->getDestinationEid(), c->getStart(),
                                                  bundle->getBundleId(), -1);

        delete bundle;
    }
    this->last_seen_[c->getDestinationEid()] = simTime().dbl();
    this->last_updated_[c->getDestinationEid()] = simTime().dbl();
    this->currently_active_.erase(c->getDestinationEid());
}

void RoutingPRoPHET::updatePredToNode(int destinationEid, double simTime) {

    if (this->last_seen_[destinationEid] ==
        simTime) // contact just finished (even possible in a real case?)
    {
        return;
    }
    // check whether contact already happened or the prop of it is too low
    if (this->pred_table_[destinationEid] < this->p_first_threshold_) {
        this->pred_table_[destinationEid] = this->p_encouter_first_;
        double interval = simTime - this->last_seen_[destinationEid];
        this->last_seen_[destinationEid] = simTime;
        this->last_updated_[destinationEid] = simTime;
        this->inactive_intervals_[destinationEid].push_back(interval);
    } else {
        double interval = simTime - this->last_seen_[destinationEid];
        this->inactive_intervals_[destinationEid].push_back(interval);
        double I_typ = accumulate(this->inactive_intervals_[destinationEid].begin(),
                                  this->inactive_intervals_[destinationEid].end(), 0.0) /
                       this->inactive_intervals_[destinationEid].size();
        double p_enc = 0;
        if (interval <= I_typ) {
            p_enc = this->p_encouter_max_ * (interval / I_typ);
        } else {
            p_enc = this->p_encouter_max_;
        }
        double p_old = this->pred_table_[destinationEid];
        this->pred_table_[destinationEid] = p_old + (1 - this->delta_ - p_old) * p_enc;
    }
}

void RoutingPRoPHET::updateTransitivity(int destinationEid, double simTime) {
    ContactDtn *dtn_b = check_and_cast<ContactDtn *>(dtn_->getParentModule()
                                           ->getParentModule()
                                           ->getSubmodule("node", destinationEid)
                                           ->getSubmodule("dtn"));
    RoutingPRoPHET *routing_b = check_and_cast<RoutingPRoPHET *>(dtn_b->getRouting());
    vector<double> *pred_table_b = routing_b->getPredTable();
    for (size_t i = 1; i < this->pred_table_.size(); i++) {
        if (i != this->eid_ && i != destinationEid) {
            this->updateAging(i, simTime);
            routing_b->updateAging(i, simTime);
            double p_old = this->pred_table_[i];
            double transitive =
                this->beta_ * this->pred_table_[destinationEid] * (*pred_table_b)[i];

            this->pred_table_[i] = (p_old < transitive) ? transitive : p_old;
        }
    }
}

bool RoutingPRoPHET::msgToMeArrive(BundlePkt *bundle) {
    /*
     *Check if current bundle has already sent to app, if not
     *it tells to dtn that bundle should be delivered to app and
     *save it as delivered bundle.
     */
    if (!isDeliveredBundle(bundle->getBundleId())) {
        deliveredBundles_.push_back(bundle->getBundleId());
        this->metricCollector_->updateReceivedBundles(this->eid_, bundle->getBundleId(),
                                                      simTime().dbl());
        return true;
    }
    return false;
}

// used when a contact starts
void RoutingPRoPHET::routeAndQueueBundle(Contact *c) {

    RoutingPRoPHET *other = check_and_cast<RoutingPRoPHET *>(
        check_and_cast<ContactDtn *>(dtn_->getParentModule()
                                  ->getParentModule()
                                  ->getSubmodule("node", c->getDestinationEid())
                                  ->getSubmodule("dtn"))
            ->getRouting());

    list<BundlePkt *> carryingBundles = sdr_->getCarryingBundles();
    vector<double> *other_table = other->getPredTable();
    for (auto it = carryingBundles.begin(); it != carryingBundles.end(); it++) {
        if (other->isDeliveredBundle((*it)->getBundleId())) {
            deliveredBundles_.push_back((*it)->getBundleId());
            sdr_->popBundle((*it)->getBundleId());
            continue;
        }
        if (other->isCarryingBundle((*it)->getBundleId())) {
            continue;
        }
        if ((*it)->getDestinationEid() == c->getDestinationEid()) {
            // always send bundle to destination if not received already!
            BundlePkt *bundleCopy = (*it)->dup();
            bundleCopy->setNextHopEid(c->getDestinationEid());
            bundleCopy->setBundlesCopies(1);
            sdr_->pushBundleToId(bundleCopy, c->getId());
            this->metricCollector_->updateSentBundles(eid_, c->getDestinationEid(), c->getStart(),
                                                      bundleCopy->getBundleId());

        } else // target is not the destination!
        {
            // GTHR
            double other_pred = (*other_table)[(*it)->getDestinationEid()];
            if (other_pred > this->pred_table_[(*it)->getDestinationEid()] ||
                other_pred > this->forw_thresh_) {
                BundlePkt *bundleCopy = (*it)->dup();
                bundleCopy->setNextHopEid(c->getDestinationEid());
                bundleCopy->setBundlesCopies(1);
                sdr_->pushBundleToId(bundleCopy, c->getId());
                this->metricCollector_->updateSentBundles(eid_, c->getDestinationEid(),
                                                          c->getStart(), bundleCopy->getBundleId());
            }
        }
    }
}

// used when a bundle was received during a contact
void RoutingPRoPHET::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    this->updateAging(bundle->getDestinationEid(), simTime);
    for (auto it = this->currently_active_.begin(); it != this->currently_active_.end(); it++) {
        RoutingPRoPHET *other = check_and_cast<RoutingPRoPHET *>(
            check_and_cast<ContactDtn *>(dtn_->getParentModule()
                                      ->getParentModule()
                                      ->getSubmodule("node", it->first)
                                      ->getSubmodule("dtn"))
                ->getRouting());
        other->updateAging(bundle->getDestinationEid(), simTime);
        vector<double> *other_table = other->getPredTable();
        if (other->isDeliveredBundle(bundle->getBundleId())) {
            deliveredBundles_.push_back(bundle->getBundleId());
            sdr_->popBundle(bundle->getBundleId());
            break;
        }
        if (other->isCarryingBundle(bundle->getBundleId())) {
            continue;
        }
        if (bundle->getDestinationEid() == it->first) {
            // always send bundle to destination if not received already!
            BundlePkt *bundleCopy = bundle->dup();
            bundleCopy->setNextHopEid(it->first);
            bundleCopy->setBundlesCopies(1);
            sdr_->pushBundleToId(bundleCopy, it->second);
            this->metricCollector_->updateSentBundles(eid_, it->first, simTime,
                                                      bundleCopy->getBundleId());

        } else // target is not the destination!
        {
            // GTHR
            double other_pred = (*other_table)[bundle->getDestinationEid()];
            if (other_pred > this->pred_table_[bundle->getDestinationEid()] ||
                other_pred > this->forw_thresh_) {
                BundlePkt *bundleCopy = bundle->dup();
                bundleCopy->setNextHopEid(it->first);
                bundleCopy->setBundlesCopies(1);
                sdr_->pushBundleToId(bundleCopy, it->second);
                this->metricCollector_->updateSentBundles(eid_, it->first, simTime,
                                                          bundleCopy->getBundleId());
            }
        }
    }
}

/***
 * Check if bundle successful forwarded is a delivered to its destination,
 * if that happens, it deletes this from bundles queue since there is no sense
 * in carrying a delivered to destination bundle.
 */
void RoutingPRoPHET::successfulBundleForwarded(long bundleId, Contact *contact,
                                               bool sentToDestination) {
    // Check if bundle was deliver to its final destination.
    // Note it is necessary to check if bundle is enqueue since bundle could be sent
    // to its final destination and deleted from queue while it was sending to another relay node.
    BundlePkt *bundle = sdr_->getEnqueuedBundle(bundleId);
    if (bundle != NULL && bundle->getDestinationEid() == contact->getDestinationEid()) {
        sdr_->popBundle(bundleId);
        deliveredBundles_.push_back(bundleId);
    }
}

void RoutingPRoPHET::refreshForwarding(Contact *c) {
    routeAndQueueBundle(c);
}

void RoutingPRoPHET::updateContactPlan(Contact *c) {
    // not required
}

/**
 * Check if it is carrying a bundle with bundleId equals to bundleId.
 */
bool RoutingPRoPHET::isCarryingBundle(long bundleId) {
    list<BundlePkt *> carryingBundles = sdr_->getCarryingBundles();
    for (list<BundlePkt *>::iterator it = carryingBundles.begin(); it != carryingBundles.end();
         ++it) {
        if ((*it)->getBundleId() == bundleId)
            return true;
    }
    return false;
}

/**
 * Check if bundleId has been delivered to App.
 */
bool RoutingPRoPHET::isDeliveredBundle(long bundleId) {
    for (list<int>::iterator it = deliveredBundles_.begin(); it != deliveredBundles_.end(); ++it)
        if ((*it) == bundleId)
            return true;
    return false;
}

void RoutingPRoPHET::msgToOtherArrive(BundlePkt *bundle, double simTime) {
    if (bundle->getSourceEid() == eid_) {
        this->metricCollector_->updateStartedBundles(eid_, bundle->getBundleId(),
                                                     bundle->getSourceEid(),
                                                     bundle->getDestinationEid(), simTime);
    }
    if (!isCarryingBundle(bundle->getBundleId()) and !isDeliveredBundle(bundle->getBundleId())) {
        sdr_->pushBundle(bundle);
        this->routeAndQueueBundle(bundle, simTime);
    } else
        delete bundle;
}

vector<double> *RoutingPRoPHET::getPredTable() {
    return &this->pred_table_;
}

void RoutingPRoPHET::updateAging(int destinationEid, double simTime) {
    if (destinationEid == this->eid_) {
        // probability of sending a bundle to yourself always 1
        return;
    }
    if (this->currently_active_.find(destinationEid) != this->currently_active_.end()) {
        // active contacts don't need to age
        return;
    }
    double p_old = this->pred_table_[destinationEid];
    double diff = simTime - this->last_updated_[destinationEid]; // diff always an integer
                                                                 // (normally)
    this->pred_table_[destinationEid] = p_old * pow(this->gamma_, diff);
    this->last_updated_[destinationEid] = simTime;
}
