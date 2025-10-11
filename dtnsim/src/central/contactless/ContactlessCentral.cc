#include <iostream>
#include "ContactlessCentral.h"
#include "src/node/MsgTypes.h"
#include "src/node/app/App.h"
#include "src/node/com/Com.h"
#include "src/node/dtn/contactless/ContactlessDtn.h"
#include "src/node/dtn/routing/RoutingAntop.h"
#include "src/central/Central.h"

Define_Module(dtnsim::ContactlessCentral);

namespace dtnsim {

ContactlessCentral::ContactlessCentral() {}

ContactlessCentral::~ContactlessCentral() {}

void ContactlessCentral::initialize() {
    Central::initialize();

    for (int i = 0; i <= nodesNumber_; i++) {
        ContactlessDtn *dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));

        dtn->setMetricCollector(&metricCollector_);
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

    // TODO deberiamos hacer algo de esto?
    // schedule dummy event to make time pass until last potential contact.
    // This is mandatory in order for nodes
    // finish() method to be called and collect some statistics when
    // none contact is scheduled.
    

    for (int i = 0; i <= nodesNumber_; i++) {
        ContactlessDtn *dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));
        dtn->setRoutingAlgorithm(new Antop());

        Com *com = check_and_cast<Com *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("com"));
        //TODO set something to com?
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