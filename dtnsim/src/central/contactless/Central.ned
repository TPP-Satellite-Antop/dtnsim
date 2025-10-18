package src.central.contactless;

import src.central.ICentral;

simple ContactlessCentral like ICentral
{
    parameters:
        @class(dtnsim::ContactlessCentral);
        int deleteNNodes = default(0);
        bool useSpecificFailureProbabilities = default(false);
        double failureProbability = default(0);
        string nodeIdsToDelete = default("");
        bool useCentrality = default(false);
        bool faultsAware = default(true);
        int mode = default(1);
        bool useUncertainty = default(false);
        string collectorPath = default("");
        int repetition = default(0);
    @display("i=device/terminal");
}
