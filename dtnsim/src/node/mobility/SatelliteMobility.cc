#include <ctime>
#include <cmath>

#include "SatelliteMobility.h"
#include "INorad.h"
#include "NoradA.h"
#include "h3api.h"

namespace inet {

Define_Module(SatelliteMobility);

SatelliteMobility::SatelliteMobility()
{
   norad = nullptr;
   mapX = 0;
   mapY = 0;
   transmitPower = 0.0;
}

void SatelliteMobility::initialize(int stage)
{
    // norad must be initialized before LineSegmentsMobilityBase calling setTargetPosition() in its initialization at stage 1
    //simTime()
    initialized = false;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        norad->initializeMobility(0);
        //norad->initializeMobility(nextChange);
    }
    LineSegmentsMobilityBase::initialize(stage);
    norad = check_and_cast< INorad* >(getParentModule()->getSubmodule("norad"));
    if (norad == nullptr) {
        error("Error in SatSGP4Mobility::initializeMobility(): Cannot find module Norad.");
    }

    //std::time_t timestamp = std::time(nullptr);       // get current time as an integral value holding the num of secs
    std::time_t timestamp =  1619119189;  //8:20PM 22/04/2021                                             // since 00:00, Jan 1 1970 UTC
    std::tm* currentTime = std::gmtime(&timestamp);   // convert timestamp into structure holding a calendar date and time
    norad->setJulian(currentTime);

    mapX = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 0));
    mapY = std::atoi(getParentModule()->getParentModule()->getDisplayString().getTagArg("bgb", 1));

    WATCH(lastPosition);
    refreshDisplay();
}

void SatelliteMobility::initializePosition()
{
    //nextChange = simTime();
    LineSegmentsMobilityBase::initializePosition();
    //nextChange = simTime(); //change this! TODO
    //LineSegmentsMobilityBase::initializePosition();
    //lastUpdate = simTime();
    //scheduleUpdate();
}

void SatelliteMobility::setInitialPosition()
{
    lastPosition.x = mapX * norad->getLongitude() / 360 + (mapX / 2);
    lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapX);
    lastPosition.y = ((-mapY * norad->getLatitude()) / 180) + (mapY / 2);
}

bool SatelliteMobility::isOnSameOrbitalPlane(double raan2, double inclination2)
{
    if(NoradA *noradAModule = dynamic_cast<NoradA*>(norad)){
        double raan = noradAModule->getRaan();
        double inclination = noradAModule->getInclination();
        if((inclination == inclination2) && (raan == raan2))
        {
            return true;
        }
    }
    return false;
}

double SatelliteMobility::getAltitude() const
{
    return norad->getAltitude();
}

double SatelliteMobility::getElevation(const double& refLatitude, const double& refLongitude,
                                     const double& refAltitude) const
{
    return norad->getElevation(refLatitude, refLongitude, refAltitude);
}

double SatelliteMobility::getAzimuth(const double& refLatitude, const double& refLongitude,
                                   const double& refAltitude) const
{
    return norad->getAzimuth(refLatitude, refLongitude, refAltitude);
}

double SatelliteMobility::getDistance(const double& refLatitude, const double& refLongitude,
                                    const double& refAltitude) const
{
    return norad->getDistance(refLatitude, refLongitude, refAltitude);
}

bool SatelliteMobility::isReachable(const double& refLatitude, const double& refLongitude, const double& refAltitude) const
{
    return norad->isReachable(refLatitude, refLongitude, refAltitude);
}

double SatelliteMobility::getLongitude() const
{
    return norad->getLongitude();
}

double SatelliteMobility::getLatitude() const
{
    return norad->getLatitude();
}

void SatelliteMobility::setTargetPosition()
{
    //nextChange += updateInterval.dbl();
    norad->updateTime(simTime());
//    lastPosition.x = mapX * norad->getLongitude() / 360 + (mapX / 2);
//    lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapX);
//    lastPosition.y = ((-mapY * norad->getLatitude()) / 180) + (mapY / 2);
//    targetPosition.x = lastPosition.x;
//    targetPosition.y = lastPosition.y;// + updateInterval.dbl();
    //norad->updateTime(simTime());
    lastPosition.x = mapX * norad->getLongitude() / 360 + (mapX / 2);
    lastPosition.x = static_cast<int>(lastPosition.x) % static_cast<int>(mapX);
    lastPosition.y = ((-mapY * norad->getLatitude()) / 180) + (mapY / 2);
    targetPosition.x = lastPosition.x;
    targetPosition.y = lastPosition.y;
    nextChange =  simTime() + updateInterval;

    { // Debugging prints for satellite position.
        H3Index cell = 0;
        const auto latLng = LatLng {deg2rad(getLatitude()), deg2rad(getLongitude())};

        latLngToCell(&latLng, 0, &cell);

        // std::cout << norad->getLongitude() << ", " << norad->getLatitude() << std::endl;
        // std::cout << std::hex << cell << std::dec << std::endl;

        // std::cout << std::dec << "(" << a->getLatitude() << ", " << a->getLongitude() << ") /// " << std::hex << this->getCurH3IndexForEid(eid) << std::endl;
        // std::cout << std::hex << this->getCurH3IndexForEid(eid) << std::endl;
        // std::cout << std::dec << eid << "," << eid << "," << a->getLatitude() << "," << a->getLongitude() << std::endl;
        // std::cout << std::dec << a->getLongitude() << "," << a->getLatitude() << std::endl;
        // std::cout << std::dec << a->getLatitude() << "," << a->getLongitude() << std::endl;
    }
}

void SatelliteMobility::move()
{
    LineSegmentsMobilityBase::move();
    //raiseErrorIfOutside();
}

void SatelliteMobility::fixIfHostGetsOutside()
{
    raiseErrorIfOutside();
}

omnetpp::simtime_t SatelliteMobility::getNextUpdateTime() {
    return LineSegmentsMobilityBase::nextChange;
}
} // namespace inet
