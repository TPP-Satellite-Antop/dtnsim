#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(
    Antop* antop,
    const int eid,
    const int nodes,
    const GetPosition &getPosition,
    const GetNextMobilityUpdate &getNextMobilityUpdate
): RoutingDeterministic(eid, nullptr) {
    this->resolution_ = antop->getResolution();
    this->routingTable = new RoutingTable(antop);
    this->getPosition = getPosition;
    this->nodes = nodes;
    this->getNextMobilityUpdate_ = getNextMobilityUpdate;
}

RoutingAntop::~RoutingAntop() = default;

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    bundle->setNextHopEid(eid_); // Default to storing bundle in SDR.

    const H3Index cur = getH3Index(eid_);
    if(cur == 0) return;

    H3Index dst = getH3Index(bundle->getDestinationEid());
    const auto antopPkt = dynamic_cast<AntopPkt*>(bundle);
    if (dst == 0){
        dst = antopPkt->getCachedDstH3Index();
        if (dst == 0) {
            return;
        }
    } else 
        antopPkt->setCachedDstH3Index(dst);

    const H3Index sender = getH3Index(bundle->getSenderEid());
    H3Index nextHop = 0;
    int nextHopEid = 0;

    // Useful print for debugging. ToDo: remove at a later stage.
    /*{
        std::cout << "Routing:" << std::endl;
        std::cout << "  Bundle: " << std::dec << bundle->getBundleId() << " /// " << bundle->getHopCount() << " /// " << (bundle->getReturnToSender() ? "true" : "false") << std::endl;
        std::cout << "  Current: " << std::dec << eid_ << " /// " << std::hex << cur << std::endl;
        std::cout << "  Source: " << std::dec << bundle->getSourceEid() << " /// " << std::hex << getH3Index_(bundle->getSourceEid()) << std::endl;
        std::cout << "  Sender: " << std::dec << bundle->getSenderEid() << " /// " << std::hex << sender << std::endl;
        std::cout << "  Destination: " << std::dec << bundle->getDestinationEid() << " /// " << std::hex << dst << std::endl;
    }*/

    const auto nextUpdateTime = getNextMobilityUpdate_();

    if (!bundle->getReturnToSender()) {
        const H3Index src = getH3Index(bundle->getSourceEid());
        int hopCount = bundle->getHopCount();
        nextHop = routingTable->findNextHop(cur, src, dst, sender, &hopCount, nextUpdateTime);
        bundle->setHopCount(hopCount);
        nextHopEid = getEidFromH3Index(nextHop, dst, bundle->getDestinationEid());
    }

    while (nextHopEid == 0) {
        nextHop = routingTable->findNewNeighbor(cur, dst, sender == 0 ? cur : sender, nextUpdateTime);
        nextHopEid = getEidFromH3Index(nextHop, dst, bundle->getDestinationEid());
    }

    if (nextHop == cur) nextHopEid = eid_;

    bundle->setNextHopEid(nextHopEid);

    if (nextHopEid != eid_)
        bundle->setReturnToSender(nextHop == sender);
}

/**
 * Returns the first valid EID of a node in the target H3 cell. Returns 0 (invalid EID) if no
 * nodes are inside the target H3 cell.
 *
 * @param idx: H3Index of the target cell.
 * @param dst: current H3Index of the bundle being routed.
 * @param dstEid: destination EID of the bundle being routed.
 */
int RoutingAntop::getEidFromH3Index(const H3Index idx, const H3Index dst, const int dstEid) const {
    // If the next hop is the destination, route to destination. If impossible (node is down), save to SDR.
    if (idx == dst) return getH3Index(dstEid) == idx ? dstEid : eid_;

    for (int eid = 1; eid <= nodes; eid++) {
        if (getH3Index(eid) == idx)
            // ToDo: figure a better way of choosing a destination EID as always choosing the first one found
            //       may lead to transmission link saturation.
            //       Potential options are:
            //       - Round-robin.
            //       - Link availability-based election (choose the least busy link).
            return eid;
    }

    return 0;
}

/**
 * Fetches the H3Index of the cell the target EID is in. Returns 0 (invalid H3Index) if unable to
 * obtain the target EID's position, or if the position cannot be mapped to a valid cell.
 *
 * @param eid: endpoint ID of the target node.
 */
H3Index RoutingAntop::getH3Index(const int eid) const {
    try {
        const auto latLng = getPosition(eid);
        H3Index cell = 0;

        if (latLngToCell(&latLng, resolution_, &cell) != E_SUCCESS)
            cout << "Error converting lat long to cell" << endl;

        return cell;
    } catch (exception& _) {
        return 0;
    }
}
