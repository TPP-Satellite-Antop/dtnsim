/*
 * MetricCollector.cc
 *
 *  Created on: Feb 1, 2022
 *      Author: Simon Rink
 */

#include "MetricCollector.h"

MetricCollector::MetricCollector() {}

MetricCollector::~MetricCollector() {
    // TODO Auto-generated destructor stub
}

void MetricCollector::initialize(int numOfNodes) {
    for (int i = 0; i < numOfNodes; i++) {
        Metrics nodeMetric = Metrics();
        nodeMetric.eid_ = i + 1;
        this->nodeMetrics_.push_back(nodeMetric);
    }

    this->bundleHops_ = map<long, int>();
    this->bundleElapsedTime_ = map<long, double>();
    this->bundleArrivalTime_ = map<long, ArrivalInfo>();
    this->startWalltime = std::chrono::steady_clock::now();
    this->nodesNumber_ = numOfNodes;
}

void MetricCollector::updateCGRCalls(int eid) {
    this->nodeMetrics_.at(eid - 1).cgrCalls_ = this->nodeMetrics_.at(eid - 1).cgrCalls_ + 1;
}

void MetricCollector::updateRUCoPCalls(int eid) {

    this->nodeMetrics_.at(eid - 1).RUCoPCalls_ = this->nodeMetrics_.at(eid - 1).RUCoPCalls_ + 1;
}

void MetricCollector::updateSentBundles(int eid, int destinationEid, double time, long bundleId) {

    if (this->nodeMetrics_.at(eid - 1).sentBundles_.find(bundleId) ==
        this->nodeMetrics_.at(eid - 1).sentBundles_.end()) {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] = 1;
    } else {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] =
            this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] + 1;
    }

    this->nodeMetrics_.at(eid - 1).routingDecisions_[bundleId].push_back(
        make_tuple(destinationEid, time));
}

void MetricCollector::updateSentBundles(int eid, int destinationEid, double time, long bundleId,
                                        int numBundles) {
    if (this->nodeMetrics_.at(eid - 1).sentBundles_.find(bundleId) ==
        this->nodeMetrics_.at(eid - 1).sentBundles_.end()) {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] = numBundles;
    } else {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] =
            this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] + numBundles;
    }

    this->nodeMetrics_.at(eid - 1).routingDecisions_[bundleId].push_back(
        make_tuple(destinationEid, time));
}
void MetricCollector::updateReceivedBundles(int eid, long bundleId, double receivingTime) {

    this->nodeMetrics_.at(eid - 1).bundleReceivingTimes_[bundleId] = receivingTime;
}

void MetricCollector::updateStartedBundles(int eid, long bundleId, int sourceEid,
                                           int destinationEid, double startTime) {
    if (this->nodeMetrics_.at(eid - 1).bundleStartTimes_.find(bundleId) ==
        this->nodeMetrics_.at(eid - 1).bundleStartTimes_.end()) {
        this->nodeMetrics_.at(eid - 1).bundleStartTimes_[bundleId] = startTime;
    }

    if (this->bundleInformation_.find(bundleId) == this->bundleInformation_.end()) {
        this->bundleInformation_[bundleId] = make_tuple(sourceEid, destinationEid);
    }
}

void MetricCollector::setNumberOfHops(long bundleId, int hops) {
    this->bundleHops_[bundleId] = hops;
}

void MetricCollector::updateBundleElapsedTime(long bundleId, double elapsedTime) {
    auto bundleElapsedTime = &this->bundleElapsedTime_;
    if ((*bundleElapsedTime).find(bundleId) == (*bundleElapsedTime).end())
        (*bundleElapsedTime)[bundleId] = elapsedTime;
    else
        (*bundleElapsedTime)[bundleId] = (*bundleElapsedTime)[bundleId] + elapsedTime;
}

void MetricCollector::intializeArrivalTime(long bundleId, std::chrono::steady_clock::time_point initialTime) {
    auto bundleArrivalTime = &this->bundleArrivalTime_;
    if ((*bundleArrivalTime).find(bundleId) == (*bundleArrivalTime).end()){
        (*bundleArrivalTime)[bundleId].generationTime = initialTime;
    }
    // if already initialized, do nothing because it is not the src node
}

void MetricCollector::setFinalArrivalTime(long bundleId, std::chrono::steady_clock::time_point finalTime) {
    this->bundleArrivalTime_[bundleId].arrivalTime = finalTime;
}

void MetricCollector::updateCGRComputationTime(long computationTime) {
    this->cgrComputationTime_ += computationTime;
}

void MetricCollector::updateRUCoPComputationTime(long computationTime) {
    this->RUCoPComputationTime_ += computationTime;
}

