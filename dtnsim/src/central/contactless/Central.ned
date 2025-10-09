package src.central.contactless;

simple Central
{
    parameters:
        
        @class(dtnsim::ContactlessCentral);       
          
//      	Contact Failure feature:
//      	DTNSim supports 4 differents ways in order to model contact failures:
//      		1) Delete a given number of nodes, which are randomly selected. (Set parameter deleteNNodes)
//			    2) Delete contact with a given probability of failure, which is specific for each contact and is contained within the contact plan.
//      		3) Delete nodes with a given probability of failure. 
//      		That is, any contact fails with a given probility. (Set parameter failureProbability)
//      		4) Delete nodes by id. A list of contact ids is given, and the repectives nodes are deleted. (Set nodeIdsToDelete)
//      		
//      		Only one of the above ways can work each time. If more than one is selected, 
//      		DTNSim chooses one according the above list(1 is priritized respects 2 and 2 respects 3 and 3 respects 4).
//      		In order to enable a contact failure model, set some of the following parameters.    
        	       
        int deleteNNodes = default(0);
        bool useSpecificFailureProbabilities = default(false);
        double failureProbability = default(0); //Probability of failure of any node 
		string nodeIdsToDelete = default(""); //Ids of nodes to be deleted
		
		bool useCentrality = default(false);
		bool faultsAware = default(true);
		int mode = default(1); //mode to define in which opp. nodes are included in the simulation, 0 for no opp. nodes, 1 for regular contact discovery, 2 for complete knowledge in advance
		bool useUncertainty = default(false);
		string collectorPath = default("");
		int repetition = default(0);
		
        @display("i=device/terminal");
}
