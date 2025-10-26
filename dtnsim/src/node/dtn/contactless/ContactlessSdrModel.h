#ifndef SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_

#include <vector>
#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/SdrModel.h"
#include "src/node/dtn/SdrStatus.h"
#include "src/utils/Subject.h"

using namespace omnetpp;
using namespace std;

class ContactlessSdrModel final : public SdrModel {
public:
    ContactlessSdrModel();
    ContactlessSdrModel(int eid, int nodesNumber);
    ~ContactlessSdrModel() override;

    int getBytesStoredToNeighbor(int eid) override;
    vector<int> getBundleSizes(int eid) override;
    vector<int> getPriorityBundleSizes(int eid, bool critical) override;
    bool pushBundle(BundlePkt *bundle) override;
    bool enqueuedBundle();
    void resetEnqueuedBundleFlag();

    // Initialization and configuration
    void setEid(int eid);
    void setNodesNumber(int nodesNumber);
    void setSize(int size);

private:
    bool enqueuedBundle_;
};

#endif /* SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_ */
