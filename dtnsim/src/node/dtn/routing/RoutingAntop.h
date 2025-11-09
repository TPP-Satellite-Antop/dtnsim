
#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "antop.h"
#include "h3api.h"
#include "src/node/mobility/SatSGP4Mobility.h"

#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>

struct CacheEntry {
  int nextHop;
  simtime_t ttl; // simulation time until which this entry is valid
};

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(Antop* antop, int eid, SdrModel *sdr, map<int, SatSGP4Mobility*> *mobilityMap);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);


  private:
    Antop* antopAlgorithm;
    map<int, SatSGP4Mobility*> *mobilityMap;
    H3Index prevSrc; // for example: we want to send bundle from 1 to 4. First call to getNextHopId(1,4,0) returns 2. 
                     // In next call prevSrc is 1: getNextHopId(2,4,1). //TODO maybe it is useless

    unordered_map<int, CacheEntry> nextHopCache; // key: destination eid, value: next hop cache entry       
    [[nodiscard]] bool isNextHopValid(H3Index nextHop) const;

    // Returns the current H3 index of the node with given eid. Returns 0 if not found.
    [[nodiscard]] H3Index getCurH3IndexForEid(int eid) const;
    unordered_map<H3Index, int> getEidsFromH3Indexes(const vector<H3Index> &candidates);
    void storeBundle(BundlePkt *bundle); //to retry routing later

    // Cache functions (routing tables)
    void saveToCache(int destinationEid, int nextHop, simtime_t simTime);
    int getFromCache(int destinationEid, simtime_t simTime); // returns 0 if not found
    void getNewNextHop(BundlePkt *bundle, simtime_t simTime); // non-cached version
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
