#ifndef MOBILITYDATA_H_
#define MOBILITYDATA_H_

#include "src/node/mobility/SatelliteMobility.h"

struct MobilityData {
  inet::SatelliteMobility* mobility;
  bool onFault;
};

#endif /* MOBILITYDATA_H_ */