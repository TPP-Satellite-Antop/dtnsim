#ifndef _DTN_H_
#define _DTN_H_

#include "src/dtnsim_m.h"
#include "src/node/dtn/CustodyModel.h"
#include "src/node/dtn/SdrModel.h"
#include "src/node/dtn/contactless/ContactlessSdrModel.h"
#include "src/node/dtn/routing/Routing.h"
#include "src/node/dtn/routing/RoutingORUCOP.h"
#include "src/node/dtn/routing/RoutingAntop.h"
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

class ContactlessDtn : public cSimpleModule, public Observer {
  public:
    ContactlessDtn();
    virtual ~ContactlessDtn();

    virtual void setOnFault(bool onFault);
    virtual void refreshForwarding();
    virtual void setMetricCollector(MetricCollector *metricCollector);
    virtual Routing *getRouting();

    virtual void update();

    // Opportunistic procedures
    void predictAllContacts(double currentTime); map<int, int> *alreadyInformed;
    map<int, int> getReachableNodes() const;
    void addCurrentNeighbor(int neighborEid);
    void removeCurrentNeighbor(int neighborEid);
    void setRoutingAlgorithm(Antop* antop);
    void setMobilityMap(map<int, SatSGP4Mobility*> *mobilityMap);

  protected:
    virtual void initialize(int stage);
    virtual int numInitStages() const;
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

    virtual void dispatchBundle(BundlePkt *bundle);

  private:
    int eid_;
    bool onFault = false;
    Antop* antop_;
    map<int, SatSGP4Mobility*> *mobilityMap_;
    void initializeRouting(string routingString);

    // Pointer to grahics module
    Graphics *graphicsModule;

    // Forwarding threads
    map<int, cMessage*> forwardingMsgs_;

    // Routing and storage
    Routing *routing;

    // An observer that collects and evaluates all the necessary simulation metrics
    MetricCollector *metricCollector_;

    CustodyModel custodyModel_;
    double custodyTimeout_;

    ContactlessSdrModel sdr_;

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
