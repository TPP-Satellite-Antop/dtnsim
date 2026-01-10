#ifndef _CONTACTLESS_DTN_H_
#define _CONTACTLESS_DTN_H_

#include "src/dtnsim_m.h"
#include "src/node/dtn/Dtn.h"
#include "src/node/dtn/CustodyModel.h"
#include "src/node/dtn/routing/RoutingORUCOP.h"
#include "src/node/dtn/routing/RoutingAntop.h"
#include "src/utils/Observer.h"
#include "src/utils/RouterUtils.h"
#include "src/utils/TopologyUtils.h"
#include <fstream>
#include <map>
#include <omnetpp.h>
#include <string>

class ContactlessDtn : public Dtn {
  public:
    ContactlessDtn();
    virtual ~ContactlessDtn();

    void setOnFault(bool onFault) override;
    void scheduleRetry();
    void setRoutingAlgorithm(Antop* antop);
    void setMobilityMap(map<int, inet::SatelliteMobility*> *mobilityMap);

  protected:
    void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void handleBundle(BundlePkt *bundle);
    void handleForwardingStart(ForwardingMsgStart *fwd);
    void handleRoutingRetry();
    void scheduleBundle(BundlePkt *bundle);
    void scheduleRoutingRetry(BundlePkt *bundle);

  private:
    int eid_;
    Antop* antop;
    map<int, inet::SatelliteMobility*> *mobilityMap_;
    void initializeRouting(string routingString);

    CustodyModel custodyModel_;
    double custodyTimeout_;

    // BundlesMap
    bool saveBundleMap_;
    ofstream bundleMap_;
    vector<BundlePkt*> pendingBundles_;
    std::unordered_map<int, bool> linkAvailability_;

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
};

#endif /* _CONTACTLESS_DTN_H_ */
