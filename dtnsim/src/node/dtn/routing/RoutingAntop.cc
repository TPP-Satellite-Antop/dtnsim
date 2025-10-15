#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(Antop* antop, int eid, SdrModel *sdr): RoutingDeterministic(eid, sdr, nullptr) { //TODO check this null
    this->prevSrc = 0;
    this->antopAlgorithm = antop;
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    printf("RoutingAntop::routeAndQueueBundle called for bundle %ld from %d to %d\n", bundle->getBundleId(), bundle->getSourceEid(), bundle->getDestinationEid());

    H3Index srcIndex = getH3IndexFromEid(bundle->getSourceEid());
    H3Index nextHopIndex = this->antopAlgorithm->getNextHopId(
        srcIndex,
        getH3IndexFromEid(bundle->getDestinationEid()),
        this->prevSrc,
        [this](H3Index idx){ return this->isNextHopValid(idx); }
    );

    int nextHopEid = getEidFromH3Index(nextHopIndex);
    cout << "Next eid: " << nextHopEid << endl;
    if (nextHopEid != -1) {
        this->prevSrc = srcIndex;
       // bundle->setNextHopEid(nextHopEid);
    }

    //todo: esto deberia estar en el if?
    sdr_->pushBundleToId(bundle, nextHopEid);
}

template<typename Func>
bool forEachCurrentPosition(const unordered_map<TimeInterval, vector<PositionEntry>>& nodePositions, double currTime, Func func) {
    /*
    for (const auto& [interval, positions] : nodePositions) {
        if (currTime < interval.tStart || currTime >= interval.tEnd)
            continue;
        for (const auto& pos : positions) {
            if (func(pos))
                return true;
        }
    }
        */
    return false;
}

bool RoutingAntop::isNextHopValid(H3Index nextHop) const {
    /*
    double currTime = simTime().dbl();
    printf("Node positions are:\n");

    return forEachCurrentPosition(this->nodePositions, currTime, [&](const PositionEntry& pos) {
        H3Index cell;
        latLngToCell(&pos.latLng, this->antopAlgorithm->getResolution(), &cell);
        cout << "cell: " << cell << " nextHop: " << nextHop << std::endl;
        if (cell == nextHop) {
            cout << "Next hop is valid: " << nextHop << std::endl;
            return true;
        }
        return false;
    });
    */
    return false;
}

H3Index RoutingAntop::getH3IndexFromEid(int eid) {
   /* double currTime = simTime().dbl();
    H3Index result = 0;
    forEachCurrentPosition(this->nodePositions, currTime, [&](const PositionEntry& pos) {
        if (pos.eId == eid) {
            latLngToCell(&pos.latLng, this->antopAlgorithm->getResolution(), &result);
            return true;
        }
        return false;
    });

    return result;
    */
   return 0;
}

int RoutingAntop::getEidFromH3Index(H3Index idx) {
    /*
    double currTime = simTime().dbl();
    int result = -1;
    forEachCurrentPosition(this->nodePositions, currTime, [&](const PositionEntry& pos) {
        H3Index cell;
        latLngToCell(&pos.latLng, this->antopAlgorithm->getResolution(), &cell);
        if (cell == idx) {
            result = pos.eId;
            return true;
        }
        return false;
    });

    return result;
    */

    return 0;
}