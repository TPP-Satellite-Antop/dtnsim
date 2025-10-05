/*
 * RoutingBRUFNCopies.h
 *
 *  Created on: Feb 4, 2019
 *      Author: fraverta
 */
#ifndef SRC_NODE_DTN_ROUTINGBRUFNCopies_H_
#define SRC_NODE_DTN_ROUTINGBRUFNCopies_H_

#include <src/node/dtn/SdrModel.h>
#include <src/node/dtn/routing/RoutingDeterministic.h>
#include <src/node/dtn/routing/brufncopies/BRUFNCopiesOracle.h>
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/utils/Observer.h"

class RoutingBRUFNCopies : public RoutingDeterministic, public Observer {
  public:
    RoutingBRUFNCopies(int eid, SdrModel *sdr, ContactPlan *contactPlan, int bundlesCopies,
                       int numOfNodes, string pathPrefix, string pathPosfix);
    virtual ~RoutingBRUFNCopies();

    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    virtual void msgToOtherArrive(BundlePkt *bundle, double simTime);
    virtual bool msgToMeArrive(BundlePkt *bundle);
    virtual void contactEnd(Contact *c);
    virtual void update(void);

  private:
    BRUFNCopiesOracle *oracle_;
    int bundlesCopies_;
    map<long, list<BundlePkt *>> copiesList_;
    // The bundles destined to this node that it has already received
    list<int> deliveredBundles_;

    void enqueueToCarryingBundles(BundlePkt *bundle);
    void routeBundle(BundlePkt *bundle, int copies, vector<int> route);

    BundlePkt *getCopiesToSend(long bundle_id, int copies);
    void generateCopies(BundlePkt *bundle);
    BundlePkt *getCarryingBundle(int source, int target);
    string reportErrorAndExit(string method, string msg);

    bool isDeliveredBundle(long bundleId);
};

#endif /* SRC_NODE_DTN_ROUTINGBRUFNCopies */
