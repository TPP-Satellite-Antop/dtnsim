#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(Antop* antop, const int eid, SdrModel *sdr, map<int, inet::SatelliteMobility *> *mobilityMap): RoutingDeterministic(eid, sdr, nullptr), routingTable(antop) {
    this->antopAlgorithm = antop;
    this->mobilityMap = mobilityMap;
    this->routingTable = RoutingTable(antop);
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    const H3Index cur = getCurH3IndexForEid(eid_);
    const H3Index dst = getCurH3IndexForEid(bundle->getDestinationEid());
    const H3Index sender = getCurH3IndexForEid(bundle->getSenderEid());
    H3Index nextHop = 0;
    int nextHopEid = 0;

    std::cout << "Received bundle " << bundle->getBundleId() << " from " << bundle->getSenderEid() << " at " << eid_ << " to " << bundle->getDestinationEid() << std::endl;
    std::cout << "Received bundle " << bundle->getBundleId() << std::hex << " from " << sender << " at " << cur << std::endl;
    std::cout << "Routing to " << dst << " with distance " << std::dec << bundle->getHopCount() << std::endl;
    std::cout << "Node " << eid_ << " routing bundle " << bundle->getBundleId() << " from src " << bundle->getSourceEid() << ", sender " << bundle->getSenderEid() << " to " << bundle->getDestinationEid() << std::endl;

    while (nextHopEid == 0) {
        if (bundle->getReturnToSender() || bundle->getSenderEid())
            nextHop = routingTable.findNewNeighbor(cur, dst, sender);
        else {
            const H3Index src = getCurH3IndexForEid(bundle->getSourceEid());
            nextHop = routingTable.findNextHop(cur, src, dst, sender, bundle->getHopCount());
        }

        nextHopEid = getEidFromH3Index(nextHop);
    }

    if (nextHopEid == cur)
        storeBundle(bundle);
    else {
        bundle->setReturnToSender(nextHop == sender);
        bundle->setBundleId(getEidFromH3Index(nextHop));

        std::cout << "Routing to " << std::hex << nextHop << std::dec << "|||" << getEidFromH3Index(nextHop) << std::endl << std::endl;
    }

    // ToDo:
    // - Make sure bundles are being forwarded.
    // - Check when we should save a bundle to SDR.
    // - Validate invalid next hops and invalid EIDs for next hop.
}

H3Index RoutingAntop::getCurH3IndexForEid(const int eid) const {
    if (eid == 0) return 0;

    try {
        const inet::SatelliteMobility *mobility = this->mobilityMap->at(eid);
        const auto latLng = LatLng {mobility->getLatitude(), mobility->getLongitude()};
        H3Index cell = 0;

        if (latLngToCell(&latLng, this->antopAlgorithm->getResolution(), &cell) != E_SUCCESS){
            cout << "Error converting lat long to cell" << endl;
        }

        return cell;
    } catch (const std::out_of_range& e) {
        cout << "Error in antop routing: no mobility module found for eid " << eid << endl;
        return 0;   
    }
}

int RoutingAntop::getEidFromH3Index(const H3Index idx) {
    int eid = 0;

    for (const auto& [id, _] : *this->mobilityMap) {
        if (idx == this->getCurH3IndexForEid(id)) {
            eid = id;
            break;
        }
    }

    return eid;
}

// Equeue bundle for later if no candidate was found
void RoutingAntop::storeBundle(BundlePkt *bundle) const {
    if(!sdr_->pushBundle(bundle)) // ToDo: handle failed push.
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR" << std::endl;
    else
        std::cout << "Enqueued bundle " << bundle->getBundleId() << " to SDR" << std::endl;
}
