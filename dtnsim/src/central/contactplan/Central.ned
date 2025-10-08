package src.central.contactplan;

simple Central
{
    parameters:
        
        @class(dtnsim::ContactPlanCentral);
        
        bool saveTopology = default(false);
        bool saveFlows = default(false);
        bool saveLpFlows = default(false);
        string contactsFile = default("contacts.txt");
        
        bool enableAvailableRoutesCalculation = default(false);
        
          
//      	Contact Failure feature:
//      	DTNSim supports 4 differents ways in order to model contact failures:
//      		1) Delete a given number of contacts, which are randomly selected. (Set parameter deleteNContacts)
//			    2) Delete contact with a given probability of failure, which is specific for each contact and is contained within the contact plan.
//      		3) Delete contacts with a given probability of failure. 
//      		That is, any contact fails with a given probility. (Set parameter failureProbability)
//      		4) Delete contacts by id. A list of contact ids is given, and the repectives contacts are deleted. (Set contactIdsToDelete)
//      		
//      		Only one of the above ways can work each time. If more than one is selected, 
//      		DTNSim chooses one according the above list(1 is priritized respects 2 and 2 respects 3 and 3 respects 4).
//      		In order to enable a contact failure model, set some of the following parameters.    
        	       
        int deleteNContacts = default(0);
        bool useSpecificFailureProbabilities = default(false);
        double failureProbability = default(0); //Probability of failure of any contact in the contact plan 
		string contactIdsToDelete = default(""); //Ids of contact to be deleted
		
		bool useCentrality = default(false);
		bool faultsAware = default(true);
		int mode = default(1); //mode to define in which opp. contacts are included in the simulation, 0 for no opp. contacts, 1 for regular contact discovery, 2 for complete knowledge in advance
		bool useUncertainty = default(false);
		string collectorPath = default("");
		int repetition = default(0);
		
		@signal[contactsNumber];
		@signal[totalRoutes];
		@signal[availableRoutes];
		@signal[pairsOfNodesWithAtLeastOneRoute];
        
        @statistic[contactsNumber](title="contactsNumber"; source="contactsNumber"; record=sum; interpolationmode=none);
        @statistic[totalRoutes](title="totalRoutes"; source="totalRoutes"; record=sum; interpolationmode=none);
        @statistic[availableRoutes](title="availableRoutes"; source="availableRoutes"; record=sum; interpolationmode=none);
        @statistic[pairsOfNodesWithAtLeastOneRoute](title="pairsOfNodesWithAtLeastOneRoute"; source="pairsOfNodesWithAtLeastOneRoute"; record=sum; interpolationmode=none);
        
        @display("i=device/terminal");
}
