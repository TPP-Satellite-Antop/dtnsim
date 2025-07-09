/*
 * RoutingUniboCgr.cc
 *
 *  Created on: Dec 2, 2021
 *      Author: simon
 */

#include "RoutingUniboCgr.h"
#include "src/node/dtn/routing/unibocgr/core/bundles/bundles.h"
#include "src/node/dtn/routing/unibocgr/core/cgr/cgr_phases.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/contactPlan.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/nodes/nodes.h"
#include "src/node/dtn/routing/unibocgr/core/library/list/list.h"
#include <src/node/dtn/Dtn.h>

static SdrModel *sdrUnibo;

int getTotalEVC(vector<int> sizes);

RoutingUniboCgr::RoutingUniboCgr(int eid, SdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                                 MetricCollector *metricCollector)
    : RoutingOpportunistic(eid, sdr, contactPlan, dtn, metricCollector) {
    sdrUnibo = this->sdr_;
    if (eid == 0) {
        return;
    }
    int result = this->initializeUniboCGR(0);

    // system("gnome-terminal -- cd ../../../ ;venv/bin/python run_bruf.py");

    cout << "UniboCGR initialized with" << result << endl;
}

RoutingUniboCgr::~RoutingUniboCgr() {

    // TODO Auto-generated destructor stub
}

void RoutingUniboCgr::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    List cgrRoutes = NULL;
    ListElt *elt;
    int numberOfRoutes = 1;

    int result = this->callUniboCGR(simTime, bundle, &cgrRoutes);
    cout << "Status of Call" << result << endl;
    // routes were found!
    if (result >= 0 && cgrRoutes != NULL) {
        // enqueue the bundle to every route
        for (elt = cgrRoutes->first; elt != NULL && result >= 0; elt = elt->next) {

            if (elt->data != NULL) {

                Route *route = (Route *)elt->data;

                if (numberOfRoutes == 1) {

                    bundle->setDlvConfidence(
                        bundle->getDlvConfidence() +
                        (1 - (1 - route->arrivalConfidence) * (1 - bundle->getDlvConfidence())));

                    if (bundle->getDlvConfidence() >= 1.0) {
                        bundle->setDlvConfidence(1.0);
                    }

                    this->enqueueBundle(bundle, simTime, route);

                    if (bundle->getDlvConfidence() >= 0.8) {
                        break;
                    }

                    numberOfRoutes++;

                } else {
                    BundlePkt *bundleCopy = bundle->dup();

                    bundleCopy->setDlvConfidence(bundleCopy->getDlvConfidence() +
                                                 (1 - (1 - route->arrivalConfidence) *
                                                          (1 - bundleCopy->getDlvConfidence())));

                    if (bundleCopy->getDlvConfidence() >= 1.0) {
                        bundleCopy->setDlvConfidence(1.0);
                    }

                    this->enqueueBundle(bundleCopy, simTime, route);

                    if (bundleCopy->getDlvConfidence() >= 0.8) {
                        break;
                    }

                    numberOfRoutes++;
                }

            } else {
                break;
            }
        }
    } else {
        bundle->setNextHopEid(0);
        this->sdr_->enqueueBundleToContact(bundle, 0);
    }
}

void RoutingUniboCgr::enqueueBundle(BundlePkt *bundle, double simTime, Route *route) {
    int sourceEid = this->eid_;
    int destinationEid = route->neighbor;
    double start = route->fromTime;

    Contact *contactDtnSim =
        this->contactPlan_->getContactBySrcDstStart(sourceEid, destinationEid, start);

    int id = contactDtnSim->getId();

    if (id != 0) {
        bundle->setNextHopEid(this->contactPlan_->getContactById(id)->getDestinationEid());
    }

    this->sdr_->enqueueBundleToContact(bundle, id);
}

int RoutingUniboCgr::convertBundlePktToCgrBundle(time_t time, BundlePkt *bundle,
                                                 CgrBundle *cgrBundle) {
    if (bundle == NULL || cgrBundle == NULL) {
        return -1;
    }

    cgrBundle->id.source_node = bundle->getSourceEid();
    cgrBundle->id.creation_timestamp = bundle->getCreationTimestamp().dbl() - this->referenceTime_;
    cgrBundle->id.sequence_number = bundle->getBundleId();
    cgrBundle->id.fragment_length = 0;
    cgrBundle->id.fragment_offset = 0;
    cgrBundle->size = bundle->getByteLength();
    cgrBundle->dlvConfidence = bundle->getDlvConfidence();
    cgrBundle->evc = computeBundleEVC(bundle->getByteLength());
    if (bundle->getCritical()) {
        cgrBundle->priority_level = Expedited;
    } else {
        cgrBundle->priority_level = Normal;
    }
    cgrBundle->sender_node = bundle->getSenderEid();
    cgrBundle->regionNbr = this->defaultRegionNbr_;
    cgrBundle->terminus_node = bundle->getDestinationEid();
    cgrBundle->expiration_time = bundle->getTtl().dbl() + cgrBundle->id.creation_timestamp;

    return 0;
}

