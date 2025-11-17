#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "antop.h"
#include "h3api.h"
#include "src/node/mobility/SatelliteMobility.h"
#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(Antop* antop, int eid, SdrModel *sdr, map<int, inet::SatelliteMobility *> *mobilityMap);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);


  private:
    Antop* antopAlgorithm;
    map<int, inet::SatelliteMobility *> *mobilityMap;
    H3Index prevSrc; // for example: we want to send bundle from 1 to 4. First call to getNextHopId(1,4,0) returns 2. 
                     // In next call prevSrc is 1: getNextHopId(2,4,1). //TODO maybe it is useless

    [[nodiscard]] bool isNextHopValid(H3Index nextHop) const;

    // Returns the current H3 index of the node with given eid. Returns 0 if not found.
    [[nodiscard]] H3Index getCurH3IndexForEid(int eid) const;
    unordered_map<H3Index, int> getEidsFromH3Indexes(const vector<H3Index> &candidates);
    void storeBundle(BundlePkt *bundle);
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