void MetricCollector::setAlgorithm(string algorithm) {
    this->algorithm_ = algorithm;
}

void MetricCollector::setFailureProb(double failureProb) {
    this->failureProb_ = failureProb;
}

void MetricCollector::setMode(int mode) {
    this->mode = mode;
}

void MetricCollector::setPath(string path) {
    this->path_ = path;
}

int MetricCollector::getMode() {
    return this->mode;
}

string MetricCollector::getPrefix() {
    string result = this->path_ + "/" + this->algorithm_;
    if (this->failureProb_ == -1) {
        result += "/pf=-1";
    } else if (this->failureProb_ == 20) {
        result += "/pf=0.2";
    } else if (this->failureProb_ == 35) {
        result += "/pf=0.35";
    } else if (this->failureProb_ == 50) {
        result += "/pf=0.5";
    } else if (this->failureProb_ == 65) {
        result += "/pf=0.65";
    } else if (this->failureProb_ == 80) {
        result += "/pf=0.8";
    } else if (this->failureProb_ == 0) {
        result += "/pf=-1";
    }

    if (this->mode == 0) {
        result += "/no_opp";
    } else if (this->mode == 2) {
        result += "/opp_known";
    }

    return result;
}

/**
 * All results from the node metrics are evaluated and printed into the .txt and .json files
 *
 * @author Simon Rink
 */
void MetricCollector::evaluateAndPrintResults() {
    // Collect necessary data
    map<long, double> bundlesToBeSent = this->getOverallSentBundles();
    map<long, double> receivedBundles = this->getOverallReceivedBundles();

    map<long, double> bundleDeliveryTimes =
        this->computeDeliveryTimes(bundlesToBeSent, receivedBundles);
    map<long, int> bundlesDeliveryCounts = this->getBundleDeliveryCounts();
    int RUCoPCalls = this->getRUCoPCalls();
    int cgrCalls = this->getCGRCalls();

    // regular .txt file
    string prefix = this->getPrefix();
    int number = this->getFileNumber(prefix);
    std::filesystem::create_directories(prefix + "/metrics");

    ofstream outputFile(prefix + "/metrics/output_" + to_string(number) + ".txt");
    outputFile << "The bundles with the following ids have been sent: " << endl;
    string sentIds = "";
    for (auto it = bundlesToBeSent.begin(); it != bundlesToBeSent.end(); it++) {
        sentIds += to_string(it->first) + "(" +
                   to_string(get<0>(this->bundleInformation_[it->first])) + " to " +
                   to_string(get<1>(this->bundleInformation_[it->first])) + "), ";
    }
    outputFile << sentIds << endl;
    string seperator = "";
    for (int i = 0; i < 20; i++) {
        seperator += "-";
    }
    outputFile << seperator << endl;
    outputFile << "The following bundles were received by the destination node with the following "
                  "delivery time: "
               << endl;
    string receivedIds = "";
    for (auto it = bundleDeliveryTimes.begin(); it != bundleDeliveryTimes.end(); it++) {
        receivedIds += "(" + to_string(it->first) + ": " + to_string(it->second) + ")" + ", ";
    }
    outputFile << receivedIds << endl;

    outputFile << seperator << endl;
    outputFile << "That means, that overall "
               << (double)bundleDeliveryTimes.size() / bundlesToBeSent.size() << " ("
               << bundleDeliveryTimes.size() << "/" << bundlesToBeSent.size()
               << ") were delivered successfully" << endl;
    outputFile << seperator << endl;

    outputFile << "Further, every bundle was sent the following amount of times: " << endl;
    string deliveryCounts = "";

    for (auto it = bundlesDeliveryCounts.begin(); it != bundlesDeliveryCounts.end(); it++) {
        deliveryCounts += "(" + to_string(it->first) + ": " + to_string(it->second) + ")" + ", ";
    }
    outputFile << deliveryCounts << endl;
    outputFile << seperator << endl;
    outputFile << "Overall, the computation required " << RUCoPCalls
               << " calls to RUCoP, which took " << to_string(this->RUCoPComputationTime_)
               << " seconds and " << cgrCalls << " calls to CGR, which took "
               << to_string(this->cgrComputationTime_) << " seconds" << endl;
    outputFile << seperator << endl;
    outputFile << "To achieve this results, the following routing decisions have been taken: "
               << endl;
    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        map<long, vector<tuple<int, double>>> routingDecisions =
            this->nodeMetrics_.at(i).routingDecisions_;
        outputFile << "Node " << i + 1 << ": " << endl;
        for (auto it = routingDecisions.begin(); it != routingDecisions.end(); it++) {
            string decisionsString = "";
            for (size_t j = 0; j < it->second.size(); j++) {
                tuple<int, double> decision = it->second.at(j);
                decisionsString += "(to: " + to_string(get<0>(decision)) +
                                   ", time: " + to_string(get<1>(decision)) + "), ";
            }
            outputFile << "Bundle " << it->first << ": " << decisionsString << endl;
        }
    }

    outputFile << seperator << endl;
    outputFile.close();

    // json file
    json j;
    vector<string> bundleIds;

    for (auto it = this->bundleInformation_.begin(); it != this->bundleInformation_.end(); it++) {
        bundleIds.push_back(this->getInformationString(it->first, bundlesToBeSent[it->first]));
    }

    j["bundleIds"] = bundleIds;
    vector<string> receivedBundleIds;

    for (auto it = receivedBundles.begin(); it != receivedBundles.end(); it++) {
        receivedBundleIds.push_back(
            this->getInformationString(it->first, bundlesToBeSent[it->first]));
    }

    j["receivedIds"] = receivedBundleIds;

    for (auto it = bundleDeliveryTimes.begin(); it != bundleDeliveryTimes.end(); it++) {
        j["bundleDeliveryTimes"]
         [this->getInformationString(it->first, bundlesToBeSent[it->first])] = it->second;
    }

    for (auto it = bundlesDeliveryCounts.begin(); it != bundlesDeliveryCounts.end(); it++) {
        j["bundleDeliveryCounts"]
         [this->getInformationString(it->first, bundlesToBeSent[it->first])] = it->second;
    }

    j["cgrCalls"] = cgrCalls;
    j["RUCoPCalls"] = RUCoPCalls;
    j["cgrComputationTime"] = this->cgrComputationTime_;
    j["RUCoPComputationTime"] = this->RUCoPComputationTime_;

    ofstream jsonFile(prefix + "/metrics/jsonResults_" + to_string(number) + ".txt");
    jsonFile << setw(4) << j << endl;

    jsonFile.close();

    cout << "MetricCollector: Results written to " << prefix + "/metrics/" << endl;
}


