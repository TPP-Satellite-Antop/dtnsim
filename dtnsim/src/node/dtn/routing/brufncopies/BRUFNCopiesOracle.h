/*
 * BRUFNCopiesOracle.h
 *
 * Created on: Feb 4, 2019
 *      Author: fraverta
 *
 *  * This class implements an Oracle which is aware of the following network events:
 * 	-> Bundle creation
 * 	-> Successful Bundle  transmission
 * 	-> fail Bundle transmission
 *
 * Using that information, the oracle makes proper routing decision which maximizes the probability
 * of delivering network traffic. These routing decision are carried on by network nodes. In order
 * to make the proper routing decision for each bundle on the network, the oracle consults the
 * corresponding BRUFNCopies1TOracle. There is one BRUFNCopies1TOracle for each traffic (
 * source->destination ) on the network. Each one has a Markov Chain which encodes all plausible
 * network status and the routing decision that have to be made in each case. That information is
 * provided to network nodes through BRUFNCopiesOracle.
 */

#ifndef SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIESORACLE_H_
#define SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIESORACLE_H_

#include <src/node/dtn/routing/brufncopies/BRUFNCopies1TOracle.h>
#include "src/dtnsim_m.h"
#include "src/utils/Subject.h"
#include "src/utils/json.hpp"

class BRUFNCopiesOracle : public Subject {

  public:
    static BRUFNCopiesOracle *getInstance(int numOfNodes, int numOfCopies, string pathPrefix,
                                          string pathPosfix);
    static void finish();

    void createBundle(int source, int target, int copies);
    void succesfulBundleForwarded(int source, int target, int sender, int receiver, int copies,
                                  bool endRoutingDecision);
    void failBundleForwarded(int source, int target, int copies);
    vector<tuple<const int, const vector<int>>> getRoutingDecision(int node_eid, int &source,
                                                                   int &target);

  private:
    static BRUFNCopiesOracle *instance_;
    map<int, map<int, BRUFNCopies1TOracle *>> oneTrafficOracles_;
    int notifySource_;
    int notifyTarget_;

    BRUFNCopiesOracle(int numOfNodes, int numOfCopies, string pathPrefix, string pathPosfix);
    virtual ~BRUFNCopiesOracle();
    void notifyST(int source, int target);
    BRUFNCopies1TOracle *get1TOracle(int source, int target);
};

#endif /* SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIESORACLE_H_ */
