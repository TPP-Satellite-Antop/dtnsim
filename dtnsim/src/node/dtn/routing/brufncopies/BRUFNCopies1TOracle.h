/*
 * BRUFNCopies1TOracle.h
 *
 *  Created on: Feb 11, 2019
 *      Author: fraverta
 */

#ifndef SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIES1TORACLE_H_
#define SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIES1TORACLE_H_

#include "src/dtnsim_m.h"
#include "src/utils/json.hpp"
#include <fstream>
#include <iostream>
#include <numeric>
#include <tuple>

using namespace std;
using json = nlohmann::json;

class Action {
  public:
    int copies_;
    vector<int> contact_ids_;
    string name_;
    int source_node_; // Which node is taking the source routing decision

    Action() {}

    ~Action() {}
};

class State {
  public:
    int id_python;
    int id_;
    int ts_;
    vector<int> copies_by_nodes_;
    vector<int> children_;
    vector<Action> actions_; // Decision to be made
    int solved_copies_;

    State(){};

    ~State(){};
};

class BRUFNCopies1TOracle {
  public:
    BRUFNCopies1TOracle(int source, int target, string pathToRouting, int numOfCopies);
    virtual ~BRUFNCopies1TOracle();

    bool createBundle(int copies);
    bool failBundleForwarded(int copies);
    bool succesfulBundleForwarded(int sender, int receiver, int copies, bool end_routing_decision);
    vector<tuple<const int, const vector<int>>> getRoutingDecision(int node_eid);

  private:
    vector<State> mcStates_; // vector containing all the states in the Marcov Chain
    int currentMCState_;     // Position of the current Marcov Chain status in mcStates_ vector

    // Information to decide next state
    int solvedCopies_; // It accounts number of copies that have been solved. When the value of this
                       // variable reaches number Of Copies, the state has to be updated.
    vector<int> currentCopiesByNode_; // Current network state which evolves according bundle
                                      // transmissions. When the state has to be updated it must be
                                      // equal to one of the state's children.
    int currentTimestamp_; // Current time stamp (it is not strictly needed)

    int numOfCopies_;  // Number of copies which have the bundle modeled by this MarcovChain
    const int source_; // The source of the bundled modeled by this MarcovChain.
    const int target_; // The destination of the bundled modeled by this MarcovChain.

    bool incSolvedCopies(int copies);
    void printEstado();
};

#endif /* SRC_NODE_DTN_ROUTING_BRUFNCOPIES_BRUFNCOPIES1TORACLE_H_ */
