package src.node.com;
import src.node.com.ICom;

simple Com like ICom
{
    parameters:
        @display("i=block/wtx");
        double packetLoss = default(0.0);
    gates:
        inout gateToDtn;
        input gateToAir @directIn;
}