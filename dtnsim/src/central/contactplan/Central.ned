package src.central.contactplan;

import src.central.ICentral;

simple ContactPlanCentral like ICentral
{
    parameters:
        @class(dtnsim::ContactPlanCentral);

        bool saveTopology = default(false);
        bool saveFlows = default(false);
        bool saveLpFlows = default(false);
        string contactsFile = default("contacts.txt");

        bool enableAvailableRoutesCalculation = default(false);

        // Contact failure feature
        int deleteNNodes = default(0);
        bool useSpecificFailureProbabilities = default(false);
        double failureProbability = default(0); // Probability of failure of any contact
        string nodeIdsToDelete = default(""); // Ids of contacts to be deleted

        bool useCentrality = default(false);
        bool faultsAware = default(true);
        int mode = default(1); // mode of including contacts, 0=no opp, 1=regular, 2=complete
        bool useUncertainty = default(false);
        string collectorPath = default("");
        int repetition = default(0);

        // Signals
        @signal[nodesNumber];
        @signal[totalRoutes];
        @signal[availableRoutes];
        @signal[pairsOfNodesWithAtLeastOneRoute];

        // Statistics
        @statistic[nodesNumber](title="nodesNumber"; source="nodesNumber"; record=sum; interpolationmode=none);
        @statistic[totalRoutes](title="totalRoutes"; source="totalRoutes"; record=sum; interpolationmode=none);
        @statistic[availableRoutes](title="availableRoutes"; source="availableRoutes"; record=sum; interpolationmode=none);
        @statistic[pairsOfNodesWithAtLeastOneRoute](title="pairsOfNodesWithAtLeastOneRoute"; source="pairsOfNodesWithAtLeastOneRoute"; record=sum; interpolationmode=none);

        @display("i=device/terminal");
}
