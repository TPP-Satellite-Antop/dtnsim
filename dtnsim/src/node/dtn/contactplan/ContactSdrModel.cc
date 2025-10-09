#include <list>

#include "ContactSdrModel.h"

#include "Contact.h"
#include "ContactPlan.h"

#include <cassert>
#include <iostream>

ContactSdrModel::ContactSdrModel() {
    bundlesNumber_ = 0;
    bytesStored_ = 0;
}

ContactSdrModel::ContactSdrModel(int eid, int nodesNumber, ContactPlan* contactPlan) {
    this->eid_ = eid;
    this->nodesNumber_ = nodesNumber;
    this->contactPlan_ = contactPlan;
    this->bundlesNumber_ = 0;
    this->bytesStored_ = 0;
}

ContactSdrModel::~ContactSdrModel() {}

/////////////////////////////////////
// Initialization and configuration
//////////////////////////////////////

void ContactSdrModel::setEid(int eid) {
    this->eid_ = eid;
}

void ContactSdrModel::setNodesNumber(int nodesNumber) {
    this->nodesNumber_ = nodesNumber;
}

void ContactSdrModel::setSize(int size) {
    this->size_ = size;
}

void ContactSdrModel::setContactPlan(ContactPlan *contactPlan) {
    this->contactPlan_ = contactPlan;
}

/////////////////////////////////////
// Get information
//////////////////////////////////////

// Returns the total number of bytes stored in the SDR that are intended to be sent to `eid` when a contact becomes available.
int ContactSdrModel::getBytesStoredToNeighbor(const int eid) {
    int totalSize = 0;
    for (const int s : getBundleSizes(eid))
        totalSize += s;
    return totalSize;
}

vector<int> ContactSdrModel::getBundleSizes(const int eid) {
    return getPriorityBundleSizes(eid, false);
}

/*
 * Returns the sizes of all bundles that are currently queued to a neighbor, but only those that
 * have a higher priority
 *
 * @param eid: The EID of the neighbor
 * 		  critical: Whether the bundle to be queued is critical or not
 *
 * @authors: Original Implementation in getBundleSizesStoredToNeighbor() by the authors of DTNSim,
 * general procedure then ported to this function and modified by Simon Rink
 */

vector<int> ContactSdrModel::getPriorityBundleSizes(const int eid, const bool critical) {
    vector<int> sizes;

    for (auto &[contactId, bundlesQueue] : indexedBundleQueue_) {
        if (contactId == 0) // Skip limbo contact
            continue;

        Contact *contact = contactPlan_->getContactById(contactId);
        if (contact == nullptr) {
            continue;
        }

        assert(contact->getSourceEid() == this->eid_);

        if (eid == contact->getDestinationEid()) {
            for (const auto *bundle : bundlesQueue) {
                if (critical && !bundle->getCritical())
                    continue;
                sizes.push_back(bundle->getByteLength());
            }
        }
    }

    return sizes;
}