void buildBundleMetrics(std::map<long, int> &bundleHops,
                        std::map<long, double> &bundleElapseTime,
                        std::map<long, ArrivalInfo> &bundleArrivalTime,
                        int &avgNumberOfHops, double &avgElapsedTime, double &avgArrivalTime,
                        nlohmann::json &bundleMetrics) {
    for (auto it = bundleHops.begin(); it != bundleHops.end(); it++) {
        long bundleId = it->first;
        int hops = it->second;
        double elapsedTime = bundleElapseTime[bundleId];
        avgNumberOfHops += hops;
        avgElapsedTime += elapsedTime;

        json bundleMetric;
        bundleMetric["id"] = bundleId;
        bundleMetric["numberOfHops"] = hops;
        bundleMetric["elapsedTime"] = elapsedTime;

        if (bundleArrivalTime[bundleId].arrivalTime == std::chrono::steady_clock::time_point()) {
            bundleMetrics.push_back(bundleMetric);
            continue; // skip bundles that were not received
        }
        double arrivalTimeSecs = std::chrono::duration<double>(
            (bundleArrivalTime[bundleId].arrivalTime) - (bundleArrivalTime[bundleId].generationTime)).count();
        avgArrivalTime += arrivalTimeSecs;
        bundleMetric["arrivalTime"] = arrivalTimeSecs;

        bundleMetrics.push_back(bundleMetric);
    }
}

/*
 * All results from the node metrics are evaluated and printed into a .json file
 */
void MetricCollector::evaluateAndPrintJsonResults() {
    string prefix = this->getPrefix();
    std::filesystem::create_directories(prefix + "/metrics");

    auto endWalltime = std::chrono::steady_clock::now();
    auto simTime = std::chrono::duration_cast<std::chrono::seconds>(endWalltime - this->startWalltime).count();

    json j;

    auto bundleMetrics = json::array();
    auto avgNumberOfHops = 0;
    auto avgElapsedTime = 0.0;
    auto avgArrivalTime = 0.0;
    buildBundleMetrics(
        this->bundleHops_,
        this->bundleElapsedTime_,
        this->bundleArrivalTime_,
        avgNumberOfHops, avgElapsedTime, avgArrivalTime,
        bundleMetrics
    );

    if(!this->bundleHops_.empty()){
        auto nBundles = this->bundleHops_.size();
        avgNumberOfHops = avgNumberOfHops / nBundles;
        avgElapsedTime = avgElapsedTime / nBundles;
        avgArrivalTime = avgArrivalTime / nBundles;
    }

    j["bundles"] = bundleMetrics;
    j["avgElapsedTime"] = avgElapsedTime; 
    j["avgNumberOfHops"] = avgNumberOfHops;
    if (avgArrivalTime > 0) // skip if no arrivals
        j["avgArrivalTime"] = avgArrivalTime;
    j["simulationWalltimeSeconds"] = simTime;

    ofstream jsonFile(prefix + "/metrics/results_" + to_string(nodesNumber_) + ".json");
    jsonFile << setw(4) << j << endl;
    jsonFile.close();

    cout << "MetricCollector: Results written to " << prefix + "/metrics/" << endl;
}

