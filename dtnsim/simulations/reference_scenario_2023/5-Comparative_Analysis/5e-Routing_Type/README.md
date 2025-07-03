# Routing Type Analysis

- This simulation is used to assess the impact of the routing type. As only the cgrModelRev17 and the epidemic are fully implemented and functionnal in dtnsim, the choice was made to analyse the routing type instead, which are different variants of cgrModelRev17. The two most interresting are first-ending and first-depleting.

- First-Ending variant : All paths are calculated at one when the contact plan changes. The route list is completed removing the first ending contact. 
    > For each route added to the table, the first ending contact of this route is search and once found, added to a list of suppressed contacts.

- First-Depleted variant :All paths are calculated at once when the contact plan changes. The route list is completed removing the first depleted contact.
    > For each route added to the table, the residual volume is consummed for each contact, and if it's the limiting contact (residual volue remaining is null), this contact is added to a list of suppressed contacts.

## To run the simulation

- To run the simulation from a terminal,  simply go to this directory and enter the following command line :

```bash
opp_runall -j8 ../../../../dtnsim RoutingTypeAnalysis.ini -n ../../../../src > /dev/null
# -j<N> to specify the number of CPUs to use
# https://doc.omnetpp.org/omnetpp/manual/#sec:run-sim:batches-using-opp-runall
```

- Then 2 scripts can be run. The first one is used to compare just the bundle sizes : 

```bash
python3 RoutingTypeGraphs.py
``` 

- The second one is used to compare at the same time bundle size and constellation. Indeed, for each metric, the plot is made with bundle size as the x-axis, and has a curve per constellation :

```bash
python3 RoutingType-Constellation.py
```

- To add a custom RoutingType or constellation, simply add the value in the [.ini file](RoutingTypeAnalysis.ini) and in the [.py script](RoutingTypeGraphs.py)
    > Note that only 6 values can be compared simultaneously : certain values need to be deleted if you want to add several. Be careful to always have the same values in the .ini and .py files. 

## Resulting graphs

- By running the python script, 2 sets of graphs are obtained
  - A first set on the packet rate is stored in the [figures.pdf](figures.pdf).
  - A second set displaying both constellation and routing type is stored in the [RoutingType-Constellation](RoutingType-Constellation.pdf)
- As it was in the previous simulations, Delta appears to be better than Star. Routing speaking, the FirstEnding variant seems slightly better than the FirstDepleted one, but with only a marginal difference : 2630s to 2650s of average delay