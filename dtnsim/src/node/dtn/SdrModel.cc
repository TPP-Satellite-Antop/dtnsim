#include "src/node/dtn/SdrModel.h"

SdrModel::SdrModel() {
    bundlesNumber_ = 0;
    bytesStored_ = 0;
}

SdrModel::~SdrModel() {}

/////////////////////////////////////
// Initialization and configuration
//////////////////////////////////////

void SdrModel::freeSdr(int eid) {
    // Delete all enqueued bundles
    map<int, list<BundlePkt *>>::iterator it1 = indexedBundleQueue_.begin();
    map<int, list<BundlePkt *>>::iterator it2 = indexedBundleQueue_.end();
    while (it1 != it2) {
        list<BundlePkt *> bundles = it1->second;

        while (!bundles.empty()) {
            delete (bundles.back());
            bundles.pop_back();
        }
        indexedBundleQueue_.erase(it1++);
    }

    // delete all messages in carriedBundles
    while (!genericBundleQueue_.empty()) {
        delete (genericBundleQueue_.back());
        genericBundleQueue_.pop_back();
    }

    // delete all messages in transmittedBundlesInCustody
    while (!transmittedBundlesInCustody_.empty()) {
        delete (transmittedBundlesInCustody_.back());
        transmittedBundlesInCustody_.pop_back();
    }

    bundlesNumber_ = 0;
    bytesStored_ = 0;
    notify();
}

/////////////////////////////////////
// Get information
//////////////////////////////////////

int SdrModel::getBundlesCountInSdr() {
    return bundlesNumber_;
}

int SdrModel::getBundlesCountInLimbo() {
    return indexedBundleQueue_[0].size();
}

list<BundlePkt *> *SdrModel::getBundlesInLimbo() {
    return &indexedBundleQueue_[0];
}

int SdrModel::getBundlesCountInContact(int cid) {
    return indexedBundleQueue_[cid].size();
}

int SdrModel::getBytesStoredInSdr() {
    return bytesStored_;
}

SdrStatus SdrModel::getSdrStatus() {
    SdrStatus sdrStatus;

    for (int i = 1; i <= nodesNumber_; i++) {
        sdrStatus.size[i] = this->getBytesStoredToNeighbor(i);
    }

    return sdrStatus;
}

bool SdrModel::isSdrFreeSpace(int sizeNewPacket) {
    if (this->size_ == 0)
        return true;
    else
        return (bytesStored_ + sizeNewPacket <= this->size_) ? true : false;
}

/////////////////////////////////////
// Enqueue and dequeue from perContactBundleQueue_
//////////////////////////////////////

bool SdrModel::pushBundleToId(BundlePkt *bundle, int id) {
    // if there is not enough space in sdr, the bundle is deleted
    // if another behavior is required, the simpleCustodyModel should be used
    // to avoid bundle deletions
    if (!(this->isSdrFreeSpace(bundle->getByteLength()))) {
        delete bundle;
        return false;
    }

    // Check is queue exits, if not, create it. Add bundle to queue.
    map<int, list<BundlePkt *>>::iterator it = indexedBundleQueue_.find(id);
    if (it != indexedBundleQueue_.end()) {
        // if custody report, enqueue it at the front so it is prioritized
        // over data bundles already in the queue
        if (bundle->getBundleIsCustodyReport())
            it->second.push_front(bundle);
        else
            it->second.push_back(bundle);
    } else {
        list<BundlePkt *> q;
        q.push_back(bundle);
        indexedBundleQueue_[id] = q;
    }

    bundlesNumber_++;
    bytesStored_ += bundle->getByteLength();
    notify();
    return true;
}

bool SdrModel::isBundleForId(int id) {
    // This functions returns true if there is a queue
    // with bundles for the contactId. If it is empty
    // or non-existent, the function returns false

    map<int, list<BundlePkt *>>::iterator it = indexedBundleQueue_.find(id);

    if (it != indexedBundleQueue_.end()) {
        if (!indexedBundleQueue_[id].empty())
            return true;
        else
            return false;
    } else {
        return false;
    }
}

BundlePkt *SdrModel::getBundle(int id) {
    map<int, list<BundlePkt *>>::iterator it = indexedBundleQueue_.find(id);

    // Just check if the function was called incorrectly
    if (it == indexedBundleQueue_.end())
        if (indexedBundleQueue_[id].empty()) {
            cout << "***getBundle called from SdrModel but queue empty***" << endl;
            exit(1);
        }

    // Find and return pointer to bundle
    list<BundlePkt *> bundlesToTx = it->second;

    return bundlesToTx.front();
}

