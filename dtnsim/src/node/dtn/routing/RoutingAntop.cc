#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"
#include "antop.h"

RoutingAntop::RoutingAntop(int eid, SdrModel *sdr, ContactPlan *contactPlan): RoutingDeterministic(eid, sdr, contactPlan) {
    this->antopAlgorithm = new Antop();
    this->nodePositions = contactPlan->getNodePositions();
    this->antopAlgorithm->init(contactPlan->getNodesNumber());
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    int nextHopId = this->antopAlgorithm->getNextHopId(
        bundle->getSourceEid(),
        bundle->getDestinationEid(),
        this->prevSrc,
        this->isNextHopValid
    );

    if (nextHopId != 0) {
        bundle->setNextHopEid(contactPlan_->getContactById(nextHopId)->getDestinationEid());
    }

    sdr_->enqueueBundleToContact(bundle, nextHopId);
}

bool RoutingAntop::isNextHopValid(H3Index nextHop) const {
    double currTime = simTime().dbl();

    for (const auto& [interval, positions] : this->nodePositions) { //TODO maybe change this data structure
        if (currTime < interval.tStart || currTime >= interval.tEnd)
            continue;

        for (const auto& pos : positions) {
            H3Index cell;
            latLngToCell(&pos.latLng, this->antopAlgorithm->getResolution(), &cell);
            if (cell == nextHop) {
                return true;
            }
        }
    }

    return false;
}

