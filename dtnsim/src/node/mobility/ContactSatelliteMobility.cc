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

    auto latLng = LatLng {deg2rad(getLatitude()), deg2rad(getLongitude())};

	const auto idx = getParentModule()->getSubmodule("norad")->par("satIndex").intValue();
	if (idx == 0) return;

	H3Index cell = 0;
    if (latLngToCell(&latLng, 0, &cell) != E_SUCCESS) {
        std::cout << "Error converting lat lng to cell" << std::endl;
        return;
    }

	std::array<H3Index, 7> neighbors{};
	if (const H3Error err = gridDisk(cell, 1, neighbors.data()); err != E_SUCCESS) {
		std::cout << "Error obtaining cell " << std::hex << cell << " neighbors" << std::endl;
        return;
	}

	const auto nodes = getParentModule()->getParentModule()->par("nodesNumber").intValue();
	for (int i = idx+1; i <= nodes; i++) {
		auto mobility = dynamic_cast<ContactSatelliteMobility*>(getSimulation()->getSystemModule()->getSubmodule("node", i)->getSubmodule("mobility"));

	    latLng = LatLng {deg2rad(mobility->getLatitude()), deg2rad(mobility->getLongitude())};
    	if (latLngToCell(&latLng, 0, &cell) != E_SUCCESS) {
        	std::cout << "Error converting lat lng to cell" << std::endl;
        	continue;
    	}

		if (std::find(neighbors.begin(), neighbors.end(), cell) != neighbors.end()) {
    		std::cout << "Cell " << std::hex << cell << " is in the array" << std::endl;
		}
	}
}
} // namespace inet
