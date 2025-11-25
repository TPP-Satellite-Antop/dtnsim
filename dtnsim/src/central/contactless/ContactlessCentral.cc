#include <iostream>
#include "ContactlessCentral.h"
#include "src/central/Central.h"
#include "src/node/MsgTypes.h"
#include "src/node/app/App.h"
#include "src/node/com/Com.h"
#include "src/node/dtn/contactless/ContactlessDtn.h"
#include "src/node/dtn/routing/RoutingAntop.h"

Define_Module(dtnsim::ContactlessCentral);

namespace dtnsim {

ContactlessCentral::ContactlessCentral() {}

ContactlessCentral::~ContactlessCentral() {}

void ContactlessCentral::initialize() {
    Central::initialize();

    this->metricCollector_.setAlgorithm("ANTOP");
    for (int i = 0; i <= nodesNumber_; i++) {
        auto dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));

        dtn->setMetricCollector(&metricCollector_);
    }

    const bool faultsAware = this->par("faultsAware");
    const int deleteNIds = this->par("deleteNNodes");
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

        const vector<int> idsToDelete = this->getNodeIdsWithSpecificFProb();
        deleteNodes(idsToDelete, false);
    } else {
        if (const double failureProbability = this->par("failureProbability");
            failureProbability > 0) {
            const vector<int> idsToDelete = getRandomNodeIdsWithFProb(failureProbability);
            deleteNodes(idsToDelete, faultsAware);
        } else {
            string toDeleteIds = this->par("nodeIdsToDelete"); 
            stringstream stream(toDeleteIds);
            vector<int> idsToDelete;
            if (toDeleteIds != "") {
                while (1) {
                    int n;
                    stream >> n; //todo bug
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

    // TODO deberiamos hacer algo de esto?
    // schedule dummy event to make time pass until last potential contact.
    // This is mandatory in order for nodes
    // finish() method to be called and collect some statistics when
    // none contact is scheduled.

    auto antop = new Antop();
    antop->init(nodesNumber_);
    auto* mobilityMap = new std::map<int, inet::SatelliteMobility*>();
    for (int i = 0; i <= nodesNumber_; i++) { //todo check
        auto dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()
                ->getSubmodule("node", i)
                ->getSubmodule("dtn")
        );
        dtn->setRoutingAlgorithm(antop);
        dtn->setMobilityMap(mobilityMap);
    }
}

double ContactlessCentral::getState(double trafficStart) {
    return 0.0;
}

void ContactlessCentral::deleteNodes(vector<int> nodesToDelete, bool faultsAware) {
}

vector<int> ContactlessCentral::getRandomNodeIds(int n) {
    return vector<int>();
}

vector<int> ContactlessCentral::getRandomNodeIdsWithFProb(double failureProbability) {
    return vector<int>();
}

vector<int> ContactlessCentral::getNodeIdsWithSpecificFProb() {
    return vector<int>();
}

vector<int> ContactlessCentral::getCentralityNodeIds(int n, int nodesNumber) {
    return vector<int>();
}

} // namespace dtnsim