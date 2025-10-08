/*
 * RoutingORUCOP.h
 *
 *  Created on: Jan 24, 2022
 *      Author: Simon Rink
 */

#ifndef SRC_NODE_DTN_ROUTING_ROUTINGORUCOP_H_
#define SRC_NODE_DTN_ROUTING_ROUTINGORUCOP_H_

#include <queue>
#include <src/node/dtn/routing/RoutingOpportunistic.h>
#include "src/utils/json.hpp"

using json = nlohmann::json;
using namespace std::chrono;
class RoutingORUCOP : public RoutingOpportunistic {
  public:
    RoutingORUCOP(int eid, SdrModel *sdr, ContactPlan *contactPlan, cModule *dtn,
                  MetricCollector *metricCollector, int bundleCopies, int repetition,
                  int numOfNodes);
    virtual ~RoutingORUCOP();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    virtual bool msgToMeArrive(BundlePkt *bundle);
    virtual void successfulBundleForwarded(long bundleId, Contact *contact, bool sentToDestination);
    virtual bool isDeliveredBundle(long bundleId);
    virtual void contactStart(Contact *c);
    virtual void contactEnd(Contact *c);
    virtual void notifyAboutMultiHop(long bundleId, queue<Contact> hops);
    virtual void notifyAboutRouting(map<int, json> jsonFunction, int destination,
                                    map<int, int> idMap, vector<int> tsStartTimes, int numOfTs);

  private:
    virtual void callToPython(BundlePkt *bundle);
    virtual void createIniFile(int source, int destination);
    virtual void convertContactPlanIntoNet(int endDestination);
    virtual void readJsonFromFile(int destination, long bundleId);
    virtual int findMaxTs(int destination);
    virtual vector<int> getTsForContact(Contact *contact, int destination);
    virtual void updateStartTimes(vector<Contact> *contacts, int destination);
    virtual int getTsForEndTime(int end, int destination);
    virtual int getTsForStartOrCurrentTime(int startOrCurrent, int destination);
    virtual queue<Contact> getNextRoutingDecisionForBundle(long bundleId, int destination, int ts);
    virtual queue<Contact> getDecisionFromRoute(vector<int> route, int destination);
    virtual void reRouteBundles();
    virtual void reRouteFailedBundles(long bundleId, int destination, int failedBundles);
    virtual void queueBundle(BundlePkt *bundle, int contactId, int destinationEid);
    virtual void routeNextBundle(long bundleId, int destination, int ts);
    virtual int checkAndQueueBundles(long bundleId);
    virtual void updateRoutingDecisions();
    virtual void queueBundlesForCurrentContact();

    map<long, int> bundleSolvedCopies_;
    map<long, queue<BundlePkt *>> storedBundles_;
    map<long, vector<queue<Contact>>> routingDecisions_;
    map<int, int> lastUpdateTime_;
    map<int, map<int, int>> idMap_;
    vector<long> deliveredBundles_;
    int bundleCopies_;
    map<long, map<int, json>> jsonFunction_;
    map<int, vector<int>> tsStartTimes_;
    map<int, int> numOfTs_;
    bool opportunistic;
};

#endif /* SRC_NODE_DTN_ROUTING_ROUTINGORUCOP_H_ */
