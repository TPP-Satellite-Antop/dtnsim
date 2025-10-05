
#ifndef CONTACTPLANUTILS_H_
#define CONTACTPLANUTILS_H_

#include "../node/dtn/contactplan/ContactPlan.h"

using namespace std;

namespace contactPlanUtils {
/// @brief get the contacts of a contact plan which are not present in a contact topology
vector<Contact> getDifferenceContacts(ContactPlan &contactPlan, ContactPlan &contactTopology);

} // namespace contactPlanUtils

#endif /* CONTACTPLANUTILS_H_ */
