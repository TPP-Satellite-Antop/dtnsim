#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(Antop* antop, const int eid, SdrModel *sdr, map<int, inet::SatelliteMobility *> *mobilityMap): RoutingDeterministic(eid, sdr, nullptr), routingTable(antop) {
    this->antopAlgorithm = antop;
    this->mobilityMap = mobilityMap;
    this->routingTable = RoutingTable(antop);
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    std::cout << "Node " << eid_ << " routing bundle " << bundle->getBundleId() << " from src " << bundle->getSourceEid() << ", sender " << bundle->getSenderEid() << " to " << bundle->getDestinationEid() << std::endl;

    const H3Index cur = getCurH3IndexForEid(eid_);
    const H3Index dst = getCurH3IndexForEid(bundle->getDestinationEid());
    const H3Index sender = getCurH3IndexForEid(bundle->getSenderEid());
    H3Index nextHop = 0;

    if (bundle->getReturnToSender())
        nextHop = routingTable.findNewNeighbor(cur, dst, sender);
    else {
        const H3Index src = getCurH3IndexForEid(bundle->getSourceEid());
        nextHop = routingTable.findNextHop(cur, src, dst, sender, 0); // ToDo: send actual hop count from bundle.
    }

    bundle->setReturnToSender(nextHop == sender);

    // ToDo: forward bundle to satellite at nextHop.
}

void RoutingAntop::getNewNextHop(BundlePkt *bundle, double simTime){
    const vector<H3Index> candidates = this->antopAlgorithm->getHopCandidates(
        getCurH3IndexForEid(eid_),
        getCurH3IndexForEid(bundle->getDestinationEid()),
        getCurH3IndexForEid(bundle->getSenderEid())
    );

    const auto eidsByCandidate = this->getEidsFromH3Indexes(candidates);

    //TODO remove debug prints
    for (auto [candidate, eid] : eidsByCandidate) {
        std::cout << "Candidate: " << std::hex << candidate << " - EID: " << std::dec << eid << std::endl;
    }

    for (auto candidate : candidates) {
        if (const int nextHop = eidsByCandidate.at(candidate); nextHop != 0) {
            bundle->setNextHopEid(nextHop);
            // saveToCache(bundle->getDestinationEid(), nextHop, simTime);
            std::cout << "Routing bundle " << bundle->getBundleId() << " to " << nextHop << std::endl;
            return;
        }
    }

    storeBundle(bundle);
}

H3Index RoutingAntop::getCurH3IndexForEid(const int eid) const {
    try {
        const inet::SatelliteMobility *mobility = this->mobilityMap->at(eid);
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

// Equeue bundle for later if no candidate was found
void RoutingAntop::storeBundle(BundlePkt *bundle) {
    if(!sdr_->pushBundle(bundle)){
        // ToDo: handle failed push.
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR" << std::endl;
    } else {
        std::cout << "Enqueued bundle " << bundle->getBundleId() << " to SDR" << std::endl;
    }
}
