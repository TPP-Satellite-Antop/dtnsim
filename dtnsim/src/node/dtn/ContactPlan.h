#ifndef CONTACTPLAN_H_
#define CONTACTPLAN_H_

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <omnetpp.h>
#include <queue>
#include <src/node/dtn/Contact.h>
#include <vector>
#include "PositionEntry.h"

using namespace std;
using namespace omnetpp;

class ContactPlan {

  public:
    ContactPlan();
    virtual ~ContactPlan();
    ContactPlan(ContactPlan &contactPlan);

    // Contact plan population functions
    int addContact(double start, double end, int sourceEid, int destinationEid, double dataRate,
                   double confidence);
    int addContact(double start, double end, int sourceEid, int destinationEid, double dataRate,
                   double confidence, double failureProbability);
    int addDiscoveredContact(double start, double end, int sourceEid, int destinationEid,
                             double dataRate, double confidence);
    int addDiscoveredContact(double start, double end, int sourceEid, int destinationEid,
                             double dataRate, double confidence, double failureProbability);
    int addPredictedContact(double start, double end, int sourceEid, int destinationEid,
                            double dataRate, double confidence);
    void addRange(double start, double end, int sourceEid, int destinationEid, double range,
                  double confidence);
    void addCurrentNeighbor(int eid);

    // Contact plan exploration functions
    Contact *getContactById(int id);
    vector<Contact> *getContacts();
    vector<Contact> *getRanges();
    vector<Contact> getContactsBySrc(int Src);
    vector<int> *getContactIdsBySrc(int Src);
    vector<Contact> getContactsByDst(int Dst);
    vector<Contact> getContactsBySrcDst(int Src, int Dst);
    vector<Contact> getDiscoveredContacts();
    vector<int> getCurrentNeighbors();
    Contact *getContactBySrcDstStart(int sourceEid, int destinationEid, double start);
    int getNodesNumber();
    unordered_map<TimeInterval, vector<PositionEntry>> getNodePositions();
    double getRangeBySrcDst(int Src, int Dst);
    void parseContactPlanFile(string fileName, int nodesNumber, int mode, double failureProb);
    void parseAction(std::string &command, double start, double end, int sourceEid,
                     int destinationEid, double dataRateOrRange, double failureProbability,
                     int mode, std::string &fileLine, int &retFlag);
    void parseOpportunisticContactPlanFile(string fileName, int nodesNumber, int mode,
                                           double failureProb);
    void setFailureProb(double failureProb, double &failureProbability);
    void setContactsFile(string contactsFile);
    const string &getContactsFile() const;
    simtime_t getLastEditTime();
    int getHighestId();

    // delete contact function
    vector<Contact>::iterator deleteContactById(int contactId);
    Contact removePredictedContactForSourceDestination(int sourceEid, int destinationEid);
    Contact removeDiscoveredContact(int sourceEid, int destinationEid);
    void removeCurrentNeighbor(int eid);
    void deleteOldContacts();

    // debug function
    void printContactPlan();

    // Checked when discovered/predicted contacts are inserted.
    bool overlapsWithContact(int sourceEid, int destinationEid, double start, double end);
    bool hasPredictedContact(int sourceEid, int destinationEid);
    bool hasDiscoveredContact(int sourceEid, int destinationEid);

  private:
    void updateContactRanges();
    void sortContactIdsBySrcByStartTime();

    static const int DELETED_CONTACT = -1;
    int nextContactId = 1;
    vector<int> currentNeighbors_;                // The current neighbors of a node
    map<double, vector<int>> contactDeleteTimes_; // The times at which a contact needs to be
                                                  // deleted
    vector<Contact> contacts_;
    vector<Contact> ranges_;
    vector<vector<int>> contactIdsBySrc_;
    vector<int> contactIdShift_ = {0}; // create dummy element for limbo contact
    simtime_t lastEditTime;
    string contactsFile_;
    int nodesNumber_;
    unordered_map<TimeInterval, vector<PositionEntry>> nodePositions_;
};

#endif /* CONTACTPLAN_H_ */
