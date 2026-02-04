#ifndef SRC_NODE_DTN_ROUTINGANTOP_H_
#define SRC_NODE_DTN_ROUTINGANTOP_H_

#include "RoutingTable.h"
#include "h3api.h"
#include <src/node/dtn/routing/RoutingDeterministic.h>

class RoutingAntop : public RoutingDeterministic {
    public:
        using GetPosition = std::function<LatLng(int)>;
        using GetNextMobilityUpdate = std::function<double()>;

        RoutingAntop(
            Antop* antop,
            int eid,
            int nodes, const GetPosition &getPosition,
            const GetNextMobilityUpdate &getNextMobilityUpdate
        );
        virtual ~RoutingAntop();
        void routeAndQueueBundle(BundlePkt *bundle, double simTime) override;

    private:
        int resolution_;
        int nodes;
        GetPosition getPosition;
        GetNextMobilityUpdate getNextMobilityUpdate_;
        RoutingTable *routingTable;

        void routeAndQueueAntopBundle(AntopPkt *bundle, double simTime) const;
        [[nodiscard]] int getEidFromH3Index(H3Index idx, H3Index dst, int dstEid) const;
        [[nodiscard]] H3Index getH3Index(int eid) const;
};

#endif /* SRC_NODE_DTN_ROUTINGANTOP_H_ */
