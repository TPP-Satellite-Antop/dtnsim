/*
 * BRUFNCopies1TOracle.h
 *
 *  Created on: Feb 11, 2019
 *      Author: fraverta
 */

#ifndef SRC_NODE_DTN_ROUTING_CGRBRUFPOWERED_CGRBRUFPOWERED_H_
#define SRC_NODE_DTN_ROUTING_CGRBRUFPOWERED_CGRBRUFPOWERED_H_

#include <src/node/dtn/routing/RoutingDeterministic.h>
//#include "bp/include/cgr.h"
//#include "ici/include/psm.h"
//#include "ici/include/ion.h"
#include "src/dtnsim_m.h"
//#include "ionadmin.h"
//#include "bpadmin.h"
#include "src/dtnsim_m.h"
#include "src/utils/json.hpp"
#include <src/node/dtn/SdrStatus.h>

using namespace std;
using json = nlohmann::json;

class CGRBRUFPowered : public RoutingDeterministic {
  public:
    CGRBRUFPowered(int eid, SdrModel *sdr, ContactPlan *contactPlan, bool printDebug,
                   double probability_of_failure, int ts_duration, std::vector<int> ts_start_times,
                   int numOfNodes, string pathPrefix, string pathPosfix);
    virtual ~CGRBRUFPowered();
    virtual void routeAndQueueBundle(BundlePkt *bundle, double simTime);
    virtual CgrRoute *getCgrBestRoute(BundlePkt *bundle, double simTime);
    virtual vector<CgrRoute> getCgrRoutes(BundlePkt *bundle, double simTime);

    // stats recollection
    int getDijkstraCalls();
    int getDijkstraLoops();
    int getRouteTableEntriesExplored();

  private:
    int dijkstraCalls;
    int dijkstraLoops;
    int tableEntriesExplored;

    bool printDebug_ = false;

    json bruf_function;
    double contact_failure_probability;
    int ts_duration;
    std::vector<int> ts_start_times;

    double get_node_future_delivery_probability(int source_eid, int target_eid, int carrier_eid,
                                                int ts);
    double get_probability_if_this_route_is_chosed(int source_eid, int target_eid, CgrRoute *route);
    int get_ts_for_contact(Contact *c);

    /////////////////////////////////////////////////
    // Ion Cgr Functions based in libcgr.c (v 3.5.0):
    /////////////////////////////////////////////////
#define MIN_CONFIDENCE_IMPROVEMENT (.05)
#define MIN_DTN_DELIVERY_CONFIDENCE (.80)
#define MAX_XMIT_COPIES (20)
#define MAX_SPEED_MPH (150000)

    typedef struct {
        int neighborNodeNbr;
        int contactId; // This is not, in ION
        double forfeitTime;
        double arrivalTime;
        float confidence;
        unsigned int hopCount; // hops from dest. node.
        CgrRoute *route;       // pointer to route so we can decrement capacities
        // Scalar	overbooked; 	//Bytes needing reforward.
        // Scalar	protected; 		//Bytes not overbooked.

        double success_probability;
    } ProximateNode;

    typedef struct {
        Contact *contact;
        Contact *predecessor; // predecessor Contact
        double arrivalTime;
        double capacity; // in Bytes
        bool visited;
        bool suppressed;
    } Work;

    map<int, vector<CgrRoute>> routeList_;
    double routeListLastEditTime = -1;

    void cgrForward(BundlePkt *bundle, double simTime);
    void identifyProximateNodes(BundlePkt *bundle, double simTime, vector<int> excludedNodes,
                                vector<ProximateNode> *proximateNodes);
    void loadRouteList(int terminusNode, double simTime);

    void findNextBestRoute(Contact *rootContact, int terminusNode, CgrRoute *route);
    void tryRoute(BundlePkt *bundle, CgrRoute *route, vector<ProximateNode> *proximateNodes);
    void recomputeRouteForContact();
    void enqueueToNeighbor(BundlePkt *bundle, ProximateNode *selectedNeighbor);
    void enqueueToLimbo(BundlePkt *bundle);
    void bpEnqueue(BundlePkt *bundle, ProximateNode *selectedNeighbor);
};

#endif /* SRC_NODE_DTN_ROUTING_CGRBRUFPOWERED_CGRBRUFPOWERED_H_ */
