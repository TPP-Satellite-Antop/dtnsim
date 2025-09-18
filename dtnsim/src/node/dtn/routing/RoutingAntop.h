
#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include <unordered_map>
#include <vector>
#include <src/node/dtn/SdrModel.h>
#include "h3api.h"
#include "antop.h"
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(int eid, SdrModel *sdr, ContactPlan *contactPlan);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);


  private:
    Antop* antopAlgorithm;
    unordered_map<TimeInterval, vector<PositionEntry>> nodePositions; 
    H3Index prevSrc; // for example: we want to send bundle from 1 to 4. First call to getNextHopId(1,4,0) returns 2. 
                     // In next call prevSrc is 1: getNextHopId(2,4,1).
    bool isNextHopValid(H3Index nextHop) const;
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
