/*
 * MetricCollector.cc
 *
 *  Created on: Feb 1, 2022
 *      Author: Simon Rink
 */

#include "MetricCollector.h"
#include <algorithm>
#include <fstream>
#include <omnetpp/csimulation.h>
#include <omnetpp/index.h>
#include <omnetpp/simtime.h>

MetricCollector::MetricCollector() {}

MetricCollector::~MetricCollector() {
    // TODO Auto-generated destructor stub
}

void MetricCollector::initialize(const int numOfNodes) {
    for (int i = 0; i < numOfNodes; i++) {
        auto nodeMetric = Metrics();
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

    this->nodeMetrics_.at(eid - 1).routingDecisions_[bundleId].emplace_back(destinationEid, time);
}

void MetricCollector::updateSentBundles(const int eid, int destinationEid, double time, const long bundleId, const int numBundles) {
    if (!this->nodeMetrics_.at(eid - 1).sentBundles_.contains(bundleId)) {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] = numBundles;
    } else {
        this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] =
            this->nodeMetrics_.at(eid - 1).sentBundles_[bundleId] + numBundles;
    }

    this->nodeMetrics_.at(eid - 1).routingDecisions_[bundleId].emplace_back(destinationEid, time);
}
void MetricCollector::updateReceivedBundles(const int eid, const long bundleId, const double receivingTime) {
    this->nodeMetrics_.at(eid - 1).bundleReceivingTimes_[bundleId] = receivingTime;
}

void MetricCollector::updateStartedBundles(const int eid, const long bundleId, int sourceEid, int destinationEid, double startTime) {
    if (!this->nodeMetrics_.at(eid - 1).bundleStartTimes_.contains(bundleId)) {
        this->nodeMetrics_.at(eid - 1).bundleStartTimes_[bundleId] = startTime;
    }

    if (this->bundleInformation_.find(bundleId) == this->bundleInformation_.end()) {
        this->bundleInformation_[bundleId] = make_tuple(sourceEid, destinationEid);
    }
}

void MetricCollector::setNumberOfHops(const long bundleId, const int hops) {
    this->bundleHops_[bundleId] = hops;
}

/*
* Updates the elapsed time for a bundle.
* If the bundle is not yet in the map, it initializes it with the given elapsed time.
* Otherwise, calculates the new elapsed time and adds it to the existing one.
*/
void MetricCollector::updateBundleElapsedTime(
    const long bundleId, std::chrono::steady_clock::time_point elapsedTimeStart) {
    double elapsedTime = std::chrono::duration<double>(std::chrono::steady_clock::now() - elapsedTimeStart).count();

    auto bundleElapsedTime = &this->bundleElapsedTime_;
    if ((*bundleElapsedTime).find(bundleId) == (*bundleElapsedTime).end())
        (*bundleElapsedTime)[bundleId] = elapsedTime;
    else
        (*bundleElapsedTime)[bundleId] = (*bundleElapsedTime)[bundleId] + elapsedTime;
}

/*
* Initializes the arrival time for a bundle.
* If the bundle is not yet in the map, it sets the generation time to the given initial time.
* If the bundle is already in the map, it does nothing (because it's not the node that generated the bundle).
*/
void MetricCollector::intializeArrivalTime(const long bundleId, const omnetpp::SimTime& initialTime) {
    if (const auto bundleArrivalTime = &this->bundleArrivalTime_; !bundleArrivalTime->contains(bundleId)){
        (*bundleArrivalTime)[bundleId].generationTime = initialTime;
    }
}

/*
* Sets the final arrival time for a bundle.
*/
void MetricCollector::setFinalArrivalTime(const long bundleId, const omnetpp::SimTime& finalTime) {
    this->bundleArrivalTime_[bundleId].arrivalTime = finalTime;
}

void MetricCollector::updateCGRComputationTime(const long computationTime) {
    this->cgrComputationTime_ += computationTime;
}

void MetricCollector::updateRUCoPComputationTime(const long computationTime) {
    this->RUCoPComputationTime_ += computationTime;
}

void MetricCollector::setAlgorithm(const string& algorithm) {
    this->algorithm_ = algorithm;
}

void MetricCollector::setFailureProb(const double failureProb) {
    this->failureProb_ = failureProb;
}

void MetricCollector::setMode(const int newMode) {
    this->mode = newMode;
}

void MetricCollector::setPath(const string& path) {
    this->path_ = path;
}

int MetricCollector::getMode() const {
    return this->mode;
}

