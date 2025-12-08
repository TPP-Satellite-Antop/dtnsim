#include <vector>
#include <bitset>

#include "RoutingTable.h"
#include "antop.h"
#include "h3api.h"

#include <iostream>
#include <ostream>

RoutingTable::RoutingTable(Antop* antop) {
    this->antop = antop;
}

H3Index RoutingTable::findNextHop(H3Index cur, H3Index src, H3Index dst, H3Index sender, int distance) {
    int distanceToSrc = pairTable[{src, dst}];
    if (distanceToSrc != 0 && distanceToSrc + 3 < distance) { // ToDo: replace hardcoded threshold.
        // return findNewNeighbor(cur, dst, sender);
        return sender;
    }

    pairTable[{src, dst}] = distanceToSrc == 0 ? distance : std::min(distanceToSrc, distance);

    if (RoutingInfo routingInfoToSrc = routingTable[src]; routingInfoToSrc.nextHop == 0 || routingInfoToSrc.distance > distance) {
        std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);
        __uint8_t bitmap = 128;

        for (auto candidate : candidates) {
            if (candidate == sender)
                break;
            bitmap = bitmap >> 1;
        }

        routingTable[src] = {sender, 0, distance, bitmap}; // ToDo: save actual TTL.
    }

    if (RoutingInfo routingInfoToDst = routingTable[dst]; routingInfoToDst.nextHop != 0)
        return routingInfoToDst.nextHop;
    return findNewNeighbor(cur, dst, sender);
}

H3Index RoutingTable::findNewNeighbor(const H3Index cur, const H3Index dst, const H3Index sender) {
    auto bitmap = routingTable[dst].visitedBitmap;
    std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);

    // Flag sender as visited.
    std::bitset<6> curNeighbor{0b100000};
    for (auto candidate : candidates) {
        if (candidate == sender) {
            bitmap = bitmap | curNeighbor;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    // ToDo: if no path is found, we should return to sender unless it has already been tried.
    H3Index nextNeighbor = cur;
    curNeighbor = {0b100000};
    for (auto candidate : candidates) {
        if ((bitmap & curNeighbor) == 0) {
            nextNeighbor = candidate;
            bitmap = bitmap | curNeighbor;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    routingTable[dst].nextHop = nextNeighbor;
    routingTable[dst].visitedBitmap = bitmap;

    return nextNeighbor;
}
