#ifndef SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_

#include <map>
#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/SdrModel.h"
#include <src/node/dtn/SdrStatus.h>
#include "src/utils/Subject.h"

using namespace omnetpp;
using namespace std;

class ContactlessSdrModel : public SdrModel {
  public:
    ContactlessSdrModel();
    ContactlessSdrModel(int eid, int nodesNumber);
    virtual ~ContactlessSdrModel();

    int getBundlesCountInSdr();
    list<BundlePkt *> getCarryingBundles();
    virtual BundlePkt *getBundle(long bundleId);
    vector<int> getBundleSizesStoredToNeighbor(int eid);
    vector<int> getBundleSizesStoredToNeighborWithHigherPriority(int eid, bool critical);
    list<BundlePkt *> getTransmittedBundlesInCustody();
    BundlePkt* getTransmittedBundleInCustody(long bundleId);
    list<BundlePkt *> *getBundlesInLimbo();
    int getBundlesCountInIndex(int id);
    int getBytesStoredInSdr();
    void removeTransmittedBundleInCustody(long bundleId);
    
    // Initialization and configuration
    void setEid(int eid);
    void setNodesNumber(int nodesNumber);
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
};

#endif /* SRC_NODE_DTN_CONTACTLESS_SDR_MODEL_H_ */
