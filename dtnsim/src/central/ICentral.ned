package src.central;

moduleinterface ICentral
{
    parameters:
        int deleteNNodes;
        bool useSpecificFailureProbabilities;
        double failureProbability;
        string nodeIdsToDelete;
        bool useCentrality;
        bool faultsAware;
        int mode;
        bool useUncertainty;
        string collectorPath;
        int repetition;
}
