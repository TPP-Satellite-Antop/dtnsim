#ifndef SRC_NODE_DTN_ROUTINGBRUF1T_H_
#define SRC_NODE_DTN_ROUTINGBRUF1T_H_

#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingBRUF1T : public RoutingDeterministic {
  public:
    RoutingBRUF1T(int eid, SdrModel *sdr, ContactPlan *contactPlan, string path_to_routing);
    virtual ~RoutingBRUF1T();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);

  private:
    //<source_node_id, target_node_id, time up to the routing decision is valid, outgoing
    //contact_id>
    std::map<int, std::map<int, std::map<int, int>>> routing_decisions_;
};

#endif
