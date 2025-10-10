#ifndef CONTACT_PLAN_COM_H_
#define CONTACT_PLAN_COM_H_


#include <iomanip>
#include <omnetpp.h>
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/node/com/Com.h"

using namespace std;
using namespace omnetpp;

class ContactPlanCom : public Com {
  public:
    ContactPlanCom();
    virtual ~ContactPlanCom();
    void setContactTopology(ContactPlan &contactTopology);

  protected:
    double getLinkDelay(int sourceEid, int nextHopEid) override;

  private:
    ContactPlan contactTopology_;
};

#endif /* CONTACT_PLAN_COM_H_ */
