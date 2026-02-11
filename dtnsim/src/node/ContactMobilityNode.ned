package src.node;

import src.node.mobility.ContactSatelliteMobility;
import src.node.mobility.NoradA;

module ContactMobilityNode {
    parameters:
        @display("i=device/satellite");
        string icon = default("satellite");
        string satelliteName = default(""); // Parameter for satellite name
        double rate = default(5000);

    submodules:
        mobility: ContactSatelliteMobility {
            parameters:
                @display("p=194,103");
        }
        norad: NoradA {
            parameters:
                @display("p=98,35");
        }

    connections:
}
