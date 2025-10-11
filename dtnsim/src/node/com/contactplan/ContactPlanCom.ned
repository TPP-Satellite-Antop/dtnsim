package src.node.com.contactplan;
import src.node.com.ICom;

simple ContactPlanCom like ICom
{
    parameters:
        @display("i=block/wtx");
        double packetLoss = default(0.0);
    gates:
        inout gateToDtn;
        input gateToAir @directIn;
}