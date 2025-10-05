/*
 * SdrModel.h
 *
 *  Created on: Nov 25, 2016
 *      Author: juanfraire
 */

#ifndef SRC_NODE_DTN_CONTACT_SDR_MODEL_H_
#define SRC_NODE_DTN_CONTACT_SDR_MODEL_H_

#include <map>
#include <omnetpp.h>
#include "src/dtnsim_m.h"
#include "src/node/dtn/contactplan/ContactPlan.h"
#include "src/dtn/node/dtn/SdrModel.h"
#include <src/node/dtn/SdrStatus.h>
#include "src/utils/Subject.h"

using namespace omnetpp;
using namespace std;

class ContactSdrModel : public SdrModel {
  public:
    ContactSdrModel();
    virtual ~ContactSdrModel();

    // Initialization and configuration
    virtual void setEid(int eid);
    virtual void setNodesNumber(int nodesNumber);
    virtual void setContactPlan(ContactPlan *contactPlan);
    virtual void setSize(int size);
    virtual void freeSdr(int eid);

  private:
    ContactPlan *contactPlan_;
};

#endif /* SRC_NODE_DTN_CONTACT_SDR_MODEL_H_ */
