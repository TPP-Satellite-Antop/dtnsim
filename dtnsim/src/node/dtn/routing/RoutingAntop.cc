#include "src/node/dtn/routing/RoutingAntop.h"
#include "Antop.h"

RoutingAntop::RoutingAntop(int eid, SdrModel *sdr, ContactPlan *contactPlan)
    : RoutingDeterministic(eid, sdr, contactPlan) {}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    int nextHopId = Antop::getNextHopId(bundle->getSourceEid(), bundle->getDestinationEid()); //TODO importar la lib Antop

    if (nextHopId != 0) {
        bundle->setNextHopEid(contactPlan_->getContactById(nextHopId)->getDestinationEid());
    }

    sdr_->enqueueBundleToContact(bundle, nextHopId);
}
