#ifndef _CONTACT_DTN_H_
#define _CONTACT_DTN_H_

#include "src/dtnsim_m.h"
#include "src/node/dtn/Dtn.h"
#include "src/node/dtn/CustodyModel.h"
#include "src/node/dtn/SdrModel.h"
#include "src/node/dtn/contactplan/Contact.h"
#include "src/node/dtn/contactplan/ContactHistory.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/routing/RoutingORUCOP.h"
#include "src/utils/Observer.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include <fstream>
#include <map>
#include <omnetpp.h>
#include <string>

class ContactDtn : public Dtn {
  public:
    ContactDtn();
    virtual ~ContactDtn();

    void setOnFault(bool onFault) override;
    void refreshForwarding();
    ContactPlan *getContactPlanPointer();
    void setContactPlan(ContactPlan &contactPlan);
    void setContactTopology(ContactPlan &contactTopology);

    // Opportunistic procedures
    void syncDiscoveredContact(Contact *c, bool start) const;
    void syncDiscoveredContactFromNeighbor(const Contact *c, bool start, int ownEid, int neighborEid) const;
    void scheduleDiscoveredContactStart(Contact *c);
    void scheduleDiscoveredContactEnd(Contact *c);
    ContactHistory *getContactHistory();
    void addDiscoveredContact(Contact c);
    void removeDiscoveredContact(const Contact& c);
    void predictAllContacts(double currentTime);
    void coordinateContactStart(Contact *c) const;
    void coordinateContactEnd(Contact *c) const;
    void notifyNeighborsAboutDiscoveredContact(Contact *c, bool start,
                                               map<int, int> *alreadyInformed);
    void updateDiscoveredContacts(Contact *c);
    map<int, int> getReachableNodes() const;
    void addCurrentNeighbor(int neighborEid);
    void removeCurrentNeighbor(int neighborEid);
    int checkExistenceOfContact(int sourceEid, int destinationEid, int start);

  protected:
    void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void dispatchBundle(BundlePkt *bundle) override;

  private:
    int eid_;
    void initializeRouting(const string& routingString);

    // Forwarding threads
    map<int, ForwardingMsgStart *> forwardingMsgs_;

    // Contact Plan to feed CGR
    // and get transmission rates
    ContactPlan contactPlan_;

    // Contact History used to collect all
    // discovered contacts;
    ContactHistory contactHistory_;

    // Contact Topology to schedule Contacts
    // and get transmission rates
    ContactPlan contactTopology_;

    CustodyModel custodyModel_;
    double custodyTimeout_;

    // BundlesMap
    bool saveBundleMap_;
    ofstream bundleMap_;

    // Signals
    simsignal_t dtnBundleSentToCom;
    simsignal_t dtnBundleSentToApp;
    simsignal_t dtnBundleSentToAppHopCount;
    simsignal_t dtnBundleSentToAppRevisitedHops;
    simsignal_t dtnBundleReceivedFromCom;
    simsignal_t dtnBundleReceivedFromApp;
    simsignal_t dtnBundleReRouted;
    simsignal_t sdrBundleStored;
    simsignal_t sdrBytesStored;
    simsignal_t routeCgrDijkstraCalls;
    simsignal_t routeCgrDijkstraLoops;
    simsignal_t routeCgrRouteTableEntriesCreated;
    simsignal_t routeCgrRouteTableEntriesExplored;
};

#endif /* _CONTACT_DTN_H_ */
