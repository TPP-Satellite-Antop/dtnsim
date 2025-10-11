#include "ContactPlanCentral.h"
#include "src/node/MsgTypes.h"
#include "src/node/app/App.h"
#include "src/node/com/contactplan/ContactPlanCom.h"
#include "src/node/dtn/contactplan/ContactSdrModel.h"
#include "src/node/dtn/contactplan/Dtn.h"

#include <iostream>
#include <src/central/Central.h>


Define_Module(dtnsim::ContactPlanCentral);

namespace dtnsim {

ContactPlanCentral::ContactPlanCentral() {}

ContactPlanCentral::~ContactPlanCentral() {}

void ContactPlanCentral::initialize() {
    Central::initialize();

    // Initialize contact plan
    contactPlan_.parseContactPlanFile(par("contactsFile"), nodesNumber_, this->par("mode"),
                                      this->par("failureProbability"));

    // Initialize topology
    int mode = this->par("mode");
    if (mode < 2) {
        contactTopology_.parseOpportunisticContactPlanFile(par("contactsFile"), nodesNumber_, mode,
                                                           this->par("failureProbability"));
    } else {
        contactTopology_.parseContactPlanFile(par("contactsFile"), nodesNumber_, 2,
                                              this->par("failureProbability"));
    }

    bool faultsAware = this->par("faultsAware");
    int deleteNIds = this->par("deleteNNodes");
    if (deleteNIds > 0) {
        // delete N contacts
        vector<int> idsToDelete;
        if (par("useCentrality")) {
            idsToDelete = getCentralityNodeIds(deleteNIds, nodesNumber_);
        } else {
            idsToDelete = getRandomNodeIds(deleteNIds);
        }

        deleteNodes(idsToDelete, faultsAware);
    } else if (this->par("useSpecificFailureProbabilities")) {
        vector<int> idsToDelete;

        idsToDelete = this->getNodeIdsWithSpecificFProb();
        deleteNodes(idsToDelete, false);
    } else {
        double failureProbability = this->par("failureProbability");
        vector<int> idsToDelete;
        if (failureProbability > 0) {
            idsToDelete = getRandomNodeIdsWithFProb(failureProbability);
            deleteNodes(idsToDelete, faultsAware);
        } else {
            string toDeleteIds = this->par("nodeIdsToDelete");
            stringstream stream(toDeleteIds);
            vector<int> idsToDelete;
            if (toDeleteIds != "") {
                while (1) {
                    int n;
                    stream >> n;
                    idsToDelete.push_back(n);
                    if (!stream)
                        break;
                }
            }
            if (idsToDelete.size() > 0) {
                deleteNodes(idsToDelete, faultsAware);
            }
        }
    }

    // schedule dummy event to make time pass until
    // last potential contact. This is mandatory in order for nodes
    // finish() method to be called and collect some statistics when
    // none contact is scheduled.
    if (!contactTopology_.getContacts()->empty()) {
        double topologyEndTime = contactTopology_.getContacts()->back().getEnd();
        ContactMsg *contactMsgEnd = new ContactMsg("contactEnd", CONTACT_END_TIMER);
        contactMsgEnd->setSchedulingPriority(CONTACT_END_TIMER);
        scheduleAt(topologyEndTime, contactMsgEnd);
    }

    // emit nodesNumber statistic
    nodesNumber = registerSignal("nodesNumber");
    emit(nodesNumber, contactPlan_.getContacts()->size());

    if (par("enableAvailableRoutesCalculation")) {
        int totalRoutesVar = 0;
        set<pair<int, int>> nodePairsRoutes1;
        totalRoutesVar =
            this->computeTotalRoutesNumber(contactPlan_, nodesNumber_, nodePairsRoutes1);

        // emit totalRoutes statistic
        totalRoutes = registerSignal("totalRoutes");
        emit(totalRoutes, totalRoutesVar);
    }

    // setting modified contact plan and contact topology
    // to each node
    for (int i = 0; i <= nodesNumber_; i++) {
        Dtn *dtn = check_and_cast<Dtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));
        dtn->setContactPlan(contactPlan_);
        dtn->setContactTopology(contactTopology_);

        ContactPlanCom *com = check_and_cast<ContactPlanCom *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("com"));
        com->setContactTopology(contactTopology_);
    }

    if (par("enableAvailableRoutesCalculation")) {
        int availableRoutesVar = 0;
        set<pair<int, int>> nodePairsRoutes2;
        availableRoutesVar =
            this->computeTotalRoutesNumber(contactPlan_, nodesNumber_, nodePairsRoutes2);

        // emit availableRoutes statistic
        availableRoutes = registerSignal("availableRoutes");
        emit(availableRoutes, availableRoutesVar);

        // emit pairsOfNodesWithAtLeastOneRoute statistic
        pairsOfNodesWithAtLeastOneRoute = registerSignal("pairsOfNodesWithAtLeastOneRoute");
        int pairsOfNodesWithAtLeastOneRouteVar = nodePairsRoutes2.size();
        emit(pairsOfNodesWithAtLeastOneRoute, pairsOfNodesWithAtLeastOneRouteVar);
    }
}

