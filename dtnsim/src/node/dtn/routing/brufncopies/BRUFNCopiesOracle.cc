/*
 * BRUFNCopiesOracle.cpp
 *
 *  Created on: Feb 4, 2019
 *      Author: fraverta
 */

#include <fstream>
#include <src/node/dtn/routing/brufncopies/BRUFNCopiesOracle.h>

BRUFNCopiesOracle *BRUFNCopiesOracle::instance_ = NULL;

BRUFNCopiesOracle::BRUFNCopiesOracle(int numOfNodes, int numOfCopies, string pathPrefix,
                                     string pathPosfix) {
    // It looks up for all routing files  {pathPrefix}{source}-{target}{pathPosfix}
    for (int source = 1; source <= numOfNodes; source++) {
        for (int target = 1; target <= numOfNodes; target++) {
            if (source != target) {
                ostringstream stream;
                stream << pathPrefix << source - 1 << "-" << target - 1 << pathPosfix;
                string pathToRouting = stream.str();
                cout << "BRUFNCopiesOracle::BRUFNCopiesOracle check if exist file: "
                     << pathToRouting << endl;
                ifstream infile(pathToRouting);
                if (infile.good()) {
                    BRUFNCopies1TOracle *oracleij =
                        new BRUFNCopies1TOracle(source, target, pathToRouting, numOfCopies);
                    oneTrafficOracles_[source][target] = oracleij;
                } else {
                    cout << "BRUFNCopiesOracle::BRUFNCopiesOracle there is not traffic from "
                         << source << " to " << target << endl;
                    // exit(1);
                }
            }
        }
    }
    // Initialize notify variables. -1 means that any Markov Chain is being updated
    notifySource_ = -1;
    notifyTarget_ = -1;
}

/**
 * Sigleton method
 */
BRUFNCopiesOracle *BRUFNCopiesOracle::getInstance(int numOfNodes, int numOfCopies,
                                                  string pathPrefix, string pathPosfix) {
    if (instance_ == 0)
        instance_ = new BRUFNCopiesOracle(numOfNodes, numOfCopies, pathPrefix, pathPosfix);

    return instance_;
}

BRUFNCopiesOracle::~BRUFNCopiesOracle() {
    // Delete oneTrafficOracles_
    auto it1 = oneTrafficOracles_.begin();
    auto it2 = oneTrafficOracles_.end();
    while (it1 != it2) {
        map<int, BRUFNCopies1TOracle *> *mapi = &(it1->second);
        map<int, BRUFNCopies1TOracle *>::iterator it3 = mapi->begin();
        map<int, BRUFNCopies1TOracle *>::iterator it4 = mapi->end();
        while (it3 != it4) {
            delete it3->second; // delete BRUFNCopies1TOracle
            mapi->erase(it3++); // delete key
        }
        oneTrafficOracles_.erase(it1++);
    }
}

void BRUFNCopiesOracle::createBundle(int source, int target, int copies) {
    BRUFNCopies1TOracle *oracle = get1TOracle(source, target);
    if (oracle->createBundle(copies))
        notifyST(source, target);
}

void BRUFNCopiesOracle::succesfulBundleForwarded(int source, int target, int sender, int receiver,
                                                 int copies, bool endRoutingDecision) {
    BRUFNCopies1TOracle *oracle = get1TOracle(source, target);

    if (oracle->succesfulBundleForwarded(sender, receiver, copies, endRoutingDecision))
        notifyST(source, target);
}

void BRUFNCopiesOracle::failBundleForwarded(int source, int target, int copies) {
    BRUFNCopies1TOracle *oracle = get1TOracle(source, target);

    if (oracle->failBundleForwarded(copies))
        notifyST(source, target);
}

vector<tuple<const int, const vector<int>>>
BRUFNCopiesOracle::getRoutingDecision(int node_eid, int &source, int &target) {
    BRUFNCopies1TOracle *oracle = get1TOracle(notifySource_, notifyTarget_);
    source = notifySource_;
    target = notifyTarget_;

    return oracle->getRoutingDecision(node_eid);
}

void BRUFNCopiesOracle::notifyST(int source, int target) {
    if (this->notifySource_ != -1 or this->notifyTarget_ != -1) {
        cout << "BRUFNCopiesOracle::notifyST : Error Multiple updates simultaneously" << endl;
        exit(1);
    }
    notifySource_ = source;
    notifyTarget_ = target;

    notify(); // Notify nodes to perform routing decision for source->target traffic.

    this->notifySource_ = -1;
    this->notifyTarget_ = -1;
}

BRUFNCopies1TOracle *BRUFNCopiesOracle::get1TOracle(int source, int target) {
    try {
        return oneTrafficOracles_[source][target];
    } catch (out_of_range &e) {
        cout << "BRUFNCopiesOracle::get1TOracle : there isn't oracle for traffic from " << source
             << " to " << target << endl;
        exit(1);
    }
}

void BRUFNCopiesOracle::finish() {
    if (instance_ != nullptr) {
        delete instance_;
        instance_ = nullptr;
    }
}
