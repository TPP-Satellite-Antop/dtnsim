#ifndef DTNSIM_ANTOPROUTINGTABLE_H
#define DTNSIM_ANTOPROUTINGTABLE_H

#include <unordered_map>
#include <bits/types.h>
#include <tuple>

#include "h3api.h"
#include "antop.h"
#include "src/dtnsim_m.h"

struct PairTableKey {
    H3Index source;
    H3Index destination;
};

template <> struct std::hash<PairTableKey> {
    std::size_t operator()(const PairTableKey &k) const noexcept {
        const std::size_t h1 = std::hash<H3Index>()(k.source);
        const std::size_t h2 = std::hash<H3Index>()(k.destination);
        return h1 ^ (h2 << 1);
    }
};

struct RoutingInfo {
    H3Index nextHop = 0;
    double ttl = 0; // Simulation time until which this entry is valid
    int distance = 0;
    __uint8_t visitedBitmap = 0; // Each bit represents a neighbour. Since a hexagon has up to 6 neighbours, last 2 bits are redundant and never used.
};

class RoutingTable {
    std::unordered_map<H3Index, RoutingInfo> routingTable;
    std::unordered_map<PairTableKey, int> pairTable;

    Antop* antop;
public:
    explicit RoutingTable(Antop* antop);
    H3Index findNextHop(H3Index cur, H3Index src, H3Index dst, H3Index sender, int distance);
    H3Index findNewNeighbor(H3Index cur, H3Index dst, H3Index sender);
};

#endif // DTNSIM_ANTOPROUTINGTABLE_H
