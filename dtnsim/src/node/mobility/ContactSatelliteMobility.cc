#include <ctime>
#include <cmath>

#include "ContactSatelliteMobility.h"
#include "INorad.h"
#include "NoradA.h"
#include "h3api.h"

namespace inet {

Define_Module(ContactSatelliteMobility);

ContactSatelliteMobility::ContactSatelliteMobility()
{
   noradModule = nullptr;
   mapX = 0;
   mapY = 0;
   transmitPower = 0.0;
}

void ContactSatelliteMobility::setTargetPosition()
{
    noradModule->updateTime(simTime());
    lastPosition.x = mapX * noradModule->getLongitude() / 360 + (mapX / 2);
    lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapX);
    lastPosition.y = ((-mapY * noradModule->getLatitude()) / 180) + (mapY / 2);
    targetPosition.x = lastPosition.x;
    targetPosition.y = lastPosition.y;
    nextChange =  simTime() + updateInterval;

    const auto latLng = LatLng {deg2rad(getLatitude()), deg2rad(getLongitude())};

    H3Index cell = 0;
    if (latLngToCell(&latLng, 0, &cell) != E_SUCCESS) {
        std::cout << "Error converting lat long to cell" << std::endl;
        return;
    }

    std::cout << "Cell: " << cell << std::endl;
}
} // namespace inet
