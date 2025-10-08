#include <list>

#include "ContactSdrModel.h"
#include "ContactPlan.h"

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
int ContactSdrModel::getBytesStoredToNeighbor(int eid) {
    int size = 0;

    map<int, list<BundlePkt *>>::iterator it1 = indexedBundleQueue_.begin();
    map<int, list<BundlePkt *>>::iterator it2 = indexedBundleQueue_.end();

    for (; it1 != it2; ++it1) {
        int contactId = it1->first;

        // if it's not the limbo contact
        if (contactId != 0) {
            list<BundlePkt *> bundlesQueue = it1->second;

            Contact *contact = contactPlan_->getContactById(contactId);
            int source = contact->getSourceEid();
            assert(source == this->eid_);

            int destination = contact->getDestinationEid();

            if (eid == destination) {
                list<BundlePkt *>::iterator ii1 = bundlesQueue.begin();
                list<BundlePkt *>::iterator ii2 = bundlesQueue.end();
                for (; ii1 != ii2; ++ii1) {
                    size += (*ii1)->getByteLength();
                }
            }
        }
    }

    return size;
}

vector<int> ContactSdrModel::getBundleSizesStoredToNeighbor(int eid) {
    vector<int> sizes;

    map<int, list<BundlePkt *>>::iterator it1 = indexedBundleQueue_.begin();
    map<int, list<BundlePkt *>>::iterator it2 = indexedBundleQueue_.end();

    for (; it1 != it2; ++it1) {
        int contactId = it1->first;

        // if it's not the limbo contact
        if (contactId != 0) {
            list<BundlePkt *> bundlesQueue = it1->second;

            Contact *contact = contactPlan_->getContactById(contactId);
            if (contact == NULL) {
                continue;
            }
            int source = contact->getSourceEid();
            assert(source == this->eid_);

            int destination = contact->getDestinationEid();

            if (eid == destination) {
                list<BundlePkt *>::iterator ii1 = bundlesQueue.begin();
                list<BundlePkt *>::iterator ii2 = bundlesQueue.end();
                for (; ii1 != ii2; ++ii1) {
                    sizes.push_back((*ii1)->getByteLength());
                }
            }
        }
    }

    return sizes;
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

vector<int> ContactSdrModel::getBundleSizesStoredToNeighborWithHigherPriority(int eid, bool critical) {
    vector<int> sizes;

    map<int, list<BundlePkt *>>::iterator it1 = indexedBundleQueue_.begin();
    map<int, list<BundlePkt *>>::iterator it2 = indexedBundleQueue_.end();

    for (; it1 != it2; ++it1) {
        int contactId = it1->first;

        // if it's not the limbo contact
        if (contactId != 0) {
            list<BundlePkt *> bundlesQueue = it1->second;

            Contact *contact = contactPlan_->getContactById(contactId);
            if (contact == NULL) {
                continue;
            }

            int source = contact->getSourceEid();
            assert(source == this->eid_);

            int destination = contact->getDestinationEid();

            if (eid == destination) {
                list<BundlePkt *>::iterator ii1 = bundlesQueue.begin();
                list<BundlePkt *>::iterator ii2 = bundlesQueue.end();
                for (; ii1 != ii2; ++ii1) {
                    if (critical) {
                        if ((*ii1)->getCritical()) {
                            sizes.push_back((*ii1)->getByteLength());
                        }
                    } else {
                        sizes.push_back((*ii1)->getByteLength());
                    }
                }
            }
        }
    }

    return sizes;
}
