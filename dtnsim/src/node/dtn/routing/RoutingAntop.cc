#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

#include "src/node/mobility/SatSGP4Mobility.h"

const double TTL_SEC = 6;

RoutingAntop::RoutingAntop(Antop* antop, int eid, SdrModel *sdr, map<int, SatSGP4Mobility*> *mobilityMap): RoutingDeterministic(eid, sdr, nullptr) { //TODO check this null
    this->prevSrc = 0;
    this->antopAlgorithm = antop;
    this->nextHopCache = unordered_map<int, CacheEntry>();
    this->mobilityMap = mobilityMap;
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    std::cout << "Node " << eid_ << " routing bundle " << bundle->getBundleId() << " from src " << bundle->getSourceEid() << ", sender " << bundle->getSenderEid() << " to " << bundle->getDestinationEid() << std::endl;

    int cachedNextHop = getFromCache(bundle->getDestinationEid(), simTime);
    if(cachedNextHop != 0){
        bundle->setNextHopEid(cachedNextHop);
        std::cout << "Using cached next hop " << cachedNextHop << " for bundle " << bundle->getBundleId() << std::endl;
        return;
    }

    getNewNextHop(bundle, simTime);
}

void RoutingAntop::getNewNextHop(BundlePkt *bundle, double simTime){
    const vector<H3Index> candidates = this->antopAlgorithm->getHopCandidates(
        getCurH3IndexForEid(eid_),
        getCurH3IndexForEid(bundle->getDestinationEid()),
        getCurH3IndexForEid(bundle->getSenderEid())
    );

    const auto eidsByCandidate = this->getEidsFromH3Indexes(candidates);

    for (auto [candidate, eid] : eidsByCandidate) {
        std::cout << "Candidate: " << std::hex << candidate << " - EID: " << std::dec << eid << std::endl;
    }

    int nextHop = 0;
    for (auto candidate : candidates) {
        if (nextHop = eidsByCandidate.at(candidate); nextHop != 0) {
            bundle->setNextHopEid(nextHop);
            saveToCache(bundle->getDestinationEid(), nextHop, simTime);
            std::cout << "Routing bundle " << bundle->getBundleId() << " to " << nextHop << std::endl;
            return;
        }
    }

    storeBundle(bundle, nextHop);
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

// Equeue bundle for later if no candidate was found
void RoutingAntop::storeBundle(BundlePkt *bundle, int nextHop) {
    if(!sdr_->pushBundleToId(bundle, nextHop)){
        // ToDo: handle failed push.
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR for next hop " << nextHop << std::endl;
    } else {
        std::cout << "Enqueued bundle " << bundle->getBundleId() << " to SDR for next hop " << nextHop << std::endl;
    }
}

void RoutingAntop::saveToCache(int destinationEid, int nextHop, double simTime){
    CacheEntry entry = {
        .nextHop = nextHop,
        .ttl = simTime + TTL_SEC
    };

    nextHopCache[destinationEid] = entry;
}

int RoutingAntop::getFromCache(int destinationEid, double simTime){
    auto it = nextHopCache.find(destinationEid);
    if(it != nextHopCache.end()){ // if found
        CacheEntry entry = it->second;
        if(simTime <= entry.ttl){
            return entry.nextHop;
        } else {
            // Entry expired
            nextHopCache.erase(it);
            return 0;
        }
    }

    return 0;
}