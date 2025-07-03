# Contact Plan Simplification

- This simulation is used to compare different scenarios without having to generate a new contact plan through STK for example. Indeed, who can do more can do less. This simulation consists in deleting nodes from the original plan contact to obtain something new, simpler. It can be used for example to see what appends if a satellite falls out of service.

- First, you have to generate the contact plan you want. To do so, you may execute a python script that, based on the reference Walker Delta contact Plan, remove random nodes. You still chose how many nodes you want. To run the script from a terminal, go to this directory and enter the following command line, with
  - [GW] the number of GW to keep
  - [EN] the number of EN to keep
  - [GS] the number of GS to keep

> These parameters are mandatory. The selection of which nodes to keep is made randomly. The generated contactPlan is [Custom.txt](contactPlan/Custom.txt). At the beginning of this file, IDs of kept nodes are written

> When a satellite is removed. All its contacts are removed.

```bash
python3 RemoveContactScript.py [GW] [EN] [GS]
# python3 RemoveContactScript.py 1 10 2 # keep 1 satellite, 10 End-Nodes and 2 GRound Stations (randomly chosen among the existing ones)
```

- To run the simulation from a terminal,  simply go to this directory and enter the following command line :

```bash
opp_runall -j8 ../../../../dtnsim RemoveContact.ini -n ../../../../src > /dev/null
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then to run the script to generate graphs : 

```bash
python3 generateGraph.py
```

## Resulting Graph

- By running the python script, graphs are obtained. They are stored in the [figures.pdf](figures.pdf).
