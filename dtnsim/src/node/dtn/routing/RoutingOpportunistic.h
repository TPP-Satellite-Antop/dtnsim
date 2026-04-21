/*
 * RoutingOpportunistic.h
 *
 *  Created on: Dec 2, 2021
 *      Author: simon
 */

#ifndef SRC_NODE_DTN_ROUTING_ROUTINGOPPORTUNISTIC_H_
#define SRC_NODE_DTN_ROUTING_ROUTINGOPPORTUNISTIC_H_

#include "src/utils/MetricCollector.h"
#include <src/node/dtn/routing/Routing.h>
#include "src/node/dtn/contactplan/ContactPlan.h"

class RoutingOpportunistic : public Routing {
  public:
    RoutingOpportunistic(int eid, SdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                         MetricCollector *metricCollector);
    ~RoutingOpportunistic() override;

    void msgToOtherArrive(BundlePkt *bundle, double simTime) override;

    bool msgToMeArrive(BundlePkt *bundle) override;

    void contactStart(Contact *c) override;

    void contactEnd(Contact *c) override;

    void successfulBundleForwarded(long bundleId, Contact *contact, bool sentToDestination) override;

    void updateContactPlan(Contact *c) override;

    void refreshForwarding(Contact *c) override;

    void updatePosition(int eid, double lat, double lng) override;

    // This is a pure virtual method (all opportunistic routing must at least
    // implement this function)
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime) = 0;

  protected:
    ContactPlan *contactPlan_;
    cModule *dtn_;
    MetricCollector *metricCollector_;
};

#endif /* SRC_NODE_DTN_ROUTING_ROUTINGOPPORTUNISTIC_H_ */