void ContactPlanCentral::finish() {
    Central::finish();
    if (this->nodesNumber_ >= 1) {
        if (this->par("saveTopology")) {
            this->saveTopology();
        }

        if (this->par("saveFlows")) {
            this->saveFlows();
        }

        if (this->par("saveLpFlows")) {
            this->saveLpFlows();
        }
    }
}

void ContactPlanCentral::saveTopology() {
#ifdef USE_BOOST_LIBRARIES
    map<double, TopologyGraph> topology =
        topologyUtils::computeTopology(&this->contactTopology_, nodesNumber_);
    topologyUtils::saveGraphs(&topology, "results/topology.dot");
#endif
}

void ContactPlanCentral::saveFlows() {
#ifdef USE_BOOST_LIBRARIES
    this->computeFlowIds();
    vector<string> dotColors = routerUtils::getDotColors();
    map<double, RouterGraph> flows =
        routerUtils::computeFlows(&this->contactTopology_, nodesNumber_, "results");
    routerUtils::saveGraphs(&this->contactTopology_, &flows, dotColors, flowIds_,
                            "results/flows.dot");
#endif
}

void ContactPlanCentral::saveLpFlows() {
#ifdef USE_BOOST_LIBRARIES
#ifdef USE_CPLEX_LIBRARY
    this->computeFlowIds();
    vector<string> dotColors = routerUtils::getDotColors();

    // traffic[k][k1][k2] generated in state k by commodity k1-k2
    map<int, map<int, map<int, double>>> traffic = getTraffics();

    Lp lp(&this->contactTopology_, nodesNumber_, traffic);

    lp.exportModel("results/lpModel");
    bool solved = lp.solve();

    if (solved) {
        map<double, RouterGraph> flows =
            lpUtils::computeFlows(&this->contactTopology_, nodesNumber_, &lp);
        routerUtils::saveGraphs(&this->contactTopology_, &flows, dotColors, flowIds_,
                                "results/lpFlows.dot");
    }

#endif
#endif
}

double ContactPlanCentral::getState(double trafficStart) {
    // compute state times
    vector<double> stateTimes;
    stateTimes.push_back(0);

    vector<Contact> *contacts = contactTopology_.getContacts();
    vector<Contact>::iterator it1 = contacts->begin();
    vector<Contact>::iterator it2 = contacts->end();
    for (; it1 != it2; ++it1) {
        Contact contact = *it1;

        stateTimes.push_back(contact.getStart());
        stateTimes.push_back(contact.getEnd());

        std::sort(stateTimes.begin(), stateTimes.end());
        stateTimes.erase(std::unique(stateTimes.begin(), stateTimes.end()), stateTimes.end());
    }

    // get state of traffic generation
    double state = 0;
    vector<double>::iterator iit1 = stateTimes.begin();
    vector<double>::iterator iit2 = stateTimes.end();
    for (; iit1 != iit2; ++iit1) {
        double stateStart = *iit1;
        double stateEnd = stateTimes.back();

        ++iit1;
        if (iit1 != iit2) {
            stateEnd = *iit1;
        }
        --iit1;

        if (trafficStart >= stateStart && trafficStart < stateEnd) {
            state = stateStart;
        }
    }

    return state;
}

void ContactPlanCentral::deleteNodes(vector<int> contactsToDelete, bool faultsAware) {
    for (size_t i = 0; i < contactsToDelete.size(); i++) {
        int contactId = contactsToDelete.at(i);

        contactTopology_.deleteContactById(contactId);

        // if faultsAware we delete contacts also from contactPlan
        // so CGR will take better decisions
        if (faultsAware) {
            contactPlan_.deleteContactById(contactId);
        }
    }
}

vector<int> ContactPlanCentral::getRandomNodeIds(int nContacts) {
    vector<int> contactIds;

    // get working copy of contactPlan_
    ContactPlan workCP(contactPlan_);

    int contactsDeleted = 0;
    while (contactsDeleted < nContacts) {
        vector<Contact> *contacts = workCP.getContacts();

        if (contacts->size() == 0) {
            break;
        }

        int randomPosition = intuniform(0, contacts->size() - 1);
        contactIds.push_back(contacts->at(randomPosition).getId());

        contacts->erase(contacts->begin() + randomPosition);

        contactsDeleted++;
    }

    return contactIds;
}