/*
 * Determines the bundles that started in the network
 *
 * @return A Map that contains for each bundle ID the corresponding start time
 *
 * @author Simon Rink
 */
map<long, double> MetricCollector::getOverallSentBundles() {
    map<long, double> bundleMap;
    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        Metrics nodeMetric = this->nodeMetrics_.at(i);

        for (auto it = nodeMetric.bundleStartTimes_.begin();
             it != nodeMetric.bundleStartTimes_.end(); it++) {
            bundleMap[it->first] = it->second;
        }
    }

    return bundleMap;
}

/*
 * Determines the bundles that were succesfully received at their destination
 *
 * @return A Map contains for each bundle the corresponding when it was received
 *
 * @author Simon Rink
 */
map<long, double> MetricCollector::getOverallReceivedBundles() {
    map<long, double> bundleMap;
    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        Metrics nodeMetric = this->nodeMetrics_.at(i);

        for (auto it = nodeMetric.bundleReceivingTimes_.begin();
             it != nodeMetric.bundleReceivingTimes_.end(); it++) {
            bundleMap[it->first] = it->second;
        }
    }

    return bundleMap;
}

/**
 * Returns a unique string for each bundle
 *
 * @param bundleid: The ID of the bundle
 * 	      start: The start time of the bundle
 *
 * @return The resulting string
 *
 * @author Simon Rink
 */
string MetricCollector::getInformationString(long bundleId, double start) {
    tuple<int, int> informations = this->bundleInformation_[bundleId];
    return to_string(get<0>(informations)) + ":" + to_string(get<1>(informations)) + ":" +
           to_string(start);
}

/*
 * Computes the delivery delay for each bundle
 *
 * @param startTimes: The start times for each bundle
 * 		  receivingTimes: The receiving times for each bundle
 *
 * @return The Map that contains for each bundle the corresponding delivery delay
 *
 * @author Simon Rink
 */
map<long, double> MetricCollector::computeDeliveryTimes(map<long, double> startTimes,
                                                        map<long, double> receivingTimes) {
    map<long, double> bundleMap;

    for (auto it = receivingTimes.begin(); it != receivingTimes.end(); it++) {
        bundleMap[it->first] =
            it->second - startTimes[it->first]; // all fields must exist, since a received bundle
                                                // must have been sent as some point
    }

    return bundleMap;
}

/*
 * Computes the overall sent copies for each bundle
 *
 * @return A Map that contains for each bundle the corresponding amount of sent bundles
 *
 * @author Simon Rink
 */
map<long, int> MetricCollector::getBundleDeliveryCounts() {
    map<long, int> bundleMap;

    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        Metrics nodeMetric = this->nodeMetrics_.at(i);

        for (auto it = nodeMetric.sentBundles_.begin(); it != nodeMetric.sentBundles_.end(); it++) {
            if (bundleMap.find(it->first) == bundleMap.end()) {
                bundleMap[it->first] = it->second;
            } else {
                bundleMap[it->first] = bundleMap[it->first] + it->second;
            }
        }
    }

    return bundleMap;
}

/**
 * Returns the calls to the RUCoP algorithm in the whole simulation
 *
 * @return The number of RUCoP calls
 *
 * @author Simon Rink
 */
int MetricCollector::getRUCoPCalls() {
    int RUCoPCalls = 0;
    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        Metrics nodeMetric = this->nodeMetrics_.at(i);

        RUCoPCalls = RUCoPCalls + nodeMetric.RUCoPCalls_;
    }

    return RUCoPCalls;
}

/**
 * Returns the calls to CGR in the whole simulation
 *
 * @return The number of CGR calls
 *
 * @author Simon Rink
 */
int MetricCollector::getCGRCalls() {
    int djikstraCalls = 0;
    for (size_t i = 0; i < this->nodeMetrics_.size(); i++) {
        Metrics nodeMetric = this->nodeMetrics_.at(i);

        djikstraCalls = djikstraCalls + nodeMetric.cgrCalls_;
    }

    return djikstraCalls;
}