int RoutingUniboCgr::addContactToSap(Contact *contact) {
    int result = addContact(this->defaultRegionNbr_, contact->getSourceEid(),
                            contact->getDestinationEid(), contact->getStart(), contact->getEnd(),
                            contact->getDataRate(), contact->getConfidence(), 0, NULL, 0, false);

    if (result <= 0) {
        return result;
    }

    result = addRange(contact->getSourceEid(), contact->getDestinationEid(), contact->getStart(),
                      contact->getEnd(), contact->getRange());

    return result;
}

int RoutingUniboCgr::removeContactFromSap(Contact *contact) {
    int result = 0;

    result = removeContact(this->defaultRegionNbr_, contact->getSourceEid(),
                           contact->getDestinationEid(), NULL);

    if (result <= 0) {
        return result;
    }

    result = removeRange(contact->getSourceEid(), contact->getDestinationEid(), NULL);

    return result;
}

void RoutingUniboCgr::updateContactPlan(Contact *c) {
    // this->populateContactPlan(); return; //(much) slower, but working
    this->restoreSAPvalues();
    ContactPlanSAP cpSAP = get_contact_plan_sap(NULL);

    if (c == NULL) {
        for (int i = this->lastId_ + 1; i <= this->contactPlan_->getHighestId(); i++) {
            this->addContactToSap(this->contactPlan_->getContactById(i));
        }
    } else {
        this->removeContactFromSap(c);
    }

    cpSAP.contactPlanEditTime.tv_sec = simTime().dbl();
    cpSAP.contactPlanEditTime.tv_usec = simTime().inUnit(SIMTIME_US);
    set_time_contact_plan_updated(cpSAP.contactPlanEditTime.tv_sec,
                                  cpSAP.contactPlanEditTime.tv_usec);

    get_contact_plan_sap(&cpSAP); // save

    this->saveSAPs();

    this->lastId_ = this->contactPlan_->getHighestId();
}

int RoutingUniboCgr::populateContactPlan() {
    this->restoreSAPvalues();
    int result = 1;
    int addResult = 1;
    ContactPlanSAP cpSAP = get_contact_plan_sap(NULL);
    reset_contact_plan();

    vector<Contact> *contacts = this->contactPlan_->getContacts();

    for (size_t i = 0; i < (*contacts).size(); i++) {
        addResult = this->addContactToSap(&((*contacts).at(i)));
        cout << "Contact from" << (*contacts).at(i).getSourceEid() << "with" << addResult << endl;
        if (addResult <= 0) { // contact was not added successfully
            result = 0;
        }
    }

    // update last edit time
    cpSAP.contactPlanEditTime.tv_sec = simTime().dbl();
    cpSAP.contactPlanEditTime.tv_usec = simTime().inUnit(SIMTIME_US);
    set_time_contact_plan_updated(cpSAP.contactPlanEditTime.tv_sec,
                                  cpSAP.contactPlanEditTime.tv_usec);

    get_contact_plan_sap(&cpSAP); // save

    this->saveSAPs();

    this->lastId_ = contacts->size();

    return result;
}

int RoutingUniboCgr::initializeUniboCGR(time_t time) {
    int result = 1;
    this->initializeSAPValues();
    this->restoreSAPvalues();

    if (!this->initialised_) {
        this->excludedNeighbors_ = list_create(NULL, NULL, NULL, free);
        this->cgrBundle_ = bundle_create();

        if (this->cgrBundle_ != NULL && this->excludedNeighbors_ != NULL) {
            result = initialize_cgr(0, this->eid_, true);

            if (result == 1) {
                this->initialised_ = true;
                this->referenceTime_ = time;
                this->saveSAPs();

                if (this->populateContactPlan() <= 0) {
                    result = -2;
                }

                this->saveSAPs();
            }
        } else { // something went wrong
            free_list(this->excludedNeighbors_);
            bundle_destroy(this->cgrBundle_);
            this->excludedNeighbors_ = NULL;
            this->cgrBundle_ = NULL;
            result = -2;
        }
    }

    return result;
}

int RoutingUniboCgr::callUniboCGR(time_t time, BundlePkt *bundle, List *cgrRoutes) {
    int result;
    this->restoreSAPvalues();
    sdrUnibo = this->sdr_;

    if (this->initialised_ && bundle != NULL) {
        result = this->convertBundlePktToCgrBundle(time, bundle, this->cgrBundle_);

        if (result >= 0) {
            result = getBestRoutes(time - this->referenceTime_, this->cgrBundle_,
                                   this->excludedNeighbors_, cgrRoutes);
            cout << "Result of Best routes:" << result << endl;
            if (result > 0 && cgrRoutes != NULL) {
                result = 0;

            } else {
                result = -3;
            }
        } else {
            result = -2;
        }

    } else {
        result = -1;
    }
    reset_bundle(this->cgrBundle_);
    this->saveSAPs();
    return result;
}

