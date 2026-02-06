package src.node.dtn.contactless;

import src.node.dtn.IDtn;

simple ContactlessDtn like IDtn
{
    parameters:

        @display("i=block/routing");

        string routing = default("antop");
        int bundlesCopies = default(1);
        double sContactProb = default(1.0);
        bool printRoutingDebug = default(false);
        bool saveBundleMap = default(false);
        int sdrSize = default(0); // SDR Memory Size in Bytes (0 = infinite)
        string frouting = default(""); //Path to file which encodes routing decisions (only valid for BRUF1T/BRUFNCopies routing)
        int ts_duration = default(-1); //The timestamp duration in seconds (only valid for CGR_BRUFPowered routing). Use ts_duration or ts_start_times both are not possible
        string ts_start_times = default(""); //"a,b,c,..." a,b,c are integer and they indicate the second of start of 0, 1, 2,.. time stamp (only valid for CGR_BRUFPowered routing) Use ts_duration or ts_start_times both are not possible.
        double pEncouterMax = default(0.7);
		double pEncouterFirst = default(0.5);
		double pFirstThreshold = default(0.1);
		double ForwThresh = default(0);
		double alpha = default(0.5);
		double beta = default(0.9);
		double gamma = default(0.999);
		double delta = default(0.01);
        double dataRate = default(5000);
        
        // Custody
        int custodyReportByteSize = default(20);
        double custodyTimeout = default(100);

        // Signals and Statistics
        @signal[dtnBundleSentToCom];
        @signal[dtnBundleSentToApp];
        @signal[dtnBundleSentToAppHopCount]; // how many times was a bundle transmitted
        @signal[dtnBundleSentToAppRevisitedHops]; // how many times was a bundle transmitted from a same node?
        @signal[dtnBundleReceivedFromCom];
        @signal[dtnBundleReceivedFromApp];
        @signal[dtnBundleReRouted]; // how many times was a bundle rerouted in a same node (contact end with bundles in queue)
        @signal[sdrBundleStored];
        @signal[sdrBytesStored];

        @statistic[dtnBundleSentToCom](title="dtnBundleSentToCom"; source="dtnBundleSentToCom"; record=count; interpolationmode=none);
        @statistic[dtnBundleSentToApp](title="dtnBundleSentToApp"; source="dtnBundleSentToApp"; record=count; interpolationmode=none);
        @statistic[dtnBundleSentToAppHopCount](title="dtnBundleSentToAppHopCount"; source="dtnBundleSentToAppHopCount"; record=vector,histogram; interpolationmode=none);
        @statistic[dtnBundleSentToAppRevisitedHops](title="dtnBundleSentToAppRevisitedHops"; source="dtnBundleSentToAppRevisitedHops"; record=histogram; interpolationmode=none);
        @statistic[dtnBundleReceivedFromCom](title="dtnBundleReceivedFromCom"; source="dtnBundleReceivedFromCom"; record=count; interpolationmode=none);
        @statistic[dtnBundleReceivedFromApp](title="dtnBundleReceivedFromApp"; source="dtnBundleReceivedFromApp"; record=count; interpolationmode=none);
        @statistic[dtnBundleReRouted](title="dtnBundleReRouted"; source="dtnBundleReRouted"; record=count; interpolationmode=none);
        @statistic[sdrBundleStored](title="sdrBundleStored"; source="sdrBundleStored"; record=timeavg,max,vector; interpolationmode=none);
        @statistic[sdrBytesStored](title="sdrBytesStored"; source="sdrBytesStored"; record=timeavg,max,vector; interpolationmode=none);

    gates:
        inout gateToApp;
        inout gateToCom;
}