void SdrModel::popBundleFromId(int contactId) {
    // Pop the next bundle for this contact
    map<int, list<BundlePkt *>>::iterator it = indexedBundleQueue_.find(contactId);
    list<BundlePkt *> bundlesToTx = it->second;

    int size = bundlesToTx.front()->getByteLength();
    bundlesToTx.pop_front();

    // Update queue after popping the bundle
    if (!bundlesToTx.empty())
        indexedBundleQueue_[contactId] = bundlesToTx;
    else
        indexedBundleQueue_.erase(contactId);

    bundlesNumber_--;
    bytesStored_ -= size;
    notify();
}

/////////////////////////////////////
// Enqueue and dequeue from genericBundleQueue_
//////////////////////////////////////

bool SdrModel::pushBundle(BundlePkt *bundle) {
    // if there is not enough space in sdr, the bundle is deleted
    // if another behaviour is required, the simpleCustodyModel should be used
    // to avoid bundle deletions
    if (!(this->isSdrFreeSpace(bundle->getByteLength()))) {
        cout << "SDRModel::enqueuBundle(BundlePkt * bundle): Bundle exceed sdr capacity so it was "
                "not enqueue."
             << endl;
        delete bundle;
        return false;
    }

    genericBundleQueue_.push_back(bundle);
    bundlesNumber_++;
    bytesStored_ += bundle->getByteLength();
    notify();
    return true;
}

// Delete bundle with bundleId from genericBundleQueue_ if it exists.
void SdrModel::popBundle(long bundleId) {
    for (list<BundlePkt *>::iterator it = genericBundleQueue_.begin();
         it != genericBundleQueue_.end(); it++)
        if ((*it)->getBundleId() == bundleId) {
            int size = (*it)->getByteLength();
            delete (*it);
            genericBundleQueue_.erase(it);
            bundlesNumber_--;
            bytesStored_ -= size;
            notify();
            break;
        }
}

list<BundlePkt *> SdrModel::getCarryingBundles() {
    return genericBundleQueue_;
}

BundlePkt *SdrModel::getEnqueuedBundle(long bundleId) {
    for (list<BundlePkt *>::iterator it = genericBundleQueue_.begin();
         it != genericBundleQueue_.end(); it++)
        if ((*it)->getBundleId())
            return *it;

    return NULL;
}

/////////////////////////////////////
// Enqueue and dequeue from transmittedBundlesInCustody_
//////////////////////////////////////

bool SdrModel::enqueueTransmittedBundleInCustody(BundlePkt *bundle) {
    cout << "Node " << eid_
         << " enqueueTransmittedBundleInCustody bundleId: " << bundle->getBundleId() << endl;

    // If the bundle is already in memory, there is nothing to do
    BundlePkt *bundleInCustody = this->getTransmittedBundleInCustody(bundle->getBundleId());
    if (bundleInCustody != NULL)
        return true;

    // if there is not enough space in sdr, the bundle is deleted
    // if another behaviour is required, the simpleCustodyModel should be used
    // to avoid bundle deletions
    if (!(this->isSdrFreeSpace(bundle->getByteLength()))) {
        cout << "SDRModel::enqueueTransmittedBundleInCustody(BundlePkt * bundle): Bundle exceed "
                "sdr capacity so it was not enqueue."
             << endl;
        delete bundle;
        return false;
    }

    transmittedBundlesInCustody_.push_back(bundle);
    bundlesNumber_++;
    bytesStored_ += bundle->getByteLength();
    notify();
    return true;
}

void SdrModel::removeTransmittedBundleInCustody(long bundleId) {
    cout << "Node " << eid_ << " removeTransmittedBundleInCustody bundleId: " << bundleId << endl;

    for (list<BundlePkt *>::iterator it = transmittedBundlesInCustody_.begin();
         it != transmittedBundlesInCustody_.end(); it++)
        if ((*it)->getBundleId() == bundleId) {
            int size = (*it)->getByteLength();
            delete (*it);
            it = transmittedBundlesInCustody_.erase(it);
            bundlesNumber_--;
            bytesStored_ -= size;
            notify();
            // break; // remove all possible instances of the same id
        }
}

BundlePkt *SdrModel::getTransmittedBundleInCustody(long bundleId) {
    for (list<BundlePkt *>::iterator it = transmittedBundlesInCustody_.begin();
         it != transmittedBundlesInCustody_.end(); it++)
        if ((*it)->getBundleId() == bundleId)
            return (*it);
    return NULL;
}

list<BundlePkt *> SdrModel::getTransmittedBundlesInCustody() {
    return transmittedBundlesInCustody_;
}

int SdrModel::getBytesStoredToNeighbor(int eid) {
    // TODO: Implement logic or return a default value
    return 0;
}

BundlePkt* SdrModel::getBundle(long bundleId) {
    // TODO: Implement logic or return nullptr
    return nullptr;
}