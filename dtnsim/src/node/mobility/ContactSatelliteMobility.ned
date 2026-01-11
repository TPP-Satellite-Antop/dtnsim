package src.node.mobility;

import src.node.mobility.SatelliteMobility;

//
// Manages the SatSGP4 Mobility - extension of OS3 mobility model
//
simple ContactSatelliteMobility extends SatelliteMobility
{
    parameters:
        @class("inet::ContactSatelliteMobility");
        @display("i=block/cogwheel_s");
}