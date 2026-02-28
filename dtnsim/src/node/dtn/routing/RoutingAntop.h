#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "RoutingTable.h"
#include "h3api.h"

#include <optional>
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
    public:
        using GetPosition = std::function<optional<LatLng>(int)>;
        using UpdatePosition = std::function<void(int, double, double)>;
        using GetNextMobilityUpdate = std::function<double()>;
        using GetQueuedBundlesCount = std::function<optional<std::tuple<int, double>>(int)>;

        RoutingAntop(
            Antop* antop,
            int eid,
            int nodes,
            std::shared_ptr<std::unordered_map<H3Index, std::vector<int>>> eidsByH3Cell,
            const GetPosition &getPosition,
            const GetQueuedBundlesCount &getQueuedBundlesCount,
            const GetNextMobilityUpdate &getNextMobilityUpdate
        );
        ~RoutingAntop() override;
        void routeAndQueueBundle(BundlePkt *bundle, double simTime) override;

    private:
        int resolution_;
        int nodes;
        GetPosition getPosition;
        GetQueuedBundlesCount getQueuedBundlesCount;
        GetNextMobilityUpdate getNextMobilityUpdate_;
        RoutingTable *routingTable;
        std::shared_ptr<std::unordered_map<H3Index, std::vector<int>>> eidsByH3Cell_;

        void routeAndQueueAntopBundle(AntopPkt *bundle, double simTime) const;
        [[nodiscard]] int getEidFromH3Index(H3Index idx, H3Index dst, int dstEid) const;
        [[nodiscard]] H3Index getH3Index(int eid) const;
        void updatePosition(int eid, double lat, double lng) override;
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
