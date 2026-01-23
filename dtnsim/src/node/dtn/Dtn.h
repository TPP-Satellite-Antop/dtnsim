#ifndef _DTN_H_
#define _DTN_H_

#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/routing/Routing.h"
#include "src/utils/MetricCollector.h"
#include "src/node/graphics/Graphics.h"
#include "src/node/dtn/SdrModel.h"

using namespace omnetpp;
using namespace std;

class Dtn : public cSimpleModule, public Observer {
  public:
    Dtn();
    virtual ~Dtn();
    Routing *getRouting();
    virtual void setOnFault(bool onFault) = 0;
    void setMetricCollector(MetricCollector *metricCollector);
    void update();

  protected:
    virtual void initialize(int stage) override = 0;
    int numInitStages() const;
    virtual void handleMessage(cMessage *msg) override = 0;
    virtual void finish() override = 0;

    Graphics *graphicsModule; 
    Routing *routing;
    MetricCollector *metricCollector_;
    SdrModel *sdr_;
    bool onFault = false;
};

#endif /* _DTN_H_ */