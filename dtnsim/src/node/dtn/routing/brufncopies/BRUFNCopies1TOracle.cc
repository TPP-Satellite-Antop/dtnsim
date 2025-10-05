/*
 * BRUFNCopies1TOracle.cpp
 *
 *  Created on: Feb 11, 2019
 *      Author: fraverta
 */

#include <fstream>
#include <src/node/dtn/routing/brufncopies/BRUFNCopies1TOracle.h>

void from_json(const json &j, Action &a) {
    j.at("copies").get_to(a.copies_);
    j.at("name").get_to(a.name_);
    j.at("contact_ids").get_to(a.contact_ids_);
    j.at("source_node").get_to(a.source_node_);
}

void from_json(const json &j, State &s) {
    j.at("id_python").get_to(s.id_python);
    j.at("ts").get_to(s.ts_);
    j.at("copies_by_node").get_to(s.copies_by_nodes_);
    j.at("actions").get_to(s.actions_);
    j.at("children").get_to(s.children_);
    j.at("solved_copies").get_to(s.solved_copies_);
}

BRUFNCopies1TOracle::BRUFNCopies1TOracle(int source, int target, string pathToRouting,
                                         int numOfCopies)
    : source_(source), target_(target) {
    ifstream routing(pathToRouting);
    if (routing.is_open()) {
        json j_complete = json::parse(routing);

        for (unsigned int i = 0; i < j_complete.size(); i++)
            mcStates_.push_back(j_complete[i].get<State>());

        this->currentMCState_ = 0;
        this->numOfCopies_ = numOfCopies;
        this->solvedCopies_ = mcStates_.at(currentMCState_).solved_copies_;
        this->currentTimestamp_ = mcStates_.at(currentMCState_).ts_;
        this->currentCopiesByNode_ = mcStates_.at(currentMCState_).copies_by_nodes_;

        cout << "BRUFNCopies1TOracle::BRUFNCopies1TOracle : Initialize routing traffic from "
             << source_ << " to " << target_ << endl;
    } else {
        cout << "Error at BRUFNCopies1TOracle::BRUFNCopies1TOracle : Error opening file "
             << pathToRouting << endl;
        exit(1);
    }
}

BRUFNCopies1TOracle::~BRUFNCopies1TOracle() {
    // TODO Auto-generated destructor stub
}

bool BRUFNCopies1TOracle::createBundle(int copies) {
    currentCopiesByNode_[this->source_ - 1] += copies;
    return incSolvedCopies(copies);
}

bool BRUFNCopies1TOracle::succesfulBundleForwarded(int sender, int receiver, int copies,
                                                   bool end_routing_decision) {
    this->currentCopiesByNode_.at(sender - 1) -= copies;
    this->currentCopiesByNode_.at(receiver - 1) += copies;

    // If the bundle reached their destination in the current time stamp, report that this copies
    // were solved
    if (end_routing_decision)
        return incSolvedCopies(copies);
    else
        return false;
}

bool BRUFNCopies1TOracle::failBundleForwarded(int copies) {
    return incSolvedCopies(copies);
}

bool BRUFNCopies1TOracle::incSolvedCopies(int copies) {
    solvedCopies_ += copies;
    cout << "BRUFNCopies1TOracle::incSolvedCopies() - " << source_ << " -> " << target_
         << " solved_copies_ = " << solvedCopies_ << endl;

    if (solvedCopies_ == numOfCopies_) {
        cout << "BRUFNCopies1TOracle::incSolvedCopies() " << source_ << " -> " << target_
             << " State " << currentMCState_
             << " - Python_id: " << mcStates_.at(currentMCState_).id_python
             << " need to be updated." << endl;
        vector<int> children = mcStates_.at(currentMCState_).children_;
        vector<int>::iterator it;
        for (it = children.begin(); it != children.end(); ++it) {
            State child = this->mcStates_.at(*it);
            if (child.copies_by_nodes_ == currentCopiesByNode_ &&
                child.ts_ == currentTimestamp_ + 1) {
                currentMCState_ = *it;
                currentTimestamp_++;
                solvedCopies_ = 0;
                cout << "BRUFNCopies1TOracle::incSolvedCopies() " << source_ << " -> " << target_
                     << " State update to " << currentMCState_
                     << " - Python_id: " << mcStates_.at(currentMCState_).id_python << endl;
                if (child.solved_copies_ < numOfCopies_) {
                    solvedCopies_ = child.solved_copies_;
                    return true; // Nodes must be notified about mc update
                } else
                    return incSolvedCopies(
                        child.solved_copies_); // Otherwise no routing decision need to be took in
                                               // current timestamp, increase to the next one
            }
        }
        if (children.size() > 0) {
            //			for(it = children.begin(); it != children.end(); ++it)
            //			{
            //				State child = mcStates_.at(*it);
            //				if(accumulate(child.copies_by_nodes_.begin(),
            //child.copies_by_nodes_.end(), 0) > numOfCopies_)
            //				{
            //					//Sink state was reached
            //					currentMCState_ = (*it);
            //					cout<<"BRUFNCopies1TOracle::incSolvedCopies()
            //Traffic"<<source_<<"
            //->
            //"<<target_<<" Sink state([Python_id = "<< child.id_python <<"]) was reached."<<endl;
            //					return true;
            //				}
            //			}
            //			cout<<"BRUFNCopies1TOracle::incSolvedCopies() "<<source_<<" ->
            //"<<target_<<" Error must update child but it fails. [Current MC State Python_id = " <<
            // mcStates_.at(currentMCState_).id_python; 			cout<<"] Current network
            // state:
            // "; 			printEstado(); 			cout<<endl;
            // exit(1);
        } else {
            // othewise a final state was reached
            cout << "BRUFNCopies1TOracle::incSolvedCopies() : Final state [Python_id = "
                 << mcStates_.at(currentMCState_).id_python << "] was reached." << endl;
            return true;
        }
    }

    return false;
}

vector<tuple<const int, const vector<int>>> BRUFNCopies1TOracle::getRoutingDecision(int node_eid) {
    vector<Action> actions = mcStates_.at(currentMCState_).actions_;
    vector<tuple<const int, const vector<int>>> result;

    for (vector<Action>::iterator it = actions.begin(); it != actions.end(); ++it) {
        Action action = *it;

        if (it->contact_ids_.size() > 0 and it->source_node_ == node_eid) {
            tuple<const int, const vector<int>> routingDecision(action.copies_,
                                                                action.contact_ids_);
            result.push_back(routingDecision);
        }
    }

    return result;
}

void BRUFNCopies1TOracle::printEstado() {
    cout << "[";
    for (auto it = currentCopiesByNode_.begin(); it != currentCopiesByNode_.end(); ++it)
        cout << " " << *it;
    cout << " ]";
}
