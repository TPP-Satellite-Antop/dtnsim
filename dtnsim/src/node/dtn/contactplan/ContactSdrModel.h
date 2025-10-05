/*
 * SdrModel.h
 *
 *  Created on: Nov 25, 2016
 *      Author: juanfraire
 */

#ifndef SRC_NODE_DTN_CONTACT_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACT_SDR_MODEL_H_

#include <map>
#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/SdrModel.h"
#include <src/node/dtn/SdrStatus.h>
#include "src/utils/Subject.h"

using namespace omnetpp;
using namespace std;

class ContactSdrModel : public SdrModel {
  public:
    ContactSdrModel();
    ContactSdrModel(int eid, int nodesNumber, ContactPlan* contactPlan);
    virtual ~ContactSdrModel();

    int getBundlesCountInSdr();
    int getBundlesCountInLimbo();
    list<BundlePkt *> getCarryingBundles();
    virtual BundlePkt *getBundle(long bundleId) override;
    vector<int> getBundleSizesStoredToNeighbor(int eid);
    vector<int> getBundleSizesStoredToNeighborWithHigherPriority(int eid, bool critical);
    list<BundlePkt *> getTransmittedBundlesInCustody();
    BundlePkt* getTransmittedBundleInCustody(long bundleId);
    list<BundlePkt *> *getBundlesInLimbo();
    int getBundlesCountInContact(int cid);
    int getBytesStoredInSdr();
    void removeTransmittedBundleInCustody(long bundleId);
    
    // Initialization and configuration
    void setEid(int eid);
    void setNodesNumber(int nodesNumber);
    void setContactPlan(ContactPlan *contactPlan);
    void setSize(int size);

    bool enqueueTransmittedBundleInCustody(BundlePkt *bundle);

    // overrides
    virtual bool isBundleForId(int id) override;
    virtual bool pushBundleToId(BundlePkt *bundle, int id) override;
    virtual bool pushBundle(BundlePkt *bundle) override;
    virtual void popBundle(long bundleId) override;
    virtual void popBundleFromId(int contactId) override;
    virtual BundlePkt *getBundle(int id) override;
    virtual int getBytesStoredToNeighbor(int eid) override;

  private:
    list<BundlePkt *> transmittedBundlesInCustody_;     // Add if not present
    ContactPlan *contactPlan_;
};

#endif /* SRC_NODE_DTN_CONTACT_SDR_MODEL_H_ */
