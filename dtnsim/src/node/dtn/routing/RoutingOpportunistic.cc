/*
 * RoutingOpportunistic.cc
 *
 *  Created on: Dec 2, 2021
 *      Author: simon
 */

#include "RoutingOpportunistic.h"

RoutingOpportunistic::RoutingOpportunistic(int eid, SdrModel *sdr, ContactPlan *contactPlan,
                                           cModule *dtn, MetricCollector *metricCollector)
    : Routing(eid, sdr) {
    this->contactPlan_ = contactPlan;
    this->dtn_ = dtn;
    this->metricCollector_ = metricCollector;
}

RoutingOpportunistic::~RoutingOpportunistic() {
    // TODO Auto-generated destructor stub
}

void RoutingOpportunistic::msgToOtherArrive(BundlePkt *bundle, double simTime) {
    this->routeAndQueueBundle(bundle, simTime);
}

bool RoutingOpportunistic::msgToMeArrive(BundlePkt *bundle) {
    return true;
}

void RoutingOpportunistic::contactEnd(Contact *c) {}

void RoutingOpportunistic::contactStart(Contact *c) {}

void RoutingOpportunistic::updateContactPlan(Contact *c) {}

void RoutingOpportunistic::successfulBundleForwarded(long bundleId, Contact *contact,
                                                     bool sentToDestination) {}

void RoutingOpportunistic::refreshForwarding(Contact *c) {}
