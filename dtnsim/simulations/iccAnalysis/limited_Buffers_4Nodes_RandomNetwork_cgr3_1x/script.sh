#!/bin/bash

opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=false_density=0.1.ini -n ../../../src -u Cmdenv -c dtnsim
opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=false_density=0.5.ini -n ../../../src -u Cmdenv -c dtnsim
opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=false_density=1.0.ini -n ../../../src -u Cmdenv -c dtnsim

opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=true_density=0.1.ini -n ../../../src -u Cmdenv -c dtnsim
opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=true_density=0.5.ini -n ../../../src -u Cmdenv -c dtnsim
opp_runall -j4 ../../../src/dtnsim test_Ion_4Nodes_deleteContacts_random_faultsAware=true_density=1.0.ini -n ../../../src -u Cmdenv -c dtnsim


: <<'END'
END
