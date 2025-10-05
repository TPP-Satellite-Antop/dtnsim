
#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include <unordered_map>
#include <vector>
#include <src/node/dtn/SdrModel.h>
#include "h3api.h"
#include "antop.h"
#include <src/node/dtn/routing/RoutingDeterministic.h>
#include <src/node/dtn/PositionEntry.h>

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(int eid, SdrModel *sdr);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);


  private:
    Antop* antopAlgorithm;
    unordered_map<TimeInterval, vector<PositionEntry>> nodePositions; 
    H3Index prevSrc; // for example: we want to send bundle from 1 to 4. First call to getNextHopId(1,4,0) returns 2. 
                     // In next call prevSrc is 1: getNextHopId(2,4,1).
    bool isNextHopValid(H3Index nextHop) const;
    H3Index getH3IndexFromEid(int eid);
    int getEidFromH3Index(H3Index idx);
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
