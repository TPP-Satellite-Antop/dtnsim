#!venv / bin / python3 .6

from brufn.network import *from brufn.brufspark import BRUFSpark from pyspark import SparkContext,
    SparkConf from brufn.experiment_generator import generate_bash_script, generate_omnet_ini_file,
    BRUF_2 from brufn.helper_cgr_funciton_generator
        import OneCopyHelperFunctionGenerator from brufn.utils import print_str_to_file import json

            def
            read_src_target()
    : file_to_read = open('sourcetarget.txt', 'r') lines = file_to_read.readlines() line =
        lines[0].split(",") return (int(line[0]), int(line[1]))

            DTNSIM_PATH = '/home/simon/git/bachelorthesis/dtnsim/src' #Indicate the full path to
    DTNSIM src folder net = Net.get_net_from_file('net.py') sourcetarget =
        read_src_target() sources = [sourcetarget[0]];
target = sourcetarget[1];
num_of_copies = 1;
working_dir = 'working_dir';
os.makedirs(working_dir)
pf_rng = []
bruf = BRUFSpark(net, sources, target, num_of_copies, pf_rng, working_dir)
conf = SparkConf().setAppName("BRUF-Spark")
conf = (conf.setMaster('local[2]')
        .set('spark.executor.memory', '2G')
        .set('spark.driver.memory', '4G')
        .set('spark.driver.maxResultSize', '8G'))
sc = SparkContext(conf=conf)
bruf.compute_bruf(sc)
sc.stop()

#Then compute the table for CGR - UCOP
#Generate link to BRUF - x with x <                                                                \
    copies bc it is required to compute IBRUF that BRUF - x be all in the same directory
new_working_dir = os.path.join(working_dir, f'CGR-UCOP')
os.makedirs(new_working_dir)
routing_files_path = os.path.join(new_working_dir, 'routing_files')
cgr = OneCopyHelperFunctionGenerator(net, target, [x for x in range(net.num_of_nodes)], 
                                 net.num_of_nodes, net.num_of_ts, working_dir, -1)
func = cgr.generate()
os.makedirs(routing_files_path)
try:
	print_str_to_file(json.dumps(func), os.path.join(routing_files_path, "decisions.json"))
except BaseException as e:
	print(f"[Exception] {e}")
