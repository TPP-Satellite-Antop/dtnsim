/*
 * RoutingUncertainUniboCgr.h
 *
 *  Created on: Jan 16, 2022
 *      Author: Simon Rink
 */

#ifndef SRC_NODE_DTN_ROUTING_ROUTINGUNCERTAINUNIBOCGR_H_
#define SRC_NODE_DTN_ROUTING_ROUTINGUNCERTAINUNIBOCGR_H_

#include "list"
#include "queue"
#include "src/node/dtn/routing/unibocgr/core/cgr/cgr_phases.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/contactPlan.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/contacts/contacts.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/nodes/nodes.h"
#include "src/node/dtn/routing/unibocgr/core/contact_plan/ranges/ranges.h"
#include <src/node/dtn/routing/RoutingOpportunistic.h>
#include "src/node/dtn/contactplan/ContactSdrModel.h"

#include "src/node/dtn/routing/unibocgr/core/cgr/cgr.h"
#include "src/utils/json.hpp"

using json = nlohmann::json;
using namespace std::chrono;
class RoutingUncertainUniboCgr : public RoutingOpportunistic {
  public:
    RoutingUncertainUniboCgr(int eid, ContactSdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                             MetricCollector *metricCollector, int tsIntervalDuration,
                             bool useUncertainty, int repetition, int numOfNodes);
    virtual ~RoutingUncertainUniboCgr();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    virtual void contactFailure(int contactId);
    virtual void updateContactPlan(Contact *c);
    virtual bool msgToMeArrive(BundlePkt *bundle);
    virtual void successfulBundleForwarded(long bundleId, Contact *contact, bool sentToDestination);
    virtual bool isDeliveredBundle(long bundleId);
    virtual void contactStart(Contact *c);
    virtual void contactEnd(Contact *c);
    virtual void callToPython();
    virtual void createSourceDestFile();
    virtual void convertContactPlanIntoNet();
    virtual void readJsonFromFile();
    virtual void notifyAboutRouting(json jsonFunction, int destination);
    virtual void notifyAboutMultiHop(vector<int> hops, long bundleId);
    virtual vector<int> hasMultiHop(Route *route);

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
    map<int, double> lastTimeUpdated_;
    int lastId_ = -1;
    vector<long> deliveredBundles_;
    map<long, queue<double>> bundleReroutable;
    map<long, json> nodeBrufFunction_;
    map<int, vector<int>> tsStartTimes_;
    map<int, int> maxTs_;
    int opportunistic;
    map<long, vector<int>> multiHops;
    vector<tuple<UniboContact *, vector<int>>> contactToHop;

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

#endif /* SRC_NODE_DTN_ROUTING_ROUTINGUNCERTAINUNIBOCGR_H_ */
