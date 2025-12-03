#ifndef DTNSIM_ANTOPROUTINGTABLE_H
#define DTNSIM_ANTOPROUTINGTABLE_H

#include <unordered_map>
#include <bits/types.h>

#include "h3api.h"
#include "antop.h"
#include "src/dtnsim_m.h"

struct CacheEntry {
    H3Index nextHop;
    int distance;
};

struct Bucket {
    std::array<CacheEntry, 2> entries{};
    __uint8_t visitedBitmap = 0; // Each bit represents a neighbour. Since a hexagon has up to 6 neighbours, last 2 bits are redundant and never used.
    bool nextIdx = false;
    double ttl = 0; // Simulation time until which this entry is valid
};

class RoutingTable {
    std::unordered_map<H3Index, Bucket> cache;

    Antop* antop;
public:
    explicit RoutingTable(Antop* antop);
    void insert(H3Index, CacheEntry);
    Bucket get(H3Index key);
    H3Index findNextNeighbor(H3Index cur, H3Index dst, H3Index sender);
};

#endif // DTNSIM_ANTOPROUTINGTABLE_H
