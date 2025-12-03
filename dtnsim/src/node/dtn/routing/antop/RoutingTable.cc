#include <vector>
#include "RoutingTable.h"
#include "antop.h"
#include "h3api.h"


RoutingTable::RoutingTable(Antop* antop) {
    this->antop = antop;
}

void RoutingTable::insert(const H3Index key, const CacheEntry& entry) {
    auto& bucket = cache[key];
    bucket.entries[bucket.nextIdx ? 1 : 0] = entry;
    bucket.nextIdx = !bucket.nextIdx;
}

Bucket RoutingTable::get(const H3Index key) {
    return cache[key];
}

H3Index RoutingTable::findNextNeighbor(const H3Index cur, const H3Index dst, const H3Index sender) {
    __uint8_t* bitmap = &cache[dst].visitedBitmap;

    std::vector<H3Index> candidates = antop->getHopCandidates(cur, dst, 0);

    __uint8_t curNeighbor = 128;
    for (auto candidate : candidates) {
        if (candidate == sender) {
            *bitmap = *bitmap | curNeighbor;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    // If no path is found, perhaps we want to return a 0 instead of the sender.
    H3Index nextNeighbor = sender;
    curNeighbor = 128;
    for (auto candidate : candidates) {
        if (*bitmap & curNeighbor == 0) {
            nextNeighbor = candidate;
            break;
        }
        curNeighbor = curNeighbor >> 1;
    }

    *bitmap = *bitmap | curNeighbor;

    return nextNeighbor;
}
