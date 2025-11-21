package src.node.mobility;

import inet.mobility.base.MovingMobilityBase;

//
// Manages the SatSGP4 Mobility - extension of OS3 mobility model
//
simple SatelliteMobility extends MovingMobilityBase
{
    parameters:
        @class("inet::SatelliteMobility");
        @display("i=block/cogwheel_s");
}