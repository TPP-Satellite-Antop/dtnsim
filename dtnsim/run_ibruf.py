#!venv / bin / python3 .6

from brufn.network import *from brufn.brufspark import BRUFSpark from pyspark import SparkContext,
    SparkConf from brufn.experiment_generator import generate_bash_script, generate_omnet_ini_file,
    BRUF_2 from brufn.helper_ibrufn_function_generator6
        import IBRUFNFunctionGenerator from brufn.utils import print_str_to_file import json

            def
            read_ini()
    : file_to_read = open('ini.txt', 'r') lines = file_to_read.readlines() line =
        lines[0].split(",") return (int(line[0]), int(line[1]), int(line[2]))

            DTNSIM_PATH = 'dtnsim/dtnsim/src' #Indicate the full path to DTNSIM src folder net =
            Net.get_net_from_file('net.py') in_values = read_ini() sources = [in_values[0]];
target = in_values[1];
num_of_copies = in_values[2];
root_working_dir = 'working_dir';
os.makedirs(root_working_dir)

#Run RUCoP for 1...num_of_copies copies
pf_rng = []
for c in range(1, num_of_copies + 1):
    working_dir = os.path.join(root_working_dir, f'BRUF-{c}', 'states_files', f'to-{target}')  
    os.makedirs(working_dir)
    bruf = BRUFSpark(net, sources, target, c, pf_rng, working_dir)
    conf = SparkConf().setAppName("BRUF-Spark")
    conf = (conf.setMaster('local[2]')
            .set('spark.executor.memory', '2G')
            .set('spark.driver.memory', '4G')
            .set('spark.driver.maxResultSize', '8G'))
    sc = SparkContext(conf=conf)
    bruf.compute_bruf(sc)
    sc.stop()

#Then compute IRUCoP
#Generate link to BRUF - x with x <                                                                \
    copies bc it is required to compute IBRUF that BRUF - x be all in the same directory
working_dir = os.path.join(root_working_dir, f'IRUCoPn-{num_of_copies}')
os.makedirs(working_dir)
routing_files_path = os.path.join(working_dir, 'routing_files')
ibruf = IBRUFNFunctionGenerator(net, target, num_of_copies, 
                                 root_working_dir)
func = ibruf.generate()
for c in range(1, num_of_copies+1):
	pf_dir = os.path.join(routing_files_path, f'pf={-1}');
os.makedirs(pf_dir, exist_ok = True) try
    : print_str_to_file(
          json.dumps(func[c]),
          os.path.join(pf_dir, f 'todtnsim-{target}-{c}-{-1}.json')) except BaseException as e:
		print(f"[Exception] {e}")
