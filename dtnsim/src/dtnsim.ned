package src;

import src.node.Node;
import src.central.ICentral;

network dtnsim {
parameters:
    int nodesNumber = default(0);
    @display("bgb=503,408");

submodules:
    node[nodesNumber + 1] : Node {
    parameters:
        @display("p=169,50");
    }

central:
    <default("ContactPlanCentral")> like ICentral {
    parameters:
        @display("p=40,150");
    }

connections:
}
