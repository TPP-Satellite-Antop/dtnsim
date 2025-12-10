#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "routingTable.h"
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
    map<int, inet::SatelliteMobility *> *mobilityMap;
    RoutingTable *routingTable;

    // Returns the current H3 index of the node with given eid. Returns 0 if not found.
    [[nodiscard]] H3Index getCurH3IndexForEid(int eid) const;
    int getEidFromH3Index(H3Index idx);
    void storeBundle(BundlePkt *bundle) const; //to retry routing later
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
