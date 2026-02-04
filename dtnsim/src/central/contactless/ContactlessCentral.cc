#include <iostream>
#include "ContactlessCentral.h"
#include "src/central/Central.h"
#include "src/node/app/App.h"
#include "src/node/dtn/contactless/ContactlessDtn.h"
#include "Antop.h"

Define_Module(dtnsim::ContactlessCentral);

namespace dtnsim {

ContactlessCentral::ContactlessCentral() = default;

ContactlessCentral::~ContactlessCentral() = default;

const double BASE_DATA_RATE = 100;
const double DATA_RATE_INCREMENT = 10;

void ContactlessCentral::initialize() {
    Central::initialize();

    this->metricCollector_.setAlgorithm("ANTOP");
    for (int i = 0; i <= nodesNumber_; i++) {
        auto dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("dtn"));

        dtn->setMetricCollector(&metricCollector_);

        auto app = check_and_cast<App *>(
            this->getParentModule()->getSubmodule("node", i)->getSubmodule("app"));
        app->setMetricCollector(&metricCollector_);
    }

    auto antop = new Antop(nodesNumber_);
    auto* mobilityMap = new std::map<int, inet::SatelliteMobility*>();
    for (int i = 0; i <= nodesNumber_; i++) { // todo: i = 1?
        const auto dtn = check_and_cast<ContactlessDtn *>(
            this->getParentModule()
                ->getSubmodule("node", i)
                ->getSubmodule("dtn")
        );
        dtn->setRoutingAlgorithm(antop);
        dtn->setMobilityMap(mobilityMap);
        dtn->setDataRate(BASE_DATA_RATE + i);
    }
}

double ContactlessCentral::getState(double trafficStart) {
    return 0.0;
}

void ContactlessCentral::deleteNodes(vector<int> nodesToDelete, bool faultsAware) {
}

vector<int> ContactlessCentral::getRandomNodeIds(int n) {
    return {};
}

vector<int> ContactlessCentral::getRandomNodeIdsWithFProb(double failureProbability) {
    return {};
}

vector<int> ContactlessCentral::getNodeIdsWithSpecificFProb() {
    return {};
}

vector<int> ContactlessCentral::getCentralityNodeIds(int n, int nodesNumber) {
    return {};
}

void ContactlessCentral::finish() {
    if (this->nodesNumber_ >= 1) {
        cout << "Central: Evaluating and printing results..." << endl;
        this->metricCollector_.evaluateAndPrintJsonResults();
    }
}

} // namespace dtnsim