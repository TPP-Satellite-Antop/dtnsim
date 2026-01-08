#include <ctime>
#include <cmath>

#include "ContactSatelliteMobility.h"
#include "INorad.h"
#include "NoradA.h"
#include "h3api.h"

namespace inet {

Define_Module(ContactSatelliteMobility);

ContactSatelliteMobility::ContactSatelliteMobility() {
   noradModule = nullptr;
   mapX = 0;
   mapY = 0;
   transmitPower = 0.0;
}

void ContactSatelliteMobility::initialize(int stage) {
    if (!initilised) {
	nodes = getParentModule()->getParentModule()->par("nodesNumber").intValue();
	idx = getParentModule()->getSubmodule("norad")->par("satIndex").intValue();
	contactPlans.resize(nodes);
    }

    SatelliteMobility::initialize(stage);
}

void ContactSatelliteMobility::setTargetPosition() {
    SatelliteMobility::setTargetPosition();

    if (idx == 0) return;

    const auto to = nextChange;
    const auto from = nextChange - updateInterval;
    auto latLng = LatLng {deg2rad(getLatitude()), deg2rad(getLongitude())};

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

    for (int i = idx-1; i > 0; i--) {
	auto mobility = dynamic_cast<ContactSatelliteMobility*>(getSimulation()->getSystemModule()->getSubmodule("node", i)->getSubmodule("mobility"));

	latLng = LatLng {deg2rad(mobility->getLatitude()), deg2rad(mobility->getLongitude())};
        if (latLngToCell(&latLng, 0, &cell) != E_SUCCESS) {
            std::cout << "Error converting lat lng to cell" << std::endl;
            continue;
        }

	if (std::find(neighbors.begin(), neighbors.end(), cell) != neighbors.end()) {
	    updateContactPlan(contactPlans[i-1], from, to);
	}
    }
}

void ContactSatelliteMobility::updateContactPlan(std::vector<ContactData>& plan, const omnetpp::SimTime from, const omnetpp::SimTime to) {
    if (!plan.empty() && plan.back().to == from) {
        plan.back().to = to;
    } else {
        plan.push_back({from, to});
    }
}

void ContactSatelliteMobility::finish() {
    SatelliteMobility::finish();

    if (idx == 0) return;

    for (int i = 0; i < contactPlans.size(); i++) {
	for (auto& contact : contactPlans[i]) {
	    std::cout << idx << "<->" << i+1 << ": " << contact.from << " -> " << contact.to << std::endl;
	}
    }
    std::cout << std::endl;
}
} // namespace inet