void RoutingUniboCgr::initializeSAPValues() {
    // set important values in certains SAPs to ensure correct initialization
    this->rangeGraphSAP.ranges = NULL;
    this->neighborsSAP.local_node_neighbors = NULL;
    this->neighborsSAP.neighbors_list_builded = 0;
    this->neighborsSAP.timeNeighborToRemove = -1;
    this->contactGraphSAP.contacts = NULL;
    this->contactPlanSAP.initialized = 0;
    this->contactPlanSAP.contactsGraph = 0;
    this->contactPlanSAP.nodes = 0;
    this->contactPlanSAP.rangesGraph = 0;
    this->contactPlanSAP.contactPlanEditTime.tv_sec = -1;
    this->contactPlanSAP.contactPlanEditTime.tv_sec = -1;
    this->phaseOneSAP.excludedNeighbors = NULL;
    this->phaseTwoSAP.routes = NULL;
    this->phaseTwoSAP.subset = NULL;
    this->phaseTwoSAP.suppressedNeighbors = NULL;
}

void RoutingUniboCgr::saveSAPs() {
    // update local SAPs
    this->contactGraphSAP = *get_contact_graph_sap(NULL);
    this->contactPlanSAP = get_contact_plan_sap(NULL);
    this->neighborsSAP = *get_neighbors_sap(NULL);
    this->rangeGraphSAP = *get_range_graph_sap(NULL);
    this->nodesRbt = *get_node_graph(NULL);
    this->uniboCgrSAP = get_unibo_cgr_sap(NULL);
    this->uniboCgrCurrentCallSAP = *get_current_call_sap(NULL);
    this->phaseOneSAP = *get_phase_one_sap(NULL);
    this->phaseTwoSAP = *get_phase_two_sap(NULL);
}

void RoutingUniboCgr::restoreSAPvalues() {
    // overwrite all SAPs with the addresses of the local SAPs
    get_contact_graph_sap(&this->contactGraphSAP);
    get_contact_plan_sap(&this->contactPlanSAP);
    get_neighbors_sap(&this->neighborsSAP);
    get_range_graph_sap(&this->rangeGraphSAP);
    get_node_graph(&this->nodesRbt);
    get_unibo_cgr_sap(&this->uniboCgrSAP);
    get_current_call_sap(&this->uniboCgrCurrentCallSAP);
    get_phase_one_sap(&this->phaseOneSAP);
    get_phase_two_sap(&this->phaseTwoSAP);
}

void RoutingUniboCgr::successfulBundleForwarded(long bundleId, Contact *contact,
                                                bool sentToDestination) {
    if (sentToDestination) {
        this->deliveredBundles_.push_back(bundleId);
    }
}

bool RoutingUniboCgr::isDeliveredBundle(long bundleId) {
    for (int i = 0; i < this->deliveredBundles_.size(); i++) {
        if (bundleId == this->deliveredBundles_[i]) {
            return true;
        }
    }

    return false;
}

bool RoutingUniboCgr::msgToMeArrive(BundlePkt *bundle) {
    if (!this->isDeliveredBundle(bundle->getId())) {

        this->deliveredBundles_.push_back(bundle->getId());
        return true;
    }

    return false;
}

void RoutingUniboCgr::contactStart(Contact *c) {
    // try out whether a new route exists for a newly discovered contact
    if (c->isDiscovered()) {
        while (this->sdr_->isBundleForContact(0)) {
            BundlePkt *bundle = this->sdr_->getNextBundleForContact(0);
            this->sdr_->popNextBundleForContact(0);
            this->routeAndQueueBundle(bundle, simTime().dbl());
        }
    }

    // check whether some bundles were already received at their respective destinations!
    RoutingUniboCgr *other = check_and_cast<RoutingUniboCgr *>(
        check_and_cast<Dtn *>(dtn_->getParentModule()
                                  ->getParentModule()
                                  ->getSubmodule("node", c->getDestinationEid())
                                  ->getSubmodule("dtn"))
            ->getRouting());

    list<BundlePkt *> nonReceived;
    while (sdr_->isBundleForContact(c->getId())) {
        BundlePkt *bundle = sdr_->getNextBundleForContact(c->getId());
        sdr_->popNextBundleForContact(c->getId());
        if (!other->isDeliveredBundle(bundle->getBundleId())) {
            nonReceived.push_back(bundle);
        } else {
            delete bundle;
        }
    }

    for (list<BundlePkt *>::iterator it = nonReceived.begin(); it != nonReceived.end(); ++it)
        sdr_->enqueueBundleToContact(*it, c->getId());
}

void RoutingUniboCgr::contactEnd(Contact *c) {
    while (sdr_->isBundleForContact(c->getId())) {
        BundlePkt *bundle = sdr_->getNextBundleForContact(c->getId());
        sdr_->popNextBundleForContact(c->getId());
        routeAndQueueBundle(bundle, simTime().dbl());
    }
}
