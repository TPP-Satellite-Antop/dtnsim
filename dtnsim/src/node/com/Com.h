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

  protected:
    void initialize();
    void handleMessage(cMessage *);
    void finish();
    virtual double getLinkDelay(int sourceEid, int nextHopEid);

  protected:
    int eid_;
    double packetLoss_;
};

#endif /* COM_H_ */
