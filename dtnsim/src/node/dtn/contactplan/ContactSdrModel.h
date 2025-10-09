/*
 * SdrModel.h
 *
 *  Created on: Nov 25, 2016
 *      Author: juanfraire
 */

#ifndef SRC_NODE_DTN_CONTACT_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACT_SDR_MODEL_H_

#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/SdrModel.h"

using namespace omnetpp;
using namespace std;

class ContactSdrModel final : public SdrModel {
  public:
    ContactSdrModel(int eid, int nodesNumber, ContactPlan* contactPlan);
    ~ContactSdrModel() override;

    int getBytesStoredToNeighbor(int eid) override;
    vector<int> getBundleSizesStoredToNeighbor(int eid);
    vector<int> getBundleSizesStoredToNeighborWithHigherPriority(int eid, bool critical);

    // Initialization and configuration
    void setEid(int eid);
    void setNodesNumber(int nodesNumber);
    void setContactPlan(ContactPlan *contactPlan);
    void setSize(int size);

  private:
    list<BundlePkt *> transmittedBundlesInCustody_;     // Add if not present
    ContactPlan *contactPlan_;
};

#endif /* SRC_NODE_DTN_CONTACT_SDR_MODEL_H_ */
