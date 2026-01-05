#ifndef MOBILITY_CONTACTSATELLITEMOBILITY_H_
#define MOBILITY_CONTACTSATELLITEMOBILITY_H_

#include "inet/common/INETDefs.h"
#include "INorad.h"
#include "SatelliteMobility.h"

//-----------------------------------------------------
// Class: ContactSatelliteMobility
//
// Performs exactly the same physical satellite movement as SatelliteMobility (SGP4-based
// propagation via INorad), with the added feature to discretize each satellite position
// into an H3 cell at every mobility update.
//
// The resulting (time, satellite, H3 cell) information is meant to be used to build a contact
// plan that is independent of continuous orbital motion simulations.
//-----------------------------------------------------
namespace inet {
class ContactSatelliteMobility : public SatelliteMobility
{
public:
    ContactSatelliteMobility();

protected:
    // sets the position of satellite
    // - sets the target position for the satellite
    // - the position is fetched from the Norad module with reference to the current timestamp
    virtual void setTargetPosition() override;
};
}// namespace inet
#endif
