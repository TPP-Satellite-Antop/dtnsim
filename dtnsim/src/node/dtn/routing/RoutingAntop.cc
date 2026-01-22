#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(Antop* antop, const int eid, SdrModel *sdr,
                           GetH3Fn getH3Index,
                           GetEidFromH3Fn getEidFromH3,
                           NextMobilityUpdateFn nextMobilityUpdateFn): RoutingDeterministic(eid, sdr, nullptr) {
    this->routingTable = new RoutingTable(antop);
    this->getH3Index_ = getH3Index;
    this->getEidFromH3Index_ = getEidFromH3;
    this->nextMobilityUpdateFn_ = nextMobilityUpdateFn;
}

RoutingAntop::~RoutingAntop() {}

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    const H3Index cur = getH3Index_(eid_);
    if(cur == 0) {
        std::cout << "Current EID " << eid_ << " is down. Skipping routing" << std::endl;
        storeBundle(bundle);
        return;
    }

    H3Index dst = getH3Index_(bundle->getDestinationEid());
    const auto antopPkt = static_cast<AntopPkt*>(bundle);
    if (dst == 0){
        dst = antopPkt->getCachedDstH3Index();
        if (dst == 0) {
            storeBundle(bundle);
            return;
        }
    } else 
        antopPkt->setCachedDstH3Index(dst);

    const H3Index sender = getH3Index_(bundle->getSenderEid());
    H3Index nextHop = 0;
    int nextHopEid = 0;

    // Useful print for debugging. ToDo: remove at a later stage.
    {
        std::cout << "Routing:" << std::endl;
        std::cout << "  Bundle: " << std::dec << bundle->getBundleId() << " /// " << bundle->getHopCount() << " /// " << (bundle->getReturnToSender() ? "true" : "false") << std::endl;
        std::cout << "  Current: " << std::dec << eid_ << " /// " << std::hex << cur << std::endl;
        std::cout << "  Source: " << std::dec << bundle->getSourceEid() << " /// " << std::hex << getH3Index_(bundle->getSourceEid()) << std::endl;
        std::cout << "  Sender: " << std::dec << bundle->getSenderEid() << " /// " << std::hex << sender << std::endl;
        std::cout << "  Destination: " << std::dec << bundle->getDestinationEid() << " /// " << std::hex << dst << std::endl;
    }

    const auto nextUpdateTime = nextMobilityUpdateFn_();

    if (!bundle->getReturnToSender()) {
        const H3Index src = getH3Index_(bundle->getSourceEid());
        int hopCount = bundle->getHopCount();
        nextHop = routingTable->findNextHop(cur, src, dst, sender, &hopCount, nextUpdateTime);
        bundle->setHopCount(hopCount);
        nextHopEid = getEidFromH3Index_(nextHop, dst, bundle->getDestinationEid());
    }

    while (nextHopEid == 0) {
        nextHop = routingTable->findNewNeighbor(cur, dst, sender == 0 ? cur : sender, nextUpdateTime);
        nextHopEid = getEidFromH3Index_(nextHop, dst, bundle->getDestinationEid());
    }

    if (nextHop == cur || nextHopEid == eid_)
        storeBundle(bundle);
    else {
        bundle->setReturnToSender(nextHop == sender);
        bundle->setNextHopEid(nextHopEid);

        std::cout << "Routing through " << std::hex << nextHop << std::dec << " ||| " << nextHopEid << std::endl;
    }
}

// Equeue bundle for later if no candidate was found
void RoutingAntop::storeBundle(BundlePkt *bundle) const {
    if(!sdr_->pushBundle(bundle))
        std::cout << "Failed to enqueue bundle " << bundle->getBundleId() << " to SDR" << std::endl;
    else
        std::cout << "Enqueued bundle " << bundle->getBundleId() << " to SDR" << std::endl;
}