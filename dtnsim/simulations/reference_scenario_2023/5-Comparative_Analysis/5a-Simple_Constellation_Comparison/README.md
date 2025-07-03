# Constellation comparison

- This simulation is used to assess the impact of different constellations. The pre-selected constellation are the 3 previous constellations : a Walker Star, a Walker Delta and a Train, having each 12 satellites (10 GS and 100 EN). All the information about these constellations is available in their respective folder.

- To run the simulation from a terminal, simply go to this directory and enter the following command line:

```bash
opp_runall -j8 ../../../../dtnsim ConstellationAnalysis.ini -n ../../../../src > /dev/null
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then to run the script :

```bash
python3 ConstellationGraphs.py
```

To add a custom constellation, you just need to have your contact plan (generated with STK for example), put the .txt file in the *contactPlan/* folder, add the path in the [.ini file](ConstellationAnalysis.ini) (line 25) and the file name (without the .txt) in the [.py script](ConstellationGraphs.py) (line 20).

## Results obtained

- By running the python script, graphs are obtained. They are stored in the [figures.pdf](figures.pdf).
- We can easily see that the train constellation is not efficient compared to the other ones. More precisely, the average delay is way higher than the other constellations (more than 20 000s compared to less than 3 000s), and the average number of hops is also 33% lower (6 down to 4). It's also the only one with a PDR lower than 1.
- For the Delta and Star, they have similar results on delays and hops, but we can still see that Delta is a little better than Star, because figures are slightly better and also because of Dijkstra Calls. Indeed, the Delta constellation call 3 times less Dijkstra than the Star constellation (40 to 120), meaning that it needs less computing power.