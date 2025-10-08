#ifndef __CENTRAL_H_
#define __CENTRAL_H_

#include <map>
#include <omnetpp.h>
#include <utility>
#include <vector>
#include "src/utils/MetricCollector.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include "src/dtnsim_m.h"

using namespace omnetpp;
using namespace std;

namespace dtnsim {

class Central : public cSimpleModule {
  public:
    Central();
    virtual ~Central();
    virtual void finish();
    virtual void initialize();
    void handleMessage(cMessage *msg);

  protected:
    /// @brief Fill flowIds_ structure with App traffic data generators
    void computeFlowIds();
    map<int, map<int, map<int, double>>> getTraffics();

    virtual double getState(double trafficStart);

    /// @param[in] toDelete are nodes ids
    /// @param[in] faultsAware specifies if it is erased or simply set to deleted
    virtual void deleteNodes(vector<int> toDelete, bool faultsAware);

    /// @brief get n randomly
    /// @return vector of ids
    virtual vector<int> getRandomNodeIds(int n);

    /// @brief get ids randomly. Any node is chosen with probability failureProbability.
    /// @return vector of ids
    virtual vector<int> getRandomNodeIdsWithFProb(double failureProbability);

    /// @brief goes to the available nodes from the network topology and chooses their id, in
    /// case they have to be deleted
    /// @return the ids of the chosen ones to be deleted
    virtual vector<int> getNodeIdsWithSpecificFProb();

    /// @brief get n by iteratively erasing nodes
    /// with highest centrality
    /// @return vector of ids
    virtual vector<int> getCentralityNodeIds(int n, int nodesNumber);

    // Nodes Number in the network
    int nodesNumber_;

    // Observer used to collect and evaluate all the necessary metrics
    MetricCollector metricCollector_;

    // flowIds map: (src,dst) -> flowId
    // save flow ids corresponding to traffic data
    // generated in App layer
    map<pair<int, int>, unsigned int> flowIds_;
};

} // namespace dtnsim

#endif
