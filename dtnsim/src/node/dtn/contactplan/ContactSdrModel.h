#ifndef SRC_NODE_DTN_CONTACT_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACT_SDR_MODEL_H_

#include <omnetpp.h>
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/SdrModel.h"

using namespace omnetpp;
using namespace std;

class ContactSdrModel final : public SdrModel {
  public:
    ContactSdrModel();
    ContactSdrModel(int eid, int nodesNumber, ContactPlan* contactPlan);
    ~ContactSdrModel() override;

    int getBytesStoredToNeighbor(int eid) override;
    vector<int> getBundleSizes(int eid) override;
    vector<int> getPriorityBundleSizes(int eid, bool critical) override;

    // Initialization and configuration
    void setEid(int eid);
    void setNodesNumber(int nodesNumber);
    void setContactPlan(ContactPlan *contactPlan);
    void setSize(int size);

  private:
    ContactPlan *contactPlan_;
};

#endif /* SRC_NODE_DTN_CONTACT_SDR_MODEL_H_ */