vector<int> ContactPlanCentral::getRandomNodeIdsWithFProb(double failureProbability) {
    vector<int> contactIds;

    // get working copy of contactPlan_
    ContactPlan workCP(contactPlan_);
    vector<Contact> *contacts = workCP.getContacts();

    for (unsigned int i = 0; i < contacts->size(); i++) {
        double randomNumber = uniform(0, 1);
        if (randomNumber < failureProbability)
            contactIds.push_back(contacts->at(i).getId());
    }

    return contactIds;
}

vector<int> ContactPlanCentral::getNodeIdsWithSpecificFProb() {
    vector<int> contactIds;

    ContactPlan workCP(this->contactPlan_);
    ContactPlan workCT(this->contactTopology_);
    vector<Contact> *contacts = workCT.getContacts();

    for (size_t i = 0; i < contacts->size(); i++) {
        double randomNumber = uniform(0, 1, 0.5);
        if (randomNumber < contacts->at(i).getFailureProbability()) {
            contactIds.push_back(contacts->at(i).getId());
        }
    }

    return contactIds;
}

vector<int> ContactPlanCentral::getCentralityNodeIds(int nContacts, int nodesNumber) {
    // cout<<"delete central contacts"<<endl;

    vector<int> contactIds;

    // get working copy of contactPlan_
    ContactPlan workCP(this->contactPlan_);

    // contact id -> centrality
    map<int, double> centralityMap;

    int contactsDeleted = 0;
    while (contactsDeleted < nContacts) {
        vector<Contact> *contacts = workCP.getContacts();
        if (contacts->size() == 0) {
            break;
        }

        for (size_t i = 0; i < contacts->size(); i++) {
            centralityMap[contacts->at(i).getId()] = 0;
        }

        for (int i = 1; i <= nodesNumber; i++) {
            // por ahora inicializamos el contact pero dps esto deberia cambiar en base a una config
            int eid = i;
            auto sdr = ContactSdrModel(eid, nodesNumber, &workCP);
            bool printDebug = false;

            Routing *routing = new RoutingCgrModel350(eid, &sdr, &workCP, printDebug);

            for (int j = 1; j <= nodesNumber; j++) {
                if (i != j) {
                    // cout<<"!!! source = "<<i<<endl;
                    // cout<<"!!! destination = "<<j<<endl;

                    BundlePkt *bundle = new BundlePkt("bundle", BUNDLE);
                    bundle->setSchedulingPriority(BUNDLE);
                    bundle->setBitLength(0);
                    bundle->setByteLength(0);
                    bundle->setSourceEid(i);
                    bundle->setDestinationEid(j);
                    bundle->setReturnToSender(true);
                    bundle->setCritical(false);
                    bundle->setTtl(1000000);
                    bundle->setCreationTimestamp(simTime());
                    bundle->setHopCount(0);
                    bundle->setNextHopEid(0);
                    bundle->setSenderEid(0);
                    bundle->getVisitedNodesForUpdate().clear();
                    CgrRoute emptyRoute;
                    emptyRoute.nextHop = EMPTY_ROUTE;
                    bundle->setCgrRoute(emptyRoute);

                    vector<CgrRoute> routes =
                        check_and_cast<RoutingCgrModel350 *>(routing)->getCgrRoutes(
                            bundle, simTime().dbl());
                    vector<CgrRoute> shortestPaths;

                    time_t bestArrivalTime = 10000000;
                    for (size_t k = 0; k < routes.size(); k++) {
                        CgrRoute route = routes.at(k);
                        if (route.arrivalTime == bestArrivalTime) {
                            shortestPaths.push_back(route);
                        } else if (route.arrivalTime < bestArrivalTime) {
                            shortestPaths.clear();
                            shortestPaths.push_back(route);
                            bestArrivalTime = route.arrivalTime;
                        }
                    }
                    // cout<<"routes size = "<<routes.size()<<endl;
                    // cout<<"sp size = "<<shortestPaths.size()<<endl;

                    set<int> affectedContacts = this->getAffectedContacts(shortestPaths);
                    set<int>::iterator cit1 = affectedContacts.begin();
                    set<int>::iterator cit2 = affectedContacts.end();
                    for (; cit1 != cit2; ++cit1) {
                        int cId = *cit1;
                        int routesThroughContact =
                            computeNumberOfRoutesThroughContact(cId, shortestPaths);
                        double centrality =
                            (double)routesThroughContact / (double)shortestPaths.size();
                        centralityMap[cId] += centrality;
                    }

                    delete bundle;
                }
            }

            delete routing;
        }

        map<int, double>::iterator x =
            max_element(centralityMap.begin(), centralityMap.end(),
                        [](const pair<int, double> &p1, const pair<int, double> &p2) {
                            return p1.second < p2.second;
                        });

        // choose random contact among all max centrality contacts
        vector<pair<int, double>> allMaxElements;
        map<int, double>::iterator ii1 = centralityMap.begin();
        map<int, double>::iterator ii2 = centralityMap.end();
        for (; ii1 != ii2; ++ii1) {
            if (ii1->second == (*x).second) {
                allMaxElements.push_back(make_pair(ii1->first, ii1->second));
            }
        }

        int randomPosition = intuniform(0, allMaxElements.size() - 1);

        //		cout<<"centralityMap = "<<endl;
        //	    map<int, double>::iterator i1 = centralityMap.begin();
        //	    map<int, double>::iterator i2 = centralityMap.end();
        //	    for(; i1 != i2; ++i1)
        //	    {
        //	    	cout<<i1->first<<" "<<i1->second<<endl;
        //	    }

        int contactToDelete = allMaxElements.at(randomPosition).first;
        contactIds.push_back(contactToDelete);
        workCP.deleteContactById(contactToDelete);

        //		cout<<"##### contact deleted = "<<contactIds.back()<<endl;
        //		cout<<endl;

        contactsDeleted++;
        centralityMap.clear();
    }

    return contactIds;
}

