/*
 * RoutingUniboCgr.h
 *
 *  Created on: Dec 2, 2021
 *      Author: simon
 */

#ifndef SRC_NODE_DTN_ROUTING_ROUTINGUNIBOCGR_H_
#define SRC_NODE_DTN_ROUTING_ROUTINGUNIBOCGR_H_

#include "src/node/dtn/routing/unibocgr/core/cgr/cgr_phases.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/contactPlan.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/contacts/contacts.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/nodes/nodes.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/ranges/ranges.h"
#include <src/node/dtn/routing/RoutingOpportunistic.h>
#include "src/node/dtn/routing/unibocgr/core/cgr/cgr.h"

class RoutingUniboCgr : public RoutingOpportunistic {
  public:
    RoutingUniboCgr(int eid, SdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                    MetricCollector *metricCollector);
    virtual ~RoutingUniboCgr();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    virtual void updateContactPlan(Contact *c);
    virtual bool msgToMeArrive(BundlePkt *bundle);
    virtual void successfulBundleForwarded(long bundleId, Contact *contact, bool sentToDestination);
    virtual bool isDeliveredBundle(long bundleId);
    virtual void contactStart(Contact *c);
    virtual void contactEnd(Contact *c);

  private:
    int convertBundlePktToCgrBundle(time_t time, BundlePkt *bundle, CgrBundle *cgrBundle);
    int addContactToSap(Contact *contact);
    int removeContactFromSap(Contact *contact);
    int populateContactPlan();
    int initializeUniboCGR(time_t time);
    int callUniboCGR(time_t time, BundlePkt *bundle, List *cgrRoutes);
    void saveSAPs();
    void restoreSAPvalues();
    void initializeSAPValues();
    void enqueueBundle(BundlePkt *bundle, double simTime, Route *contact);

    time_t referenceTime_ = -1;
    bool initialised_ = false;
    int defaultRegionNbr_ = 1;
    CgrBundle *cgrBundle_ = NULL;
    List excludedNeighbors_ = NULL;
    int lastId_ = -1;
    vector<long> deliveredBundles_;

    // SAPs for Contact graph management
    ContactPlanSAP contactPlanSAP;
    RangeGraphSAP rangeGraphSAP;
    NeighborsSAP neighborsSAP;
    ContactGraphSAP contactGraphSAP;
    Rbt nodesRbt;

    // SAPs for general Unibo
    UniboCgrSAP uniboCgrSAP;
    UniboCgrCurrentCallSAP uniboCgrCurrentCallSAP;
    PhaseOneSAP phaseOneSAP;
    PhaseTwoSAP phaseTwoSAP;
};

#endif /* SRC_NODE_DTN_ROUTING_ROUTINGUNIBOCGR_H_ */
