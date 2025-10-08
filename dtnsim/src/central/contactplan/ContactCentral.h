#ifndef __CONTACT_PLAN_CENTRAL_H_
#define __CONTACT_PLAN_CENTRAL_H_

#include <map>
#include <omnetpp.h>
#include <utility>
#include <vector>
#include "../../node/dtn/contactplan/ContactPlan.h"
#include "src/utils/MetricCollector.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include "src/node/dtn/routing/RoutingCgrModel350.h"
#include "../Central.h"
#include "src/dtnsim_m.h"

using namespace omnetpp;
using namespace std;

namespace dtnsim {

class ContactPlanCentral : public Central {
  public:
    ContactPlanCentral();
    virtual ~ContactPlanCentral();
    void finish() override;
    void initialize() override;

  private:
    /// @brief Compute Topology from  contactPlan_ and save it
    /// in dot and pdf files inside "results" folder
    void saveTopology();
    /// @brief Compute Flows from BundleMaps files and save them
    /// in dot and pdf files inside "results" folder
    void saveFlows();
    void saveLpFlows();
    
    /// @brief get n by iteratively erasing contacts
    /// with highest centrality
    /// @return vector of contact ids
    int computeNumberOfRoutesThroughContact(int contactId, vector<CgrRoute> shortestPaths);
    set<int> getAffectedContacts(vector<CgrRoute> shortestPaths);
    
    /// @brief compute total routes from all to all nodes
    /// nodePairsRoutes is returned as output, it will contain an element i->j if there is at least
    /// a route between i and j
    int computeTotalRoutesNumber(ContactPlan &contactPlan, int nodesNumber,
      set<pair<int, int>> &nodePairsRoutes);

    // overrides
    double getState(double trafficStart) override;
    void deleteNodes(vector<int> toDelete, bool faultsAware) override;
    vector<int> getRandomNodeIds(int n);
    vector<int> getRandomNodeIdsWithFProb(double failureProbability) override;
    vector<int> getNodeIdsWithSpecificFProb() override;
    vector<int> getCentralityNodeIds(int n, int nodesNumber) override;

    // Contact Plan passed to all nodes to feed CGR
    // It can be different to the real topology
    ContactPlan contactPlan_;
    
    // Contact Plan passed to all nodes to schedule Contacts
    // and get transmission rates
    ContactPlan contactTopology_;
    
    // specify if there are ion nodes in the simulation
    bool ionNodes_;

    // stat signals
    simsignal_t nodesNumber;
    simsignal_t totalRoutes;
    simsignal_t availableRoutes;
    simsignal_t pairsOfNodesWithAtLeastOneRoute;
};

} // namespace dtnsim

#endif 
