/*
 * MetricCollector.h
 *
 *  Created on: Feb 1, 2022
 *      Author: Simon Rink
 */

#ifndef SRC_UTILS_METRICCOLLECTOR_H_
#define SRC_UTILS_METRICCOLLECTOR_H_

#include "src/utils/json.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <chrono>

using json = nlohmann::json;
using namespace std;
class Metrics {
  public:
    int eid_;
    int RUCoPCalls_ = 0;
    int cgrCalls_ = 0;
    int antopCalls_ = 0;
    map<long, int> bundleHops_;
    map<long, double> bundleElapsedTime_;

    map<long, double> bundleStartTimes_;
    map<long, int> sentBundles_;
    map<long, vector<tuple<int, double>>> routingDecisions_;
    map<long, double> bundleReceivingTimes_; // Decision to be made

    Metrics(){};

    ~Metrics(){};
};

class MetricCollector {
  public:
    MetricCollector();
    virtual ~MetricCollector();

    void updateCGRCalls(int eid);
    void updateAntopCalls(int eid);
    int &calls(int eid);
    void setAlgorithm(string algoritm);
    void setFailureProb(double failureProb);
    void setMode(int mode);
    void initialize(int numOfNodes);
    void updateRUCoPCalls(int eid);
    void setPath(string path);
    void updateStartedBundles(int eid, long bundleId, int sourceEid, int destinationEid,
                              double startTime);
    void updateSentBundles(int eid, int destinationEid, double time, long bundleId);
    void updateSentBundles(int eid, int destinationEid, double time, long bundleId, int numBundles);
    void updateReceivedBundles(int eid, long bundleId, double receivingTime);
    void updateRUCoPComputationTime(long computationTime);
    void updateCGRComputationTime(long computationTime);
    void increaseBundleHops(int eid, long bundleId);
    void updateBundleElapsedTime(int eid, long bundleId, double elapsedTime);
    void evaluateAndPrintResults();
    void evaluateAndPrintContactlessResults();
    int getFileNumber(string prefix);
    int getMode();

  private:
    map<long, double> getOverallSentBundles();
    map<long, double> getOverallReceivedBundles();
    map<long, double> computeDeliveryTimes(map<long, double> startTimes,
                                           map<long, double> receivingTimes);
    map<long, int> getBundleDeliveryCounts();
    string getInformationString(long bundleId, double start);
    string getPrefix();
    int getCGRCalls();
    int getRUCoPCalls();
    int getAntopCalls();
    void putToJson();
    string path_;
    vector<Metrics> nodeMetrics_;
    long RUCoPComputationTime_ = 0;
    long cgrComputationTime_ = 0;
    map<long, tuple<int, int>> bundleInformation_;
    std::chrono::steady_clock::time_point startWalltime;
    string algorithm_;
    double failureProb_;
    int mode;
};

#endif /* SRC_UTILS_METRICCOLLECTOR_H_ */
