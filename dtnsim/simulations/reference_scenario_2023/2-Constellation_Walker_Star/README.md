# Reference Simulation

- To run the simulation from a terminal,  simply go to this directory and enter the following command line :

```bash
opp_runall -j8 ../../../dtnsim Reference.ini -n ../../../src > /dev/null 
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then to run the script :
```bash
python3 generateGraphs.py
```

## Parameters of this simulation

Here are all the parameters from this reference simulation

- **Constellation** : All the information on this constellation can be found in this [.json file](../1-Constellation_%26_Contact_Plan_Design/Star.json), with orbits from GW, and position for each EN and GS. The contact plan was made thanks to STK, during a 24h period.
  - 12 GW (nodes 1 to 12) in a Walker Star configuration
  - 100 EN (nodes 13 to 112) distributed uniformly and randomly over the Earth with [populateWithEndNodeAndGroundStations.py](../1-Constellation_&_Contact_Plan_Design/populateWithEndNodeAndGroundStations.py)
  - 10 GS (nodes 113 to 122) with a chosen location on the Earth
- **Routing** : *cgrModelRev17* was chosen, mainly because it's the only one that works on such constellations on dtnsim.
- **Bundle** : 
  - each EN sends a bundle every hour (3600s) for 9h starting at t=0s (10 bundles in total for each EN). The interval is chosen to be realistic, and the 9h duration is chosen to be in accord with the 24h of the contact plan.
  - A bundle weighs 100 B, as it's more likely a small information such as position or weather.
  - EN to GW, GW to GW and GW to GS data rate are fixed to 1 KB/s
- **Ground Station Network** : It has been assumed that all GS are linked to the internet. In consequence, there is a permanent and instantaneous link between each one of them, meaning that they can exchange bundles at all time and very quickly.
- **Failure** : This networks has been assumed perfect, meaning that nodes don't experience failures and are operational at all times.

## Resulting Graphs

- By running the python script, graphs are obtained. They are stored in the [figures.pdf](figures.pdf).

## Results

- The first important result is the **average delay of received bundles**. We can see that it's approx **2700s**, which corresponds to 45 min. It's not very fast, but given the situation that seems quite reasonable.
- The other important result might be the **average hops of received bundles**, to understand if our constellation is efficient or not. The minimum value being 2 (EN->GW and GW->GS), having **6** as a result on is a correct value, meaning that in average, a bundle will go through 5 satellites.
- We can also look at the **Dijkstra Calls** : this constellation average **140** calls per node, which is a lot knowing that each one only sends 10 bundles.
    > It is worth noting that calls aren't evenly distributed across the nodes : it's due to the position of the end node. Indeed, the more the node is near the poles, the more contacts it will have, and so have a lot of Dijkstra Calls to find all paths.
- Some graphs are not that interesting for absolute value but can be used for comparison between simulations. For example, the **bundleStoredTimeavg** is a mean on the 86400s of the simulation, knowing that the node send packets for only the first 32400s : the result is lower than in reality
