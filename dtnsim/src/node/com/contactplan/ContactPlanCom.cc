#include "ContactPlanCom.h"
#include "src/node/app/App.h"

Define_Module(ContactPlanCom);

void ContactPlanCom::setContactTopology(ContactPlan &contactTopology) {
    this->contactTopology_ = contactTopology;
}

double ContactPlanCom::getLinkDelay(int sourceEid, int nextHopEid) {
    double linkDelay = contactTopology_.getRangeBySrcDst(eid_, nextHopEid);
    if (linkDelay == -1) {
        // cout << "warning, range not available for nodes " << eid_ << "-"
        //         << nextHopEid << ", assuming range is 0" << endl;
        linkDelay = 0;
    }

    return linkDelay;
}

ContactPlanCom::ContactPlanCom() {}

ContactPlanCom::~ContactPlanCom() {}
