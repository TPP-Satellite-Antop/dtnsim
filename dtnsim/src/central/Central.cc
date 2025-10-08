#include "src/node/MsgTypes.h"
#include "src/node/app/App.h"
#include "src/node/com/Com.h"
#include "src/node/dtn/contactplan/Dtn.h"
#include "src/node/dtn/contactplan/ContactSdrModel.h"

#include <src/central/Central.h>


Define_Module(dtnsim::Central);

namespace dtnsim {

Central::Central() {}

Central::~Central() {}

void Central::initialize() {
    nodesNumber_ = this->getParentModule()->par("nodesNumber");

    this->metricCollector_.initialize(nodesNumber_);
    this->metricCollector_.setMode(this->par("mode"));
    this->metricCollector_.setFailureProb(this->par("failureProbability"));
    this->metricCollector_.setPath(this->par("collectorPath"));

    for (int i = 0; i <= nodesNumber_; i++) {
        Dtn *dtn = check_and_cast<Dtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));

        dtn->setMetricCollector(&metricCollector_);
    }
}

void Central::finish() {
    if (nodesNumber_ >= 1) {
        this->metricCollector_.evaluateAndPrintResults();
    }
}

void Central::handleMessage(cMessage *msg) {
    // delete dummy msg
    delete msg;
}

void Central::computeFlowIds() {
    int flowId = 0;

    for (int i = 1; i <= nodesNumber_; i++) {
        App *app = check_and_cast<App *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("app"));

        int src = i;
        vector<int> dsts = app->getDestinationEidVec();

        vector<int>::iterator it1 = dsts.begin();
        vector<int>::iterator it2 = dsts.end();
        for (; it1 != it2; ++it1) {
            int dst = *it1;
            pair<int, int> pSrcDst(src, dst);

            map<pair<int, int>, unsigned int>::iterator iit = flowIds_.find(pSrcDst);
            if (iit == flowIds_.end()) {
                flowIds_[pSrcDst] = flowId++;
            }
        }
    }
}

map<int, map<int, map<int, double>>> Central::getTraffics() {
    /// traffic[k][k1][k2] tráfico generado en el estado k desde el nodo k1 hacia el nodo k2
    map<int, map<int, map<int, double>>> traffics;

    for (int i = 1; i <= nodesNumber_; i++) {
        App *app = check_and_cast<App *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("app"));

        int src = i;

        vector<int> bundlesNumberVec = app->getBundlesNumberVec();
        vector<int> destinationEidVec = app->getDestinationEidVec();
        vector<int> sizeVec = app->getSizeVec();
        vector<double> startVec = app->getStartVec();

        for (size_t j = 0; j < bundlesNumberVec.size(); j++) {
            double stateStart = this->getState(startVec.at(j));

            int dst = destinationEidVec.at(j);

            double totalSize = bundlesNumberVec.at(j) * sizeVec.at(j);

            map<int, map<int, map<int, double>>>::iterator it1 = traffics.find(stateStart);
            if (it1 != traffics.end()) {
                map<int, map<int, double>> m1 = it1->second;
                map<int, map<int, double>>::iterator it2 = m1.find(src);

                if (it2 != m1.end()) {
                    map<int, double> m2 = it2->second;
                    map<int, double>::iterator it3 = m2.find(dst);

                    if (it3 != m2.end()) {
                        m2[dst] += totalSize;
                    } else {
                        m2[dst] = totalSize;
                    }
                    m1[src] = m2;
                } else {
                    map<int, double> m2;
                    m2[dst] = totalSize;
                    m1[src] = m2;
                }
                traffics[stateStart] = m1;
            } else {
                map<int, map<int, double>> m1;
                m1[src][dst] = totalSize;
                traffics[stateStart] = m1;
            }
        }
    }

    return traffics;
}

double Central::getState(double trafficStart) {
    return 0; // Dummy implementation
}

void Central::deleteNodes(vector<int> toDelete, bool faultsAware) {}

vector<int> Central::getRandomNodeIds(int n) {
    return vector<int>(); // Dummy implementation
}

vector<int> Central::getRandomNodeIdsWithFProb(double failureProbability) {
    return vector<int>(); // Dummy implementation
}

vector<int> Central::getNodeIdsWithSpecificFProb() {
    return vector<int>(); // Dummy implementation
}

vector<int> Central::getCentralityNodeIds(int n, int nodesNumber) {
    return vector<int>(); // Dummy implementation
}

} // namespace dtnsim