string MetricCollector::getPrefix() const {
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

    map<long, double> bundleDeliveryTimes = computeDeliveryTimes(bundlesToBeSent, receivedBundles);
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
    string seperator;
    for (int i = 0; i < 20; i++) {
        seperator += "-";
    }
    outputFile << seperator << endl;
    outputFile << "The following bundles were received by the destination node with the following "
                  "delivery time: "
               << endl;
    string receivedIds;
    for (auto it = bundleDeliveryTimes.begin(); it != bundleDeliveryTimes.end(); it++) {
        receivedIds += "(" + to_string(it->first) + ": " + to_string(it->second) + ")" + ", ";
    }
    outputFile << receivedIds << endl;

    outputFile << seperator << endl;
    outputFile << "That means, that overall "
               << static_cast<double>(bundleDeliveryTimes.size()) / static_cast<double>(bundlesToBeSent.size()) << " ("
               << bundleDeliveryTimes.size() << "/" << bundlesToBeSent.size()
               << ") were delivered successfully" << endl;
    outputFile << seperator << endl;

    outputFile << "Further, every bundle was sent the following amount of times: " << endl;
    string deliveryCounts;

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
            string decisionsString;
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

    for (auto &[bundleId, startTime] : receivedBundles) {
        receivedBundleIds.push_back(this->getInformationString(bundleId, bundlesToBeSent[bundleId]));
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


void buildBundleMetrics(
    std::map<long, int> &bundleHops,
    std::map<long, double> &bundleElapseTime,
    std::map<long, ArrivalInfo> &bundleArrivalTime,
    int &avgNumberOfHops, double &avgElapsedTime, double &avgArrivalTime,
    nlohmann::json &bundleMetrics
){
    for (auto &[bundleId, hops] : bundleHops) {
        double elapsedTime = bundleElapseTime[bundleId];
        avgNumberOfHops += hops;
        avgElapsedTime += elapsedTime;

        json bundleMetric = json::object();
        bundleMetric["id"] = bundleId;
        bundleMetric["numberOfHops"] = hops;
        bundleMetric["elapsedTime"] = elapsedTime;

        if (bundleArrivalTime[bundleId].arrivalTime != -1) {
            const auto arrivalTimeRaw = bundleArrivalTime[bundleId].arrivalTime - bundleArrivalTime[bundleId].generationTime;
            const auto arrivalTime = arrivalTimeRaw.str();

            avgArrivalTime += stod(arrivalTime);
            bundleMetric["arrivalTime"] = stod(arrivalTime);
        }

        bundleMetrics.push_back(bundleMetric);
    }
}

// Build timestamp string: YYYYMMDD-HHMMSS
std::string makeTimestamp() {
    const auto now =  std::chrono::system_clock::now();
    const std::time_t tt =  std::chrono::system_clock::to_time_t(now);

    std::tm localTm{};
    localtime_r(&tt, &localTm); // thread-safe (POSIX)

    std::ostringstream oss;
    oss << std::put_time(&localTm, "%Y%m%d-%H%M%S");
    return oss.str();
}

/*
 * All results from the node metrics are evaluated and printed into a .json file
 */
void MetricCollector::evaluateAndPrintJsonResults() {
    auto endWalltime = std::chrono::steady_clock::now();
    auto simTime = std::chrono::duration_cast<std::chrono::seconds>(endWalltime - this->startWalltime).count();

    json j = json::object();

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

    std::filesystem::path p = this->path_;
    std::filesystem::create_directories(p.parent_path());

    // use this to add timestamp to the file name, if needed:
    // std::string timestamp = makeTimestamp();
    // std::filesystem::path timestampedPath =
    //     p.parent_path() /
    //     (p.stem().string() + "-" + timestamp + p.extension().string());

    ofstream jsonFile(p);
    jsonFile << setw(4) << j << endl;
    jsonFile.close();

    cout << "MetricCollector: Results written to " << p << endl;
}

/*
 * Determines the bundles that started in the network
 *
 * @return A Map that contains for each bundle ID the corresponding start time
 *
 * @author Simon Rink
 */
map<long, double> MetricCollector::getOverallSentBundles() const {
    map<long, double> bundleMap;
    for (const auto& nodeMetric : this->nodeMetrics_) {
        for (const auto &[bundleId, time] : nodeMetric.bundleStartTimes_) {
            bundleMap[bundleId] = time;
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
map<long, double> MetricCollector::getOverallReceivedBundles() const {
    map<long, double> bundleMap;
    for (const auto& nodeMetric : this->nodeMetrics_) {
        for (const auto &[bundleId, time] : nodeMetric.bundleReceivingTimes_) {
            bundleMap[bundleId] = time;
        }
    }

    return bundleMap;
}

/**
 * Returns a unique string for each bundle
 *
 * @param bundleId: The ID of the bundle
 * @param start: The start time of the bundle
 *
 * @return The resulting string
 *
 * @author Simon Rink
 */
string MetricCollector::getInformationString(const long bundleId, const double start) {
    tuple<int, int> informations = this->bundleInformation_[bundleId];
    return to_string(get<0>(informations)) + ":" + to_string(get<1>(informations)) + ":" + to_string(start);
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
map<long, double> MetricCollector::computeDeliveryTimes(map<long, double> startTimes, const map<long, double>& receivingTimes) {
    map<long, double> bundleMap;

    for (const auto &[bundleId, time] : receivingTimes) {
        bundleMap[bundleId] = time - startTimes[bundleId];
        // all fields must exist, since a received bundle must have been sent as some point
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
map<long, int> MetricCollector::getBundleDeliveryCounts() const {
    map<long, int> bundleMap;

    for (const auto& nodeMetric : this->nodeMetrics_) {
        for (const auto &[bundleId, counter] : nodeMetric.sentBundles_) {
            if (!bundleMap.contains(bundleId)) {
                bundleMap[bundleId] = counter;
            } else {
                bundleMap[bundleId] = bundleMap[bundleId] + counter;
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
int MetricCollector::getRUCoPCalls() const {
    int RUCoPCalls = 0;
    for (const auto& nodeMetric : this->nodeMetrics_) {
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
int MetricCollector::getCGRCalls() const {
    int djikstraCalls = 0;
    for (const auto& nodeMetric : this->nodeMetrics_) {
        djikstraCalls = djikstraCalls + nodeMetric.cgrCalls_;
    }

    return djikstraCalls;
}

int MetricCollector::getFileNumber(const string& prefix) {
    int number = 0;
    string fileName = prefix + "/metrics/output_" + to_string(number) + ".txt";

    while (FILE *test = fopen(fileName.c_str(), "r"))
    {
	    number++;
	    fileName = prefix + "/metrics/output_" + to_string(number) + ".txt";
    }

    return number;
}
