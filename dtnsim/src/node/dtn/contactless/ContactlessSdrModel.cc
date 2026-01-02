#include "ContactlessSdrModel.h"

ContactlessSdrModel::ContactlessSdrModel() {
    bundlesNumber_ = 0;
    bytesStored_ = 0;
    this->resetEnqueuedBundleFlag();
}

ContactlessSdrModel::ContactlessSdrModel(int eid, int nodesNumber) {
    this->eid_ = eid;
    this->nodesNumber_ = nodesNumber;
    this->bundlesNumber_ = 0;
    this->bytesStored_ = 0;
    this->resetEnqueuedBundleFlag();
}

ContactlessSdrModel::~ContactlessSdrModel() {}

/////////////////////////////////////
// Initialization and configuration
//////////////////////////////////////

void ContactlessSdrModel::setEid(int eid) {
    this->eid_ = eid;
}

void ContactlessSdrModel::setNodesNumber(int nodesNumber) {
    this->nodesNumber_ = nodesNumber;
}

void ContactlessSdrModel::setSize(int size) {
    this->size_ = size;
}

/////////////////////////////////////
// Get information
//////////////////////////////////////

// Returns the total number of bytes stored in the SDR that are intended to be sent to `eid`.
int ContactlessSdrModel::getBytesStoredToNeighbor(const int eid) {
    int totalSize = 0;
    for (const int s : getBundleSizes(eid))
        totalSize += s;
    return totalSize;
}

vector<int> ContactlessSdrModel::getBundleSizes(const int eid) {
    return getPriorityBundleSizes(eid, false);
}

vector<int> ContactlessSdrModel::getPriorityBundleSizes(const int eid, const bool critical) {
    vector<int> sizes;

    auto it = indexedBundleQueue_.find(eid);
    if (it == indexedBundleQueue_.end())
        return sizes;

    const auto &bundlesQueue = it->second;
    sizes.reserve(bundlesQueue.size());

    for (const auto *bundle : bundlesQueue) {
        if (critical && !bundle->getCritical())
            continue;
        sizes.push_back(bundle->getByteLength());
    }

    return sizes;
}

bool ContactlessSdrModel::enqueuedBundle() {
    return enqueuedBundle_;
}

void ContactlessSdrModel::resetEnqueuedBundleFlag() {
    enqueuedBundle_ = false;
}

bool ContactlessSdrModel::pushBundle(BundlePkt *bundle){
    const bool result = SdrModel::pushBundle(bundle);
    if (result)
        enqueuedBundle_ = true;

    return result;
}

BundlePkt* ContactlessSdrModel::popBundle(){
    if (genericBundleQueue_.empty())
        return nullptr;
    auto bundle = genericBundleQueue_.front();
    genericBundleQueue_.pop_front();
    bundlesNumber_--;
    bytesStored_ -= bundle->getByteLength();
    return bundle;
}