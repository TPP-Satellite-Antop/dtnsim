#include "src/node/dtn/SdrModel.h"

SdrModel::SdrModel() {
    bundlesNumber_ = 0;
    bytesStored_ = 0;
}

SdrModel::~SdrModel() = default;

/////////////////////////////////////
// Initialization and configuration
//////////////////////////////////////

void SdrModel::freeSdr() {
    // Delete all enqueued bundles
    for (auto it = indexedBundleQueue_.begin(); it != indexedBundleQueue_.end(); it = indexedBundleQueue_.erase(it)) {
        for (const auto *bundle : it->second) {
            delete bundle;
        }
    }

    // Delete all messages in carriedBundles
    while (!genericBundleQueue_.empty()) {
        delete (genericBundleQueue_.back());
        genericBundleQueue_.pop_back();
    }

    // Delete all messages in transmittedBundlesInCustody
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

int SdrModel::getBundlesCountInIndex(const int id) {
    return indexedBundleQueue_[id].size();
}

int SdrModel::getBytesStoredInSdr() {
    return bytesStored_;
}

SdrStatus SdrModel::getSdrStatus() {
    SdrStatus sdrStatus{};

    for (int i = 1; i <= nodesNumber_; i++) {
        sdrStatus.size[i] = this->getBytesStoredToNeighbor(i);
    }

    return sdrStatus;
}

bool SdrModel::isSdrFreeSpace(int sizeNewPacket) {
    if (this->size_ == 0)
        return true;

    return bytesStored_ + sizeNewPacket <= this->size_;
}

/////////////////////////////////////
// Enqueue and dequeue from perContactBundleQueue_
//////////////////////////////////////

bool SdrModel::pushBundleToId(BundlePkt *bundle, int id) {
    // if there is not enough space in sdr, the bundle is deleted
    // if another behavior is required, the simpleCustodyModel should be used
    // to avoid bundle deletions
    if (!this->isSdrFreeSpace(bundle->getByteLength())) {
        delete bundle;
        return false;
    }

    auto& queue = indexedBundleQueue_[id];  // creates a new list if not found
    if (bundle->getBundleIsCustodyReport())
        queue.push_front(bundle); // Pushed to the front so it is prioritized over data bundles already in the queue
    else
        queue.push_back(bundle);

    bundlesNumber_++;
    bytesStored_ += bundle->getByteLength();
    notify();
    return true;
}

bool SdrModel::isBundleForId(const int id) {
    // This functions returns true if there is a queue with bundles for the provided id.
    // If it is empty or non-existent, the function returns false.
    const auto it = indexedBundleQueue_.find(id);
    return it != indexedBundleQueue_.end() && !indexedBundleQueue_[id].empty();
}

BundlePkt *SdrModel::getBundle(int id) {
    const auto it = indexedBundleQueue_.find(id);

    // Just check if the function was called incorrectly
    if (it == indexedBundleQueue_.end())
        if (indexedBundleQueue_[id].empty()) {
            cout << "***getBundle called from SdrModel but queue empty***" << endl;
            exit(1);
        }

    return it->second.front(); // ToDo: if queue is empty, this might cause the program to panic.
}
void SdrModel::popBundleFromId(const int id) {
    auto it = indexedBundleQueue_.find(id);
    if (it == indexedBundleQueue_.end() || it->second.empty()) {
        std::cerr << "*** popBundleFromId called with missing or empty queue ***" << std::endl;
        return;
    }

    BundlePkt* bundle = it->second.front();
    const int size = bundle->getByteLength();

    it->second.pop_front();

    if (it->second.empty())
        indexedBundleQueue_.erase(it);

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
void SdrModel::popBundle(const long bundleId) {
    for (auto it = genericBundleQueue_.begin(); it != genericBundleQueue_.end(); ++it) {
        if ((*it)->getBundleId() == bundleId) {
            const int size = (*it)->getByteLength();
            delete (*it);
            genericBundleQueue_.erase(it);
            bundlesNumber_--;
            bytesStored_ -= size;
            notify();
            break;
        }
    }
}

list<BundlePkt *> SdrModel::getCarryingBundles() {
    return genericBundleQueue_;
}

BundlePkt *SdrModel::getEnqueuedBundle(long bundleId) {
    for (const auto & it : genericBundleQueue_) {
        if (it->getBundleId())
            return it;
    }
    return nullptr;
}

/////////////////////////////////////
// Enqueue and dequeue from transmittedBundlesInCustody_
//////////////////////////////////////

bool SdrModel::enqueueTransmittedBundleInCustody(BundlePkt *bundle) {
    cout << "Node " << eid_ << " enqueueTransmittedBundleInCustody bundleId: " << bundle->getBundleId() << endl;

    // If the bundle is already in memory, there is nothing to do
    if (const BundlePkt *bundleInCustody = this->getTransmittedBundleInCustody(bundle->getBundleId()); bundleInCustody != nullptr)
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

void SdrModel::removeTransmittedBundleInCustody(const long bundleId) {
    cout << "Node " << eid_ << " removeTransmittedBundleInCustody bundleId: " << bundleId << endl;

    for (auto it = transmittedBundlesInCustody_.begin(); it != transmittedBundlesInCustody_.end(); ++it)
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

BundlePkt *SdrModel::getTransmittedBundleInCustody(const long bundleId) {
    for (const auto & it : transmittedBundlesInCustody_) {
        if (it->getBundleId() == bundleId)
            return it;
    }
    return nullptr;
}

list<BundlePkt *> SdrModel::getTransmittedBundlesInCustody() {
    return transmittedBundlesInCustody_;
}
