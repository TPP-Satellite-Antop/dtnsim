#include "LpUtils.h"

#ifdef USE_CPLEX_LIBRARY
#ifdef USE_BOOST_LIBRARIES

using namespace std;

namespace lpUtils {

map<double, RouterGraph> computeFlows(ContactPlan *contactPlan, int nodesNumber, Lp *lp) {
    map<double, RouterGraph> flows;
    vector<RouterGraph> lpFlows = lp->getSolvedStates();

    vector<pair<int, int>> intervals = lp->getIntervals();

    assert(lpFlows.size() == intervals.size());

    int k = 0;
    vector<pair<int, int>>::iterator it1 = intervals.begin();
    vector<pair<int, int>>::iterator it2 = intervals.end();
    for (; it1 != it2; ++it1) {
        double stateStart = it1->first;
        flows[stateStart] = lpFlows.at(k);
        k++;
    }

    return flows;
}

double getMaxDeliveryTime(Lp *lp, int solutionNumber) {
    map<int, double> usedContacts = lp->getUsedContacts(solutionNumber);

    double maxDeliveryTime = 0.0;

    map<int, double>::const_iterator it = usedContacts.begin();
    for (; it != usedContacts.end(); ++it) {
        int contactId = it->first;
        double traffic = it->second;

        if (traffic != 0) {
            Contact contact = *(lp->getContactPlan()->getContactById(contactId));
            double start = contact.getStart();
            double byteRate = contact.getDataRate();
            double txTime = (traffic) / (byteRate);
            double endTime = start + txTime;

            if (endTime > maxDeliveryTime) {
                maxDeliveryTime = endTime;
            }
        }
    }

    return maxDeliveryTime;
}

double getTotalTxBytes(Lp *lp, int solutionNumber) {
    map<int, double> usedContacts = lp->getUsedContacts(solutionNumber);

    double totalTxBytes = 0.0;

    map<int, double>::const_iterator it = usedContacts.begin();
    for (; it != usedContacts.end(); ++it) {
        double traffic = it->second;
        totalTxBytes += traffic;
    }

    return totalTxBytes;
}

} // namespace lpUtils

#endif /* USE_BOOST_LIBRARIES */
#endif /* USE_CPLEX_LIBRARY */
