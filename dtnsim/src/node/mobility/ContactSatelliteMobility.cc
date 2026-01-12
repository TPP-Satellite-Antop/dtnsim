#include <ctime>
#include <cmath>
#include <fstream>
#include "ContactSatelliteMobility.h"
#include "INorad.h"
#include "NoradA.h"
#include "h3api.h"

constexpr int RATE = 100;
constexpr int MAX_NEIGHBORS = 7;
constexpr int DISK_DISTANCE = 1;
const auto FILENAME = "contact_plan.txt";

namespace inet {

Define_Module(ContactSatelliteMobility);

ContactSatelliteMobility::ContactSatelliteMobility() {
   norad = nullptr;
   mapX = 0;
   mapY = 0;
   transmitPower = 0.0;
}

void ContactSatelliteMobility::initialize(int stage) {
    if (!initialized) {
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

    std::array<H3Index, MAX_NEIGHBORS> neighbors{};
    if (const H3Error err = gridDisk(cell, DISK_DISTANCE, neighbors.data()); err != E_SUCCESS) {
	std::cout << "Error obtaining cell " << std::hex << cell << " neighbors" << std::endl;
        return;
    }

    for (int i = idx-1; i > 0; i--) {
        const auto mobility = dynamic_cast<ContactSatelliteMobility*>(getSimulation()->getSystemModule()->getSubmodule("node", i)->getSubmodule("mobility"));

	latLng = LatLng {deg2rad(mobility->getLatitude()), deg2rad(mobility->getLongitude())};
        if (latLngToCell(&latLng, 0, &cell) != E_SUCCESS) {
            std::cout << "Error converting lat lng to cell" << std::endl;
            continue;
        }

	if (std::find(neighbors.begin(), neighbors.end(), cell) != neighbors.end())
	    updateContactPlan(contactPlans[i-1], from, to);
    }
}

void ContactSatelliteMobility::updateContactPlan(std::vector<ContactData>& plan, const omnetpp::SimTime from, const omnetpp::SimTime to) {
    if (!plan.empty() && plan.back().to == from)
        plan.back().to = to;
    else
        plan.push_back({from, to});
}

void ContactSatelliteMobility::finish() {
    SatelliteMobility::finish();

    if (idx == 0) return;

    if (idx == 1) {
        std::ofstream out(FILENAME, std::ios::out);
        if (!out.is_open())
            throw cRuntimeError("Failed to create contact_plan.txt");

        out << "m horizon +0" << std::endl;
        out.close();
    }

    std::ofstream out(FILENAME, std::ios::app);
    if (!out.is_open())
        throw cRuntimeError("Failed to open contact_plan.txt for appending");

    for (int i = 0; i < contactPlans.size(); i++) {
	for (auto &[from, to] : contactPlans[i]) {
	    out << "a contact " << from << " " << to << " " << idx << " " << i+1 << " " << RATE << std::endl;
	    out << "a contact " << from << " " << to << " " << i+1 << " " << idx << " " << RATE << std::endl;
	    // ToDo: add range lines in case not having them makes contacts have an infinite range instead of 0.
	}
    }

    out.close();
}
} // namespace inet
