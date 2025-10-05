/*
 * RoutingPRoPHET.h
 *
 *  Created on: Sep 13, 2022
 *      Author: simon
 */

#ifndef SRC_NODE_DTN_ROUTING_ROUTINGPROPHET_H_
#define SRC_NODE_DTN_ROUTING_ROUTINGPROPHET_H_

#include "../contactplan/Dtn.h"
#include "src/utils/MetricCollector.h"
#include <src/node/dtn/routing/Routing.h>

class RoutingPRoPHET : public Routing {
  public:
    RoutingPRoPHET(int eid, SdrModel *sdr, cModule *dtn, float p_enc_max, float p_enc_first,
                   float p_first_thresh, float forw_thresh, float alpha, float beta, float gamma,
                   float delta, int numOfNodes, MetricCollector *metricCollector);
    virtual ~RoutingPRoPHET();

    virtual void msgToOtherArrive(BundlePkt *bundle, double simTime);

    virtual bool msgToMeArrive(BundlePkt *bundle);

    virtual void contactStart(Contact *c);

    virtual void contactEnd(Contact *c);

    virtual void successfulBundleForwarded(long bundleId, Contact *contact, bool sentToDestination);

    virtual void updateContactPlan(Contact *c);

    virtual void refreshForwarding(Contact *c);

    virtual void routeAndQueueBundle(Contact *c);

  private:
    cModule *dtn_;

    // The bundles this node has received as the final recipient or sent to final destination
    list<int> deliveredBundles_;

    bool isCarryingBundle(long bundleId);
    bool isDeliveredBundle(long bundleId);
    void updateAging(int destinationEid, double simTime);
    void updatePredToNode(int destinationEid, double simTime);
    void updateTransitivity(int destinationEid, double simTime);
    void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    vector<double> *getPredTable();
    vector<double> pred_table_;
    vector<double> last_seen_;
    vector<double> last_updated_;
    unordered_map<int, int> currently_active_;
    MetricCollector *metricCollector_;
    vector<vector<double>> inactive_intervals_;
    float p_encouter_max_;
    float p_encouter_first_;
    float p_first_threshold_;
    float forw_thresh_;
    float alpha_;
    float beta_;
    float gamma_;
    float delta_;
};

#endif /* SRC_NODE_DTN_ROUTING_ROUTINGPROPHET_H_ */
