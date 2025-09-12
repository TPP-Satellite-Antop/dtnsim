
#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include <src/node/dtn/SdrModel.h>
#include "h3api.h"
#include "antop.h"
#include <src/node/dtn/routing/RoutingDeterministic.h>
#include <unordered_map>

class RoutingAntop : public RoutingDeterministic {
  public:
    RoutingAntop(int eid, SdrModel *sdr, ContactPlan *contactPlan);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);

  private:
    Antop* antopAlgorithm;
    std::unordered_map<H3Index, int> h3IndexToEid;
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
