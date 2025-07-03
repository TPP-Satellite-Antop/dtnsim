import os

SIMULATION_PATH = "../../../experiment_results"
SIM_NUMBER = 8
pfs = [-1]

def generate_omnet_ini_file(bundle_infos, algorithm, mode, fp, sim):
	final_string = ""
	final_string += '[General]\n'
	final_string += 'network = src.dtnsim\nrepeat = 30\n'
	final_string += 'dtnsim.nodesNumber = 15\ndtnsim.node[1..3].icon = "satellite"\n'
	final_string += 'dtnsim.node[*].fault.enable = false\ndtnsim.node[*].fault.faultSeed = ${repetition}*10\ndtnsim.node[*].fault.meanTTF = 20s\ndtnsim.node[*].fault.meanTTR = 5s\n'
	final_string += 'dtnsim.node[*].dtn.routingType = "routeListType:allPaths-initial+anchor,volumeAware:allContacts,extensionBlock:on,contactPlan:local"\ndtnsim.node[*].dtn.printRoutingDebug = true\n'
	final_string += 'dtnsim.central.contactsFile = "contactPlan/contactPlan.txt"\ndtnsim.central.useSpecificFailureProbabilities = true\n'
	final_string += 'seed-set = ${repetition}\n'
	final_string += 'dtnsim.central.repetition = ${repetition}\n'
	final_string += 'dtnsim.node[*].dtn.saveBundleMap = true\ndtnsim.central.saveTopology = true\ndtnsim.central.saveFlows = true\n'
	if fp != -1:
		final_string += 'dtnsim.central.failureProbability = ' + str(fp * 100) + "\n"
	else:
		final_string += 'dtnsim.central.failureProbability = ' + str(-1) + "\n"
	final_string += 'dtnsim.central.mode = ' + str(mode) + "\n"
	if algorithm == "OCGR":
		final_string += 'dtnsim.central.useUncertainty = false\n'
		final_string += 'dtnsim.node[*].dtn.routing = "uncertainUniboCgr"\n'
	elif algorithm == "OCGR-UCoP":
		final_string += 'dtnsim.central.useUncertainty = true\n'
		final_string += 'dtnsim.node[*].dtn.routing = "uncertainUniboCgr"\n'
	else:
		final_string += 'dtnsim.central.useUncertainty = true\n'
		final_string += 'dtnsim.node[*].dtn.routing = "ORUCOP"\n'
	final_string += 'dtnsim.central.collectorPath = "../../../experiment_results/simulation_' + str(sim) + '"\n'
	final_string += generate_omnet_traffic(bundle_infos)


	with open("sim.ini", "w") as output:
		output.write(final_string)
		

def generate_omnet_traffic(bundle_infos):
	bundle_string = ""
	
	for source in bundle_infos:
		bundleNumber = ''
		bundleStarts = ''
		bundleDestinations = ''
		bundleSizes = ''
		bundle_string += 'dtnsim.node[' + str(source) + '].app.enable=true\n'
		bundle_string += 'dtnsim.node[' + str(source) + '].app.returnToSender=false\n'
		if len(bundle_infos[source]) == 1:
			bundleNumber += '"1"\n'
			bundleStarts += '"' + str(bundle_infos[source][0]["start"]) + '"\n'
			bundleDestinations += '"' + str(bundle_infos[source][0]["destination"]) + '"\n'
			bundleSizes += '"0"\n'
		else:
			for i in range(0, len(bundle_infos[source])):
				if i == (len(bundle_infos[source]) - 1):
					bundleNumber += '1"\n'
					bundleStarts += str(bundle_infos[source][i]["start"]) + '"\n'
					bundleDestinations += str(bundle_infos[source][i]["destination"]) + '"\n'
					bundleSizes += '0"\n'
				elif i == 0:
					bundleNumber += '"1, '
					bundleStarts += '"' + str(bundle_infos[source][i]["start"]) + ', '
					bundleDestinations += '"' + str(bundle_infos[source][i]["destination"]) + ", "
					bundleSizes += '"0, '
				else:
					bundleNumber += '1, '
					bundleStarts += str(bundle_infos[source][i]["start"]) + ', '
					bundleDestinations += str(bundle_infos[source][i]["destination"]) + ", "
					bundleSizes += '0, '
			
		bundle_string += 'dtnsim.node[' + str(source) + '].app.bundlesNumber=' + bundleNumber 
		bundle_string += 'dtnsim.node[' + str(source) + '].app.start=' + bundleStarts 
		bundle_string += 'dtnsim.node[' + str(source) + '].app.destinationEid=' + bundleDestinations 
		bundle_string += 'dtnsim.node[' + str(source) + '].app.size=' + bundleSizes 
		
	return bundle_string



