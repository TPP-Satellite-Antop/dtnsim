#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "routingTable.h"
#include "h3api.h"
#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
  public:
    using GetH3Fn = std::function<H3Index(int)>;
    using GetEidFromH3Fn = std::function<int(H3Index, H3Index, int)>;
    using NextMobilityUpdateFn = std::function<double()>;

    RoutingAntop(
      Antop* antop,
      int eid,
      SdrModel *sdr,
      GetH3Fn getH3Index,
      GetEidFromH3Fn getEidFromH3,
      NextMobilityUpdateFn nextMobilityUpdateFn);
    virtual ~RoutingAntop();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    int getAntopResolution() const {
        return this->routingTable->getAntopResolution();
    }

  private:
    GetH3Fn getH3Index_;
    GetEidFromH3Fn getEidFromH3Index_;
    NextMobilityUpdateFn nextMobilityUpdateFn_;
    RoutingTable *routingTable;
    void storeBundle(BundlePkt *bundle) const; //to retry routing later
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
