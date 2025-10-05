/*
 * SdrModel.h
 *
 *  Created on: Nov 25, 2016
 *      Author: juanfraire
 */

#ifndef SRC_NODE_DTN_SDRMODEL_H_
#define SRC_NODE_DTN_SDRMODEL_H_

#include <map>
#include <omnetpp.h>
#include <src/node/dtn/SdrStatus.h>
#include "src/utils/Subject.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/dtnsim_m.h"

using namespace omnetpp;
using namespace std;

class SdrModel : public Subject {
  public:
    SdrModel();
    virtual ~SdrModel();

    virtual int getBundlesCountInSdr();
    virtual int getBundlesCountInContact(int cid);
    virtual int getBundlesCountInLimbo();
    virtual list<BundlePkt *> *getBundlesInLimbo();
    virtual int getBytesStoredInSdr();
    virtual int getBytesStoredToNeighbor(int eid);
    virtual SdrStatus getSdrStatus();
    virtual BundlePkt *getEnqueuedBundle(long bundleId);
    virtual bool isSdrFreeSpace(int sizeNewPacket);
    virtual void freeSdr(int eid);

    // Enqueue and dequeue from indexedBundleQueue_
    virtual bool pushBundleToId(BundlePkt *bundle, int id);
    virtual bool isBundleForId(int id);
    virtual BundlePkt *getBundle(int id);
    virtual void popBundleFromId(int contactId);

    // Enqueue and dequeue generic
    virtual bool pushBundle(BundlePkt *bundle);
    virtual void popBundle(long bundleId);
    virtual BundlePkt *getBundle(long bundleId);
    virtual list<BundlePkt *> getCarryingBundles();

    // Enqueue and dequeue from transmittedBundlesInCustody_
    virtual bool enqueueTransmittedBundleInCustody(BundlePkt *bundle);
    virtual void removeTransmittedBundleInCustody(long bundleId);
    virtual BundlePkt *getTransmittedBundleInCustody(long bundleId);
    virtual list<BundlePkt *> getTransmittedBundlesInCustody();

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
    // arrives with either custody acceptance or rejection of a remote node
    list<BundlePkt *> transmittedBundlesInCustody_;
};

#endif /* SRC_NODE_DTN_SDRMODEL_H_ */
