#include <vector>
#include <bitset>

#include "RoutingTable.h"
#include "antop.h"
#include "h3api.h"

#include <iostream>
#include <ostream>

const int THRESHOLD_DISTANCE = 3;

RoutingTable::RoutingTable(Antop* antop) {
    this->antop = antop;
}

H3Index RoutingTable::findNextHop(H3Index cur, H3Index src, H3Index dst, H3Index sender, int curDistance) {
    PairTableKey key{src, dst};
    int storedDistance = pairTable[key];
    if (potentialLoop(storedDistance, curDistance)) { // Potential loop detected, return to sender.
        return sender;
    }

    pairTable[key] = storedDistance == 0 ? curDistance : std::min(storedDistance, curDistance);

    RoutingInfo routingInfoToSrc = routingTable[src];
    if (shouldUpdateSrcInfo(routingInfoToSrc, curDistance)) {
        std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);
        __uint8_t senderIdxInCandidates = 128;

        for (auto candidate : candidates) {
            if (candidate == sender)
                break;
            senderIdxInCandidates = senderIdxInCandidates >> 1;
        }

        routingTable[src] = {sender, 0, curDistance, senderIdxInCandidates}; // ToDo: save actual TTL.
    }

    RoutingInfo routingInfoToDst = routingTable[dst];
    if (routingInfoToDst.nextHop != 0)
        return routingInfoToDst.nextHop;

    return findNewNeighbor(cur, dst, sender);
}

// Returns true if the routing info to source should be updated.
bool RoutingTable::shouldUpdateSrcInfo(RoutingInfo &routingInfoToSrc, int curDistance) {
    return routingInfoToSrc.nextHop == 0 || routingInfoToSrc.distance > curDistance;
}

bool RoutingTable::potentialLoop(int storedDistance, int curDistance) {
    return storedDistance != 0 && storedDistance + THRESHOLD_DISTANCE < curDistance;
}

constexpr std::bitset<MAX_CANDIDATES> msbMask() {
    std::bitset<MAX_CANDIDATES> b;
    b.set(MAX_CANDIDATES - 1);  // highest bit
    return b;
}

void markVisited(
    std::bitset<MAX_CANDIDATES>& bitmap,
    const std::vector<H3Index>& candidates,
    H3Index visited
) {
    auto curNeighbor = msbMask();

    for (auto candidate : candidates) {
        if (candidate == visited) {
            bitmap |= curNeighbor;
            return;
        }
        curNeighbor >>= 1;
    }

}

H3Index findNextUnvisitedNeighbor(
    std::bitset<MAX_CANDIDATES>& bitmap,
    const std::vector<H3Index>& candidates,
    H3Index fallback
) {
    auto curNeighbor = msbMask();

    for (auto candidate : candidates) {
        if ((bitmap & curNeighbor).none()) {
            bitmap |= curNeighbor;   // mark as visited
            return candidate;
        }
        curNeighbor >>= 1;
    }

    // No unvisited neighbor found, fallback 
    return fallback;
}


H3Index RoutingTable::findNewNeighbor(const H3Index cur, const H3Index dst, const H3Index sender) {
    auto bitmap = routingTable[dst].visitedNeighboursBitmap;
    std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);

    // Flag sender as visited.
    markVisited(bitmap, candidates, sender);
    
    // Find next unvisited neighbor fallbacking to sender
    H3Index nextNeighbor = findNextUnvisitedNeighbor(bitmap, candidates, sender);

    routingTable[dst].nextHop = nextNeighbor;
    routingTable[dst].visitedNeighboursBitmap = bitmap;

    return nextNeighbor;
}
