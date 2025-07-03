# Bundle Size Analysis

- This simulation is used to assess the impact of the size of bundles sent by end nodes. The reference simulation value is 100 B, and the other chosen value are 500 B, 1 KB, 5 KB and 10 KB. Remember that all data rate are set to 1 KB/s, which means that bundle sizes range from 10% to 1000% of the data rate second

- To run the simulation from a terminal,  simply go to this directory and enter the following command line :

```bash
opp_runall -j8 ../../../../dtnsim BundleSizeAnalysis.ini -n ../../../../src > /dev/null
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then 2 scripts can be run. The first one is used to compare just the bundle sizes :

```bash
python3 BundleSizeGraphs.py
``` 

- The second one is used to compare at the same time bundle size and constellation. Indeed, for each metric, the plot is made with bundle size as the x-axis, and has a curve per constellation :

```bash
python3 Size-Constellation.py
```

- To add a custom size or constellation, simply add the value in the [.ini file](BundleSizeAnalysis.ini) and in the [.py script](BundleSizeGraphs.py)

> Note that only 6 values can be compared simultaneously : certain values need to be deleted if you want to add several. Be careful to always have the same values in the .ini and .py files. 

## Resulting graphs

- By running the python script, 2 sets of graphs are obtained
  - A first set on the bundle size is stored in the [figures.pdf](figures.pdf). Those plots are done on the reference delta constellation.
  - A second set displaying both constellations and bundle size is stored in the [Size-Constellation](Size-Constellation.pdf)
- When increasing the size of bundles, we can see that Delta appears to be more effective (as it was anticipated in the previous simulation 5a), as the PDR falls faster for Star than for Delta (0.95 to 0.96 for size = 10 000B). We can also see that the average delay skyrockets as size increases for Star (4700s to 3300s for size = 10 000B)
- The interesting point here is the apparition of rerouted bundles as the size goes beyond 5 000B. It explains in part the previous differences and shows that Star is less resistant to change than Delta.
