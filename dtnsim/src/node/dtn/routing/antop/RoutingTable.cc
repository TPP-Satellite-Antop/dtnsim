#include <vector>
#include "RoutingTable.h"
#include "antop.h"
#include "h3api.h"

RoutingTable::RoutingTable(Antop* antop) {
    this->antop = antop;
}

H3Index RoutingTable::findNextHop(H3Index cur, H3Index src, H3Index dst, H3Index sender, int distance) {
    int shortestDistanceToSrc = pairTable[{src, dst}];
    if (shortestDistanceToSrc != 0 && shortestDistanceToSrc + 3 < distance) // ToDo: replace hardcoded threshold.
        return sender;

    if (shortestDistanceToSrc == 0 || shortestDistanceToSrc < distance)
        pairTable[{src, dst}] = distance;

    if (RoutingInfo routingInfoToSrc = routingTable[src]; routingInfoToSrc.nextHop == 0 || routingInfoToSrc.distance < distance) {
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
    __uint8_t bitmap = routingTable[dst].visitedBitmap;

    std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);

    __uint8_t curNeighbor = 128;
    for (auto candidate : candidates) {
        if (candidate == sender) {
            bitmap = bitmap | curNeighbor;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    // If no path is found, perhaps we want to return a 0 instead of the sender.
    H3Index nextNeighbor = sender;
    curNeighbor = 128;
    for (auto candidate : candidates) {
        if (bitmap & curNeighbor == 0) {
            nextNeighbor = candidate;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    routingTable[dst].nextHop = nextNeighbor;
    routingTable[dst].visitedBitmap = bitmap;

    return nextNeighbor;
}
