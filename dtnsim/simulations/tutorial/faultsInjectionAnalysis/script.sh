#!/bin/bash


# run example 5 config example5a
opp_runall -j4 ../../../src/dtnsim example5.ini -n ../../../src -u Cmdenv -c example5a
# run example 5 config example5b
opp_runall -j4 ../../../src/dtnsim example5.ini -n ../../../src -u Cmdenv -c example5b

# run example 6 config example6a
opp_runall -j4 ../../../src/dtnsim example6.ini -n ../../../src -u Cmdenv -c example6a
# run example 6 config example6b
opp_runall -j4 ../../../src/dtnsim example6.ini -n ../../../src -u Cmdenv -c example6b


: <<'END'
END