/// @brief compute total routes from all to all nodes
int ContactPlanCentral::computeTotalRoutesNumber(ContactPlan &contactPlan, int nodesNumber,
                                      set<pair<int, int>> &nodePairsRoutes) {
    int totalRoutesNumber = 0;

    // get working copy of contactPlan_
    ContactPlan workCP(this->contactPlan_);

    vector<Contact> *contacts = workCP.getContacts();
    if (contacts->size() == 0) {
        return 0;
    }

    for (int i = 1; i <= nodesNumber; i++) {
        int eid = i;
        auto sdr = ContactSdrModel(eid, nodesNumber, &workCP);
        bool printDebug = false;

        Routing *routing = new RoutingCgrModel350(eid, &sdr, &workCP, printDebug);

        for (int j = 1; j <= nodesNumber; j++) {
            if (i != j) {
                // cout<<"!!! source = "<<i<<endl;
                // cout<<"!!! destination = "<<j<<endl;

                BundlePkt *bundle = new BundlePkt("bundle", BUNDLE);
                bundle->setSchedulingPriority(BUNDLE);
                bundle->setBitLength(0);
                bundle->setByteLength(0);
                bundle->setSourceEid(i);
                bundle->setDestinationEid(j);
                bundle->setReturnToSender(true);
                bundle->setCritical(false);
                bundle->setTtl(1000000);
                bundle->setCreationTimestamp(simTime());
                bundle->setHopCount(0);
                bundle->setNextHopEid(0);
                bundle->setSenderEid(0);
                bundle->getVisitedNodesForUpdate().clear();
                CgrRoute emptyRoute;
                emptyRoute.nextHop = EMPTY_ROUTE;
                bundle->setCgrRoute(emptyRoute);

                vector<CgrRoute> routes =
                    check_and_cast<RoutingCgrModel350 *>(routing)->getCgrRoutes(bundle,
                                                                                simTime().dbl());

                if (!routes.empty()) {
                    nodePairsRoutes.insert(make_pair(i, j));
                }

                totalRoutesNumber += routes.size();

                delete bundle;
            }
        }

        delete routing;
    }

    return totalRoutesNumber;
}

int ContactPlanCentral::computeNumberOfRoutesThroughContact(int contactId, vector<CgrRoute> shortestPaths) {
    int numberOfRoutesThroughContact = 0;
    for (size_t r = 0; r < shortestPaths.size(); r++) {
        CgrRoute route = shortestPaths.at(r);
        vector<Contact *> hops = route.hops;
        for (size_t h = 0; h < hops.size(); h++) {
            Contact *contact = hops.at(h);
            if (contact->getId() == contactId) {
                numberOfRoutesThroughContact++;
            }
        }
    }

    return numberOfRoutesThroughContact;
}

set<int> ContactPlanCentral::getAffectedContacts(vector<CgrRoute> shortestPaths) {
    set<int> affectedContacts;
    for (size_t r = 0; r < shortestPaths.size(); r++) {
        CgrRoute route = shortestPaths.at(r);
        vector<Contact *> hops = route.hops;
        for (size_t h = 0; h < hops.size(); h++) {
            affectedContacts.insert(hops.at(h)->getId());
        }
    }

    return affectedContacts;
}

} // namespace dtnsim
