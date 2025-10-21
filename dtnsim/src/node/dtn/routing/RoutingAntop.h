
#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "antop.h"
#include "h3api.h"
#include "src/node/mobility/SatSGP4Mobility.h"

#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(Antop* antop, int eid, SdrModel *sdr, SatSGP4Mobility *mobility);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);


  private:
    Antop* antopAlgorithm;
    SatSGP4Mobility* mobility;
    H3Index prevSrc; // for example: we want to send bundle from 1 to 4. First call to getNextHopId(1,4,0) returns 2. 
                     // In next call prevSrc is 1: getNextHopId(2,4,1).

    [[nodiscard]] bool isNextHopValid(H3Index nextHop) const;
    [[nodiscard]] H3Index getCurH3Index() const;
    static int getEidFromH3Index(H3Index idx);
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
