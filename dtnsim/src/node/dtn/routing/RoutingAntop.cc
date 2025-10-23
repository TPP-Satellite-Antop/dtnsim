#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

#include "src/node/mobility/SatSGP4Mobility.h"

RoutingAntop::RoutingAntop(Antop* antop, int eid, SdrModel *sdr, map<int, SatSGP4Mobility*> *mobilityMap): RoutingDeterministic(eid, sdr, nullptr) { //TODO check this null
    this->prevSrc = 0;
    this->antopAlgorithm = antop;
    this->mobilityMap = mobilityMap;
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    std::cout << "Node " << eid_ << ": RoutingAntop::routeAndQueueBundle called for bundle " << bundle->getBundleId() << " from " << bundle->getSourceEid() << " to " << bundle->getDestinationEid() << std::endl;

    // TODO is this eid always the same (the current one)?
    const H3Index srcIndex = getCurH3IndexForEid(bundle->getSourceEid());

    const vector<H3Index> candidates = this->antopAlgorithm->getHopCandidates(
        srcIndex,
        getCurH3IndexForEid(bundle->getDestinationEid()),
        0 // ToDo: send actual pervious last hop.
    );

    const auto eidsByCandidate = this->getEidsFromH3Indexes(candidates);

    for (auto candidate : candidates) {
        if (const auto nextHop = eidsByCandidate.at(candidate); nextHop != 0) {
            bundle->setNextHopEid(nextHop);
            if(!sdr_->pushBundleToId(bundle, nextHop)){
                std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR for next hop " << nextHop << std::endl;
                return;
            }

            std::cout << "Routing bundle " << bundle->getBundleId() << " to " << nextHop << std::endl;
            return;
        }
    }

    std::cout << "Failed to route bundle " << bundle->getBundleId() << " due to isolation" << std::endl;
    // ToDo: handlear qué pasa si no hay satélites alrededor.
}

H3Index RoutingAntop::getCurH3IndexForEid(const int eid) const {
    try {
        const SatSGP4Mobility *mobility = this->mobilityMap->at(eid);
        const auto latLng = LatLng {mobility->getLatitude(), mobility->getLongitude()};
        H3Index cell = 0;

        if (latLngToCell(&latLng, this->antopAlgorithm->getResolution(), &cell) != E_SUCCESS){
            cout << "Error converting lat long to cell" << endl;
        }

        return cell;
    } catch (const std::out_of_range& e) {
        cout << "Error in antop routing: no mobility module found for eid " << eid << endl;
        return 0;   
    }
}

unordered_map<H3Index, int> RoutingAntop::getEidsFromH3Indexes(const vector<H3Index> &candidates) {
    unordered_map<H3Index, int> eidsByCandidates; // ToDo: should be vector<int> instead of int to gather ALL options for fault tolerance.
    eidsByCandidates.reserve(candidates.size());

    for (const auto& candidate : candidates)
        eidsByCandidates[candidate] = 0;

    for (const auto& [eid, _] : *this->mobilityMap) {
        const auto idx = this->getCurH3IndexForEid(eid);

        if (auto it = eidsByCandidates.find(idx); it != eidsByCandidates.end())
            it->second = eid;
    }

    return eidsByCandidates;
}