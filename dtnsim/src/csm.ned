package src;

import src.node.ContactMobilityNode;

network csm {
    parameters:
        int nodesNumber = default(0);
        @display("bgb=503,408");

    submodules:
        node[nodesNumber + 1]: ContactMobilityNode {
            parameters:
                @display("p=169,50");
        }

    connections:
}
