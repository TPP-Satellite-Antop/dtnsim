/*
 * RoutingBRUF1T.cpp
 *
 *  Created on: Nov 8, 2016
 *      Author: FRaverta
 */

#include <src/node/dtn/routing/RoutingBRUF1T.h>

RoutingBRUF1T::RoutingBRUF1T(int eid, SdrModel *sdr, ContactPlan *contactPlan,
                             string path_to_routing)
    : RoutingDeterministic(eid, sdr, contactPlan) {
    ifstream routing(path_to_routing);

    if (routing.is_open()) {
        while (!routing.eof()) {
            string line;
            getline(routing, line);
            if (line[0] != '#' && line.size() > 0) {
                int from, bundle_source, bundle_target, expire_time, contact_id;
                sscanf(line.c_str(), "%d %d %d %d %d", &from, &bundle_source, &bundle_target,
                       &expire_time, &contact_id);
                if (from == eid)
                    routing_decisions_[bundle_source][bundle_target][expire_time] = contact_id;
            }
        }
        routing.close();
    } else {
        cout << "Error at RoutingBRUF1T::RoutingBRUF1T: Error opening file " << path_to_routing
             << endl;
        exit(1);
    }

    // cout << "[RoutingBRUF1T] node "<< eid << endl;
    //	for (map<int, map<int,int> >::iterator it=routing_decisions_.begin();
    // it!=routing_decisions_.end(); ++it) 		for (map<int,int>::iterator it2=
    // (it->second).begin();
    // it2!=(it->second).end(); ++it2) 			cout <<"TO: "<< it->first << " TS: " <<  it2->first <<
    // " Contact_id_ " << it2->second <<endl;
}

RoutingBRUF1T::~RoutingBRUF1T() {}

void RoutingBRUF1T::routeAndQueueBundle(BundlePkt *bundle, double simTime) {
    int contactId = 0; // contact 0 is the limbo

    // get bundle source routing decisions
    map<int, map<int, map<int, int>>>::iterator source_it =
        routing_decisions_.find(bundle->getSourceEid());
    if (source_it != routing_decisions_.end()) {
        // get bundle target routing decisions
        map<int, map<int, int>>::iterator target_it =
            source_it->second.find(bundle->getDestinationEid());
        if (target_it != source_it->second.end()) {
            // get routing decision for current time stamp
            for (map<int, int>::iterator it2 = (target_it->second).begin();
                 it2 != (target_it->second).end(); ++it2) {
                if (simTime < it2->first) {
                    contactId = it2->second;
                    break;
                }
            }
        }
    }

    // Enqueue the bundle
    if (contactId != 0) {
        bundle->setNextHopEid(contactPlan_->getContactById(contactId)->getDestinationEid());
    }

    sdr_->enqueueBundleToContact(bundle, contactId);
}
