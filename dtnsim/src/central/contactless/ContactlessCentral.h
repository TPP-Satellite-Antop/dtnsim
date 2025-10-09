#ifndef __CONTACTLESS_CENTRAL_H_
#define __CONTACTLESS_CENTRAL_H_

#include <map>
#include <omnetpp.h>
#include <utility>
#include <vector>
#include "src/utils/MetricCollector.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include "../Central.h"
#include "src/dtnsim_m.h"

using namespace omnetpp;
using namespace std;

namespace dtnsim {

class ContactlessCentral : public Central {
  public:
    ContactlessCentral();
    virtual ~ContactlessCentral();
    void initialize() override;

  private:    
    // overrides
    double getState(double trafficStart) override;
    void deleteNodes(vector<int> toDelete, bool faultsAware) override;
    vector<int> getRandomNodeIds(int n) override;
    vector<int> getRandomNodeIdsWithFProb(double failureProbability) override;
    vector<int> getNodeIdsWithSpecificFProb() override;
    vector<int> getCentralityNodeIds(int n, int nodesNumber) override;
};

} // namespace dtnsim

#endif 
