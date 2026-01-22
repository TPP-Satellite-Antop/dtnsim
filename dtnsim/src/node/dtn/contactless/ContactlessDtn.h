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
#include "src/node/mobility/SatelliteMobility.h"

class ContactlessDtn : public Dtn {
  public:
    ContactlessDtn();
    virtual ~ContactlessDtn();

    void setOnFault(bool onFault) override;
    void scheduleRetry();
    void setRoutingAlgorithm(Antop* antop);
    void setMobilityMap(map<int, inet::SatelliteMobility*> *mobilityMap);
    double nextMobilityUpdate();
    // Returns the current H3 index of the node with given eid. Returns 0 if not found.
    H3Index getCurH3IndexForEid(int eid) const;
    int getEidFromH3Index(H3Index idx, H3Index dst, int dstEid);

  protected:
    void initialize(int stage) override;
    void handleMessage(cMessage *msg) override;
    void finish() override;
    void dispatchBundle(BundlePkt *bundle) override;
    void handleBundleForwarding(BundlePkt *bundle);
    virtual void sendMsg(BundlePkt *bundle);
    virtual void retryForwarding();

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
