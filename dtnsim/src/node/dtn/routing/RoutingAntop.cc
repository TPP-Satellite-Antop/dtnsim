#include <functional>
#include "src/node/dtn/routing/RoutingAntop.h"

RoutingAntop::RoutingAntop(
    Antop* antop,
    const int eid,
    const int nodes,
    std::shared_ptr<std::unordered_map<H3Index, std::vector<int>>> eidsByH3Cell,
    const GetPosition &getPosition,
    const GetQueuedBundlesCount &getQueuedBundlesCount,
    const GetNextMobilityUpdate &getNextMobilityUpdate
): RoutingDeterministic(eid, nullptr), eidsByH3Cell_(std::move(eidsByH3Cell)) {
    this->nodes = nodes;
    this->resolution_ = antop->getResolution();
    this->routingTable = new RoutingTable(antop);
    this->getPosition = getPosition;
    this->getQueuedBundlesCount = getQueuedBundlesCount;
    this->getNextMobilityUpdate_ = getNextMobilityUpdate;
}

RoutingAntop::~RoutingAntop() = default;

void RoutingAntop::routeAndQueueBundle(BundlePkt *bundle, const double simTime) {
    routeAndQueueAntopBundle(dynamic_cast<AntopPkt*>(bundle), simTime);
}

void RoutingAntop::routeAndQueueAntopBundle(AntopPkt *bundle, const double simTime) const {
    bundle->setNextHopEid(eid_); // Default to storing bundle in SDR.

    const H3Index cur = getH3Index(eid_);
    if(cur == 0) return;

    H3Index dst = getH3Index(bundle->getDestinationEid());
    if (dst == 0){
        dst = bundle->getCachedDstH3Index();
        if (dst == 0) return;
    } else
        bundle->setCachedDstH3Index(dst);

    const auto nextUpdateTime = getNextMobilityUpdate_();
    const H3Index src = getH3Index(bundle->getSourceEid());
    const H3Index sender = getH3Index(bundle->getSenderEid());
    int loopEpoch = bundle->getLoopEpoch();
    int hopCount = bundle->getHopCount();
    int nextHopEid = 0;
    H3Index nextHop = 0;

    // Useful print for debugging. ToDo: remove at a later stage.
    /* {
        std::cout << "Routing:" << std::endl;
        std::cout << "  Bundle: " << std::dec << bundle->getBundleId() << " /// " << bundle->getHopCount() << " /// " << bundle->getLoopEpoch() << std::endl;
        std::cout << "  Current: " << std::dec << eid_ << " /// " << std::hex << cur << std::endl;
        std::cout << "  Source: " << std::dec << bundle->getSourceEid() << " /// " << std::hex << getH3Index(bundle->getSourceEid()) << std::endl;
        std::cout << "  Sender: " << std::dec << bundle->getSenderEid() << " /// " << std::hex << sender << std::endl;
        std::cout << "  Destination: " << std::dec << bundle->getDestinationEid() << " /// " << std::hex << dst << std::endl;
    } */

    nextHop = routingTable->findNextHop(cur, src, dst, sender, &hopCount, &loopEpoch, nextUpdateTime);
    nextHopEid = getEidFromH3Index(nextHop, dst, bundle->getDestinationEid());

    bundle->setHopCount(hopCount);
    bundle->setLoopEpoch(loopEpoch);

    while (nextHopEid == 0) {
        nextHop = routingTable->findNewNeighbor(cur, dst, sender == 0 ? cur : sender, nextUpdateTime);
        nextHopEid = getEidFromH3Index(nextHop, dst, bundle->getDestinationEid());
    }

    if (nextHop == cur) nextHopEid = eid_;

    bundle->setNextHopEid(nextHopEid);

    if (nextHopEid != eid_) bundle->setReturnToSender(nextHop == sender);
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

    int bestCandidate = 0;

    if (const auto it = eidsByH3Cell_->find(idx); it != eidsByH3Cell_->end()) {
        int bestCandidateQueuedBundles = 0;
        double bestCandidateFwdArrivalTime = 0;

        for (const int eid : it->second) {
            const auto queueInfo = getQueuedBundlesCount(eid);
            if (!queueInfo) continue;

            const auto [queuedBundles, fwdArrivalTime] = *queueInfo;

            if (queuedBundles == 0 && fwdArrivalTime == 0) {
                return eid;
            }

            if (bestCandidate == 0 || bestCandidateQueuedBundles > queuedBundles || (bestCandidateQueuedBundles == queuedBundles && fwdArrivalTime < bestCandidateFwdArrivalTime)) {
                bestCandidate = eid;
                bestCandidateQueuedBundles = queuedBundles;
                bestCandidateFwdArrivalTime = fwdArrivalTime;
            }
        }
    }

    return bestCandidate;
}

/**
 * Fetches the H3Index of the cell the target EID is in. Returns 0 (invalid H3Index) if unable to
 * obtain the target EID's position, or if the position cannot be mapped to a valid cell.
 *
 * @param eid: endpoint ID of the target node.
 */
H3Index RoutingAntop::getH3Index(const int eid) const {
    const auto latLng = getPosition(eid);
    if (!latLng) return 0;

    H3Index cell = 0;

    if (latLngToCell(&*latLng, resolution_, &cell) != E_SUCCESS)
        cout << "Error converting lat long to cell" << endl;

    return cell;
}

void RoutingAntop::updatePosition(const int eid, const double lat, const double lng) {
    if (eid == 0) {
        eidsByH3Cell_->clear();
        return;
    }
    const auto latLng = LatLng {lat, lng};
    H3Index cell = 0;

    if (latLngToCell(&latLng, resolution_, &cell) != E_SUCCESS) {
        cout << "Error converting lat long to cell" << endl;
        exit(1); // This branch should be unreachable.
    }

    (*eidsByH3Cell_)[cell].push_back(eid);
}