def setup_run(path):
	os.system("rm -r " + path + "/metrics")
	os.system("mkdir " + path + "/metrics")
	
def setup_pf(path):
	setup_run(path + "/no_opp")
	setup_run(path + "/opp_not_known")
	setup_run(path + "/opp_known")
	
def setup_pfs(path):
	for pf in pfs:
		setup_pf(path + "/pf=" + str(pf))
	
def setup_algorithms(path):
	#setup_pfs(path + "/OCGR")
	#setup_pfs(path + "/ORUCoP")
	setup_pfs(path + "/OCGR-UCoP")
	
def setup_simulation():
	setup_algorithms(SIMULATION_PATH + "/simulation_" +  str(SIM_NUMBER))
	
def get_bundle_infos(sim):
	if sim == 2:
		bundle_infos = dict()
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 15
		bundle_infos[1][0]["destination"] = 12
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 60
		bundle_infos[12][0]["destination"] = 11
		bundle_infos[8] = dict()
		bundle_infos[8][0] = dict()
		bundle_infos[8][0]["start"] = 40
		bundle_infos[8][0]["destination"] = 2
		bundle_infos[5] = dict()
		bundle_infos[5][0] = dict()
		bundle_infos[5][0]["start"] = 70
		bundle_infos[5][0]["destination"] = 3
		bundle_infos[2] = dict()
		bundle_infos[2][0] = dict()
		bundle_infos[2][0]["start"] = 60
		bundle_infos[2][0]["destination"] = 4
		bundle_infos[9] = dict()
		bundle_infos[9][0] = dict()
		bundle_infos[9][0]["start"] = 60
		bundle_infos[9][0]["destination"] = 10
		bundle_infos[4] = dict()
		bundle_infos[4][0] = dict()
		bundle_infos[4][0]["start"] = 55
		bundle_infos[4][0]["destination"] = 5
		bundle_infos[6] = dict()
		bundle_infos[6][0] = dict()
		bundle_infos[6][0]["start"] = 25
		bundle_infos[6][0]["destination"] = 3
	elif sim == 1:
		bundle_infos = dict()
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 25
		bundle_infos[1][0]["destination"] = 11
		bundle_infos[1][1] = dict()
		bundle_infos[1][1]["start"] = 35
		bundle_infos[1][1]["destination"] = 6
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 5
		bundle_infos[10][0]["destination"] = 6
		bundle_infos[9] = dict()
		bundle_infos[9][0] = dict()
		bundle_infos[9][0]["start"] = 55
		bundle_infos[9][0]["destination"] = 12
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 65
		bundle_infos[3][0]["destination"] = 9
		bundle_infos[5] = dict()
		bundle_infos[5][0] = dict()
		bundle_infos[5][0]["start"] = 35
		bundle_infos[5][0]["destination"] = 10
		bundle_infos[4] = dict()
		bundle_infos[4][0] = dict()
		bundle_infos[4][0]["start"] = 80
		bundle_infos[4][0]["destination"] = 1
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 85
		bundle_infos[12][0]["destination"] = 1
	elif sim == 3:
		bundle_infos = dict()
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 45
		bundle_infos[3][0]["destination"] = 7
		bundle_infos[7] = dict()
		bundle_infos[7][0] = dict()
		bundle_infos[7][0]["start"] = 0
		bundle_infos[7][0]["destination"] = 5
		bundle_infos[8] = dict()
		bundle_infos[8][0] = dict()
		bundle_infos[8][0]["start"] = 30
		bundle_infos[8][0]["destination"] = 3
		bundle_infos[8][1] = dict()
		bundle_infos[8][1]["start"] = 20
		bundle_infos[8][1]["destination"] = 4
		bundle_infos[11] = dict()
		bundle_infos[11][0] = dict()
		bundle_infos[11][0]["start"] = 80
		bundle_infos[11][0]["destination"] = 1
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 10
		bundle_infos[12][0]["destination"] = 5
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 30
		bundle_infos[10][0]["destination"] = 7
		bundle_infos[4] = dict()
		bundle_infos[4][0] = dict()
		bundle_infos[4][0]["start"] = 85
		bundle_infos[4][0]["destination"] = 3
	elif sim == 4:
		bundle_infos = dict()
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 0
		bundle_infos[1][0]["destination"] = 6
		bundle_infos[9] = dict()
		bundle_infos[9][0] = dict()
		bundle_infos[9][0]["start"] = 0
		bundle_infos[9][0]["destination"] = 4
		bundle_infos[6] = dict()
		bundle_infos[6][0] = dict()
		bundle_infos[6][0]["start"] = 0
		bundle_infos[6][0]["destination"] = 10
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 40
		bundle_infos[10][0]["destination"] = 9
		bundle_infos[5] = dict()
		bundle_infos[5][0] = dict()
		bundle_infos[5][0]["start"] = 25
		bundle_infos[5][0]["destination"] = 12
		bundle_infos[2] = dict()
		bundle_infos[2][0] = dict()
		bundle_infos[2][0]["start"] = 15
		bundle_infos[2][0]["destination"] = 8
		bundle_infos[7] = dict()
		bundle_infos[7][0] = dict()
		bundle_infos[7][0]["start"] = 40
		bundle_infos[7][0]["destination"] = 12
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 55
		bundle_infos[3][0]["destination"] = 8
	elif sim == 5:
		bundle_infos = dict()
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 0
		bundle_infos[1][0]["destination"] = 12
		bundle_infos[7] = dict()
		bundle_infos[7][0] = dict()
		bundle_infos[7][0]["start"] = 25
		bundle_infos[7][0]["destination"] = 3
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 25
		bundle_infos[10][0]["destination"] = 6
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 0
		bundle_infos[12][0]["destination"] = 5
		bundle_infos[11] = dict()
		bundle_infos[11][0] = dict()
		bundle_infos[11][0]["start"] = 25
		bundle_infos[11][0]["destination"] = 1
		bundle_infos[8] = dict()
		bundle_infos[8][0] = dict()
		bundle_infos[8][0]["start"] = 55
		bundle_infos[8][0]["destination"] = 4
		bundle_infos[6] = dict()
		bundle_infos[6][0] = dict()
		bundle_infos[6][0]["start"] = 35
		bundle_infos[6][0]["destination"] = 7
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 50
		bundle_infos[3][0]["destination"] = 4
	elif sim == 6:
		bundle_infos = dict()
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 0
		bundle_infos[1][0]["destination"] = 11
		bundle_infos[9] = dict()
		bundle_infos[9][0] = dict()
		bundle_infos[9][0]["start"] = 55
		bundle_infos[9][0]["destination"] = 3
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 35
		bundle_infos[12][0]["destination"] = 4
		bundle_infos[12][1] = dict()
		bundle_infos[12][1]["start"] = 35
		bundle_infos[12][1]["destination"] = 5
		bundle_infos[4] = dict()
		bundle_infos[4][0] = dict()
		bundle_infos[4][0]["start"] = 25
		bundle_infos[4][0]["destination"] = 6
		bundle_infos[8] = dict()
		bundle_infos[8][0] = dict()
		bundle_infos[8][0]["start"] = 45
		bundle_infos[8][0]["destination"] = 1
		bundle_infos[7] = dict()
		bundle_infos[7][0] = dict()
		bundle_infos[7][0]["start"] = 25
		bundle_infos[7][0]["destination"] = 9
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 25
		bundle_infos[3][0]["destination"] = 10
	elif sim == 7:
		bundle_infos = dict()
		bundle_infos[8] = dict()
		bundle_infos[8][0] = dict()
		bundle_infos[8][0]["start"] = 15
		bundle_infos[8][0]["destination"] = 3
		bundle_infos[11] = dict()
		bundle_infos[11][0] = dict()
		bundle_infos[11][0]["start"] = 45
		bundle_infos[11][0]["destination"] = 6
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 35
		bundle_infos[10][0]["destination"] = 5
		bundle_infos[1] = dict()
		bundle_infos[1][0] = dict()
		bundle_infos[1][0]["start"] = 55
		bundle_infos[1][0]["destination"] = 9
		bundle_infos[7] = dict()
		bundle_infos[7][0] = dict()
		bundle_infos[7][0]["start"] = 75
		bundle_infos[7][0]["destination"] = 12
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 25
		bundle_infos[3][0]["destination"] = 11
		bundle_infos[2] = dict()
		bundle_infos[2][0] = dict()
		bundle_infos[2][0]["start"] = 15
		bundle_infos[2][0]["destination"] = 10
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 25
		bundle_infos[12][0]["destination"] = 10
	elif sim == 8:
		bundle_infos = dict()
		bundle_infos[4] = dict()
		bundle_infos[4][0] = dict()
		bundle_infos[4][0]["start"] = 55
		bundle_infos[4][0]["destination"] = 9
		bundle_infos[11] = dict()
		bundle_infos[11][0] = dict()
		bundle_infos[11][0]["start"] = 95
		bundle_infos[11][0]["destination"] = 2
		bundle_infos[10] = dict()
		bundle_infos[10][0] = dict()
		bundle_infos[10][0]["start"] = 65
		bundle_infos[10][0]["destination"] = 3
		bundle_infos[10][1] = dict()
		bundle_infos[10][1]["start"] = 140
		bundle_infos[10][1]["destination"] = 6
		bundle_infos[3] = dict()
		bundle_infos[3][0] = dict()
		bundle_infos[3][0]["start"] = 25
		bundle_infos[3][0]["destination"] = 7
		bundle_infos[9] = dict()
		bundle_infos[9][0] = dict()
		bundle_infos[9][0]["start"] = 115
		bundle_infos[9][0]["destination"] = 4
		bundle_infos[5] = dict()
		bundle_infos[5][0] = dict()
		bundle_infos[5][0]["start"] = 125
		bundle_infos[5][0]["destination"] = 12
		bundle_infos[12] = dict()
		bundle_infos[12][0] = dict()
		bundle_infos[12][0]["start"] = 45
		bundle_infos[12][0]["destination"] = 1
	return bundle_infos
	
def run_simulations(bundle_infos, algorithm, mode, pf, sim):
	generate_omnet_ini_file(bundle_infos, algorithm, mode, pf, sim)
	os.system("mkdir sharedFolder")
	os.system("opp_runall -j1 ../../../dtnsim_dbg sim.ini -n ../../../ -u Cmdenv -c General")
	os.system("rm -r sharedFolder")
	
def main():
	setup_simulation()
	sim_infos = dict()
	sim_infos[SIM_NUMBER] = get_bundle_infos(SIM_NUMBER)
	for pf in pfs:
		run_simulations(sim_infos[SIM_NUMBER], "ORUCOP", 0, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR-UCoP", 0, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR", 0, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "ORUCOP", 1, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR-UCoP", 1, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR", 1, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "ORUCOP", 2, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR-UCoP", 2, pf, SIM_NUMBER)
		run_simulations(sim_infos[SIM_NUMBER], "OCGR", 2, pf, SIM_NUMBER)
		


if __name__ == "__main__":
    main()
