#ifndef COM_H_
#define COM_H_


#include <iomanip>
#include <omnetpp.h>
#include "src/node/dtn/contactplan/ContactPlan.h"

using namespace std;
using namespace omnetpp;

class Com : public cSimpleModule {
  public:
    Com();
    virtual ~Com();
    virtual void setContactTopology(ContactPlan &contactTopology);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *);
    virtual void finish();

  private:
    int eid_;
    ContactPlan contactTopology_;

    double packetLoss_;
};

#endif /* COM_H_ */
