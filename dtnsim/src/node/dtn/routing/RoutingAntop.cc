#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(Antop* antop, const int eid, SdrModel *sdr, map<int, inet::SatelliteMobility *> *mobilityMap): RoutingDeterministic(eid, sdr, nullptr) {
    this->mobilityMap = mobilityMap;
    this->routingTable = new RoutingTable(antop);
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    const H3Index cur = getCurH3IndexForEid(eid_);
    if(cur == 0) {
        std::cout << "Current EID " << eid_ << " is down. Skipping routing" << std::endl;
       // storeBundle(bundle); TODO si solo retornamos no avanza el sim time pero si lo guardo se queda en loop
        return;
    }

    const H3Index dst = getCurH3IndexForEid(bundle->getDestinationEid());
    if(dst == 0) {
        std::cout << "Destination EID " << bundle->getDestinationEid() << " is unreachable (node down). Storing bundle " << bundle->getBundleId() << " in SDR." << std::endl;
        storeBundle(bundle);
        return;
    }

    const H3Index sender = getCurH3IndexForEid(bundle->getSenderEid());
    H3Index nextHop = 0;
    int nextHopEid = 0;

    // Useful print for debugging. ToDo: remove at a later stage.
    {
        std::cout << "Routing:" << std::endl;
        std::cout << "  Bundle: " << std::dec << bundle->getBundleId() << " /// " << bundle->getHopCount() << " /// " << (bundle->getReturnToSender() ? "true" : "false") << std::endl;
        std::cout << "  Current: " << std::dec << eid_ << " /// " << std::hex << cur << std::endl;
        std::cout << "  Source: " << std::dec << bundle->getSourceEid() << " /// " << std::hex << getCurH3IndexForEid(bundle->getSourceEid()) << std::endl;
        std::cout << "  Sender: " << std::dec << bundle->getSenderEid() << " /// " << std::hex << sender << std::endl;
        std::cout << "  Destination: " << std::dec << bundle->getDestinationEid() << " /// " << std::hex << dst << std::endl;

        for (const auto& [eid, a] : *this->mobilityMap) {
            if (eid != 0) {
                // std::cout << std::dec << "(" << a->getLatitude() << ", " << a->getLongitude() << ") /// " << std::hex << this->getCurH3IndexForEid(eid) << std::endl;
                // std::cout << std::hex << this->getCurH3IndexForEid(eid) << std::endl;
                // std::cout << std::dec << eid << "," << eid << "," << a->getLatitude() << "," << a->getLongitude() << std::endl;
                // std::cout << std::dec << a->getLongitude() << "," << a->getLatitude() << std::endl;
                // std::cout << std::dec << a->getLatitude() << "," << a->getLongitude() << std::endl;
            }
        }
    }

    const auto nextUpdateTime = (*mobilityMap)[eid_]->getNextUpdateTime().dbl();

    if (!bundle->getReturnToSender()) {
        const H3Index src = getCurH3IndexForEid(bundle->getSourceEid());
        if (src != 0){
            int hopCount = bundle->getHopCount();
            nextHop = routingTable->findNextHop(cur, src, dst, sender, &hopCount, nextUpdateTime);
            bundle->setHopCount(hopCount);
            nextHopEid = getEidFromH3Index(nextHop);
        } else {
            std::cout << "Source EID " << bundle->getSourceEid() << " is unreachable (node down). Storing bundle " << bundle->getBundleId() << " in SDR." << std::endl;
        }
    }

    while (nextHopEid == 0) {
        nextHop = routingTable->findNewNeighbor(cur, dst, sender == 0 ? cur : sender, nextUpdateTime);
		nextHopEid = getEidFromH3Index(nextHop);
    }

    if (nextHop == cur)
        storeBundle(bundle);
    else {
        bundle->setReturnToSender(nextHop == sender);
        bundle->setNextHopEid(getEidFromH3Index(nextHop));

        std::cout << "Routing through " << std::hex << nextHop << std::dec << " ||| " << getEidFromH3Index(nextHop) << std::endl << std::endl;
    }
}

H3Index RoutingAntop::getCurH3IndexForEid(const int eid) const {
    if (eid == 0) return 0;

    try {
        const inet::SatelliteMobility *mobility = this->mobilityMap->at(eid);
        const auto latLng = LatLng {deg2rad(mobility->getLatitude()), deg2rad(mobility->getLongitude())};
        H3Index cell = 0;

        if (latLngToCell(&latLng, this->routingTable->getAntopResolution(), &cell) != E_SUCCESS){
            cout << "Error converting lat long to cell" << endl;
        }

        return cell;
    } catch (exception& e) {
        cout << "No mobility module found for eid " << eid  << ". Node must be down! " << endl;
        return 0;   
    }
}

int RoutingAntop::getEidFromH3Index(const H3Index idx) {
    for (const auto& [eid, _] : *this->mobilityMap) {
        if (eid != 0 && idx == this->getCurH3IndexForEid(eid)) return eid;
    }

    return 0;
}

// Equeue bundle for later if no candidate was found
void RoutingAntop::storeBundle(BundlePkt *bundle) const {
    if(!sdr_->pushBundle(bundle))
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR" << std::endl;
    else
        std::cout << "Enqueued bundle " << bundle->getBundleId() << " to SDR" << std::endl;
}
