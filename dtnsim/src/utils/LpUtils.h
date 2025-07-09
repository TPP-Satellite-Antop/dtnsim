#include "../Config.h"

#ifdef USE_CPLEX_LIBRARY
#ifdef USE_BOOST_LIBRARIES

#ifndef LPUTILS_H_
#define LPUTILS_H_

#include "ContactPlan.h"
#include "Lp.h"
#include "utils/RouterGraphInfo.h"
#include "utils/TopologyGraphInfo.h"
#include <boost/graph/adjacency_list.hpp>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

namespace lpUtils {

/// @brief Compute Flows of traffic from the Lp model.
/// @return map that associate one RouterGraph per state
map<double, RouterGraph> computeFlows(ContactPlan *contactPlan, int nodesNumber, Lp *lp);

/// @brief Gets delivery time of the last delivered traffic
double getMaxDeliveryTime(Lp *lp, int solutionNumber);

/// @brief Gets total transmissions in bytes
double getTotalTxBytes(Lp *lp, int solutionNumber);

} /* namespace lpUtils */

#endif /* LPUTILS_H_ */
#endif /* USE_BOOST_LIBRARIES */
#endif /* USE_CPLEX_LIBRARY */
