# Packet Rate Analysis

- This simulation is used to assess the impact of the packet rate sent by end nodes. The reference simulation periodicity is one packet every 3600s, 360s and 36s. To achieve such rate, the total number of bundles have been changed to keep sending bundles for 10 hours

- To run the simulation from a terminal,  simply go to this directory and enter the following command line :

```
opp_runall -j8 ../../../../dtnsim BundleSizeAnalysis.ini -n ../../../../src > /dev/null
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then 2 scripts can be run. The first one is used to compare just the bundle sizes : 

```
python3 RateGraphs.py
```

- The second one is used to compare at the same time bundle size and constellation. Indeed, for each metric, the plot is made with bundle rate as the x-axis, and has a curve per constellation :

```
python3 Rate-Constellation.py
```

- To add a custom rate or constellation, simply add the value in the [.ini file](RateAnalysis.ini) and in the two .py scripts [1](RateGraphs.py) [2](Rate-Constellation.py)
    > Note that only 6 values can be compared simultaneously : certain values need to be deleted if you want to add several. Be careful to always have the same values in the .ini and .py files

## Resulting graphs

- By running the python script, 2 sets of graphs are obtained
  - A first set on the packet rate is stored in the [figures.pdf](figures.pdf)
  - A second set displaying both constellation and packet rate is stored in the [Rate-Constellation](Rate-Constellation.pdf)
- The results from this simulation are the same from the previous one (5b) and therefore need no specific explanations