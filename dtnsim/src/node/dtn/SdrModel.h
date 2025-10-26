#ifndef SRC_NODE_DTN_SDRMODEL_H_
#define SRC_NODE_DTN_SDRMODEL_H_

#include <map>
#include <omnetpp.h>
#include "src/node/dtn/SdrStatus.h"
#include "src/utils/Subject.h"
#include "src/dtnsim_m.h"

using namespace omnetpp;
using namespace std;

class SdrModel : public Subject {
  public:
    SdrModel();
    ~SdrModel() override;

    virtual int getBytesStoredToNeighbor(int eid) = 0;
    virtual vector<int> getBundleSizes(int eid) = 0;
    virtual vector<int> getPriorityBundleSizes(int eid, bool critical) = 0;

    virtual int getBundlesCountInSdr() final;
    virtual int getBundlesCountInIndex(int id) final;
    virtual int getBundlesCountInLimbo() final;
    virtual list<BundlePkt *> *getBundlesInLimbo() final;
    virtual int getBytesStoredInSdr() final;
    virtual SdrStatus getSdrStatus() final;
    virtual BundlePkt *getEnqueuedBundle(long bundleId) final;
    virtual bool isSdrFreeSpace(int sizeNewPacket) final;
    virtual void freeSdr() final;

    // Interface for indexed bundle queue.
    virtual bool pushBundleToId(BundlePkt *bundle, int id) final;
    virtual bool isBundleForId(int id) final;
    virtual BundlePkt *getBundle(int id) final;
    virtual void popBundleFromId(int id) final;

    // Interface for generic bundle queue.
    virtual bool pushBundle(BundlePkt *bundle);
    virtual void popBundle(long bundleId) final;
    virtual list<BundlePkt *> getCarryingBundles() final;

    // Enqueue and dequeue from transmittedBundlesInCustody_
    virtual bool enqueueTransmittedBundleInCustody(BundlePkt *bundle) final;
    virtual void removeTransmittedBundleInCustody(long bundleId) final;
    virtual BundlePkt *getTransmittedBundleInCustody(long bundleId) final;
    virtual list<BundlePkt *> getTransmittedBundlesInCustody() final;

  protected:
    int size_;          // Capacity of sdr in bytes
    int eid_;           // Local eid of the node
    int nodesNumber_;   // Number of nodes in the network
    int bytesStored_;   // Total Bytes stored in Sdr
    int bundlesNumber_; // Total bundles enqueued in all sdr queues (index, generic, in custody)

    // Indexed queues where index can be used by routing algorithms
    // to enqueue bundles to specific contacts or nodes. When there
    // is no need for an indexed queue, a generic one can be used instead
    map<int, list<BundlePkt *>> indexedBundleQueue_;
    list<BundlePkt *> genericBundleQueue_;

    // A separate area of memory to store transmitted bundles for which
    // the current node is custodian. Bundles are removed as custody reports
    // arrive with either custody acceptance or rejection of a remote node
    list<BundlePkt *> transmittedBundlesInCustody_;
};

#endif /* SRC_NODE_DTN_SDRMODEL_H_ */
