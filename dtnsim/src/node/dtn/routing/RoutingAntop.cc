#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

#include "src/node/mobility/SatSGP4Mobility.h"

RoutingAntop::RoutingAntop(Antop* antop, int eid, SdrModel *sdr, SatSGP4Mobility* mobility): RoutingDeterministic(eid, sdr, nullptr) { //TODO check this null
    this->prevSrc = 0;
    this->antopAlgorithm = antop;
    this->mobility = mobility;
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    std::cout << "RoutingAntop::routeAndQueueBundle called for bundle " << bundle->getBundleId() << " from " << bundle->getSourceEid() << " to " << bundle->getDestinationEid() << std::endl;

    H3Index srcIndex = getCurH3Index();
    H3Index nextHopIndex = this->antopAlgorithm->getNextHopId(
        srcIndex,
        getH3IndexFromEid(bundle->getDestinationEid()),
        this->prevSrc,
        [this](const H3Index idx){ return this->isNextHopValid(idx); }
    );

    const int nextHopEid = getEidFromH3Index(nextHopIndex);
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
    return true;
}

//TODO is this eid always the same (the current one)?
H3Index RoutingAntop::getCurH3Index() const {
    const auto latLng = LatLng {this->mobility->getLatitude(), this->mobility->getLongitude()};
    H3Index cell = 0;

    if (latLngToCell(&latLng, this->antopAlgorithm->getResolution(), &cell) != E_SUCCESS){
        cout << "Error converting lat long to cell" << endl;
    }

    return cell;
}

int RoutingAntop::getEidFromH3Index(H3Index idx) {
    /*
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