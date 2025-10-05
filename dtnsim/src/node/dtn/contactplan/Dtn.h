#ifndef _DTN_H_
#define _DTN_H_

#include "src/dtnsim_m.h"
#include "src/node/dtn/CustodyModel.h"
#include "src/node/dtn/SdrModel.h"
#include "src/node/dtn/contactplan/Contact.h"
#include "src/node/dtn/contactplan/ContactHistory.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/dtn/routing/Routing.h"
#include "src/node/dtn/routing/RoutingORUCOP.h"
#include "src/node/graphics/Graphics.h"
#include "src/utils/MetricCollector.h"
#include "src/utils/Observer.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include <fstream>
#include <map>
#include <omnetpp.h>
#include <string>

using namespace omnetpp;
using namespace std;

class Dtn : public cSimpleModule, public Observer {
  public:
    Dtn();
    virtual ~Dtn();

    virtual void setOnFault(bool onFault);
    virtual void refreshForwarding();
    ContactPlan *getContactPlanPointer();
    virtual void setContactPlan(ContactPlan &contactPlan);
    virtual void setContactTopology(ContactPlan &contactTopology);
    virtual void setMetricCollector(MetricCollector *metricCollector);
    virtual Routing *getRouting();

    virtual void update();

    // Opportunistic procedures
    void syncDiscoveredContact(Contact *c, bool start);
    void syncDiscoveredContactFromNeighbor(Contact *c, bool start, int ownEid, int neighborEid);
    void scheduleDiscoveredContactStart(Contact *c);
    void scheduleDiscoveredContactEnd(Contact *c);
    ContactHistory *getContactHistory();
    void addDiscoveredContact(Contact c);
    void removeDiscoveredContact(Contact c);
    void predictAllContacts(double currentTime);
    void coordinateContactStart(Contact *c);
    void coordinateContactEnd(Contact *c);
    void notifyNeighborsAboutDiscoveredContact(Contact *c, bool start,
                                               map<int, int> *alreadyInformed);
    void updateDiscoveredContacts(Contact *c);
    map<int, int> getReachableNodes();
    void addCurrentNeighbor(int neighborEid);
    void removeCurrentNeighbor(int neighborEid);
    int checkExistenceOfContact(int sourceEid, int destinationEid, int start);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const;
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void dispatchBundle(BundlePkt *bundle);

  private:
    int eid_;
    bool onFault = false;
    void initializeRouting(string routingString);

    // Pointer to grahics module
    Graphics *graphicsModule;

    // Forwarding threads
    map<int, ForwardingMsgStart *> forwardingMsgs_;

    // Routing and storage
    Routing *routing;

    // Contact Plan to feed CGR
    // and get transmission rates
    ContactPlan contactPlan_;

    // Contact History used to collect all
    // discovered contacts;
    ContactHistory contactHistory_;

    // An observer that collects and evaluates all the necessary simulation metrics
    MetricCollector *metricCollector_;

    // Contact Topology to schedule Contacts
    // and get transmission rates
    ContactPlan contactTopology_;

    CustodyModel custodyModel_;
    double custodyTimeout_;

    SdrModel sdr_;

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

#endif /* DTN_H_ */
