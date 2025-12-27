package src.node.fault;

simple Fault
{
    parameters:
        @display("i=block/control");

        bool enable = default(false);
        int faultSeed = default(0);
        double meanTTF @unit(seconds) = default(0s); // If 0 no faults enabled. Mean Time To Failure
        double meanTTR @unit(seconds) = default(0s); // If 0 no faults enabled. Mean Time To Repair / Recovery
}
