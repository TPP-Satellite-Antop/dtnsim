/*
 * MetricCollector.h
 *
 *  Created on: Feb 1, 2022
 *      Author: Simon Rink
 */

#ifndef SRC_UTILS_METRICCOLLECTOR_H_
#define SRC_UTILS_METRICCOLLECTOR_H_

#include "src/utils/json.hpp"
#include <chrono>
#include <iostream>
#include <map>
#include <omnetpp/simtime.h>
#include <string>
#include <vector>

using json = nlohmann::json;
using namespace std;
class Metrics {
  public:
    int eid_;
    int RUCoPCalls_ = 0;
    int cgrCalls_ = 0;
    map<long, double> bundleStartTimes_;
    map<long, int> sentBundles_;
    map<long, vector<tuple<int, double>>> routingDecisions_;
    map<long, double> bundleReceivingTimes_; // Decision to be made

    Metrics() =default;
    ~Metrics()= default;
};

struct ArrivalInfo
{
  omnetpp::SimTime generationTime = omnetpp::SimTime::ZERO;
  omnetpp::SimTime arrivalTime = omnetpp::SimTime::ZERO;
};

class MetricCollector {
  public:
    MetricCollector();
    virtual ~MetricCollector();

    void updateCGRCalls(int eid);
    void setAlgorithm(const string& algoritm);
    void setFailureProb(double failureProb);
    void setMode(int newMode);
    void initialize(int numOfNodes);
    void updateRUCoPCalls(int eid);
    void setPath(const string& path);
    void updateStartedBundles(int eid, long bundleId, int sourceEid, int destinationEid,
                              double startTime);
    void updateSentBundles(int eid, int destinationEid, double time, long bundleId);
    void updateSentBundles(int eid, int destinationEid, double time, long bundleId, int numBundles);
    void updateReceivedBundles(int eid, long bundleId, double receivingTime);
    void updateRUCoPComputationTime(long computationTime);
    void updateCGRComputationTime(long computationTime);
    void setNumberOfHops(long bundleId, int hops);
    void updateBundleElapsedTime(long bundleId, std::chrono::steady_clock::time_point elapsedTimeStart);
    void intializeArrivalTime(long bundleId, const omnetpp::SimTime& initialTime);
    void setFinalArrivalTime(long bundleId, const omnetpp::SimTime& finalTime);
    void evaluateAndPrintResults();
    void evaluateAndPrintJsonResults();
    static int getFileNumber(const string& prefix);
    int getMode() const;

  private:
    [[nodiscard]] map<long, double> getOverallSentBundles() const;
    [[nodiscard]] map<long, double> getOverallReceivedBundles() const;
    static map<long, double> computeDeliveryTimes(map<long, double> startTimes,
                                           const map<long, double>& receivingTimes);
    [[nodiscard]] map<long, int> getBundleDeliveryCounts() const;
    string getInformationString(long bundleId, double start);
    [[nodiscard]] string getPrefix() const;
    [[nodiscard]] int getCGRCalls() const;
    [[nodiscard]] int getRUCoPCalls() const;
    string path_;
    vector<Metrics> nodeMetrics_;
    long RUCoPComputationTime_ = 0;
    long cgrComputationTime_ = 0;
    map<long, tuple<int, int>> bundleInformation_;
    std::chrono::steady_clock::time_point startWalltime;
    string algorithm_;
    double failureProb_;
    int mode;
    int nodesNumber_;

    map<long, int> bundleHops_; // number of hops per bundle
    map<long, double> bundleElapsedTime_; // total elapsed time per bundle in seconds. It measures the time spent
                                          // processing the bundle in each node (handleMessage + routing).
                                          // Does not measure time spent waiting in queues.
    map<long, ArrivalInfo> bundleArrivalTime_; // generation and arrival time per bundle. With this info it is then
                                               // possible to compute final arrival time (arrival - generation)
};

#endif /* SRC_UTILS_METRICCOLLECTOR_H_ */
