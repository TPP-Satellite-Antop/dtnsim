/*
 * ContactHistory.h
 *
 *  Created on: Nov 16, 2021
 *      Author: Simon Rink
 */

#ifndef SRC_NODE_DTN_CONTACTHISTORY_H_
#define SRC_NODE_DTN_CONTACTHISTORY_H_

#include <cstddef>
#include <map>
#include <math.h>
#include <omnetpp.h>
#include <src/node/dtn/Contact.h>
#include <src/node/dtn/ContactPlan.h>
#include <tuple>
#include <vector>

using namespace std;

class ContactHistory {
  public:
    ContactHistory();
    virtual ~ContactHistory();
    vector<tuple<Contact, Contact>> getContactList();
    map<int, int> getSenderStartPositions();

    // get contact at the specified position and tuplePostion
    Contact *getContact(int position, int tuplePostion);

    // get sender Contact, or Receiver contact if not available
    Contact *getReliableContact(int position);

    // you always have 2 versions of the same contact, one from the sender,
    // one from the receiver, as the receiver is likely to produce uncertain results
    void addContact(Contact *senderContact, Contact *destinationContact);

    // Used when 2 nodes exchange their history information.
    void combineContactHistories(ContactHistory *otherHistory);

    // Called by nodes to predict new contact.
    Contact predictAndAddContact(double currentTime, int sourceEid, int destinationEid,
                                 ContactPlan *contactPlan);
    vector<Contact> predictAndAddAllContacts(double currentTime, ContactPlan *contactPlan);

    // Helper function for contact prediction.
    vector<Contact *> getAllContactsForSourceDestination(int sourceEid, int destinationEid);
    bool isNewContact(int position, Contact newContact);
    double calculateStandardDeviation(vector<double> intervals, double mean);
    double calculateMean(vector<double> intervals);
    double computeNetConfidence(double baseConfidence, int numberOfOccurences);

  private:
    // Lists of all discovered opp. contacts,
    // ordered by Sender id > Receiver id > start time
    // receiver is only used, if a corresponding entry does not exist in the sender list
    vector<tuple<Contact, Contact>> contactList_;

    // Helper to identify the starting positions for each source to speed up read/write operations.
    map<int, int> sourceStartPositions_;

    // contacts that need to be predicted the next time they are predicted
    vector<tuple<int, int>> updatedContacts_;
};

#endif /* SRC_NODE_DTN_CONTACTHISTORY_H_ */
