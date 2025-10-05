#ifndef POSITION_ENTRY_H_
#define POSITION_ENTRY_H_

#include "h3api.h"

struct PositionEntry {
    int eId;      
    LatLng latLng; // latitude and longitude in radians
};

struct TimeInterval {
    double tStart;
    double tEnd;

    bool operator==(const TimeInterval& other) const {
        const double EPS = 1e-9;
        return std::abs(tStart - other.tStart) < EPS && std::abs(tEnd - other.tEnd) < EPS;
    }
};

//TODO test
namespace std {
    template <>
    struct hash<TimeInterval> {
        std::size_t operator()(const TimeInterval& ti) const noexcept {
            return std::hash<double>()(ti.tStart) ^ (std::hash<double>()(ti.tEnd) << 1);
        }
    };
}

#endif /* POSITION_ENTRY_H_ */