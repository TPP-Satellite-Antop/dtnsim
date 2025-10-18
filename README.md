# DTNSIM #

DTNSIM is a simulator devoted to study different aspects of Delay/Disruption Tolerant Network (DTN) such as routing, forwarding, scheduling, planning, and others. 

The simulator is implemented on the [Omnet++ framework version 6.x](https://omnetpp.org/) and can be conveniently utilized using the Qtenv environment and modified or extended using the Omnet++ Eclipse-based IDE. Check [this video in youtube](https://youtu.be/_5HhfNULjtk) for a quick overview of the tool.

The simulator is still under development: this is a beta version. Nonetheless, feel free to use it and test it. Our contact information is below. 

## Installation and build##

1. Directly from Omnet++
* Download [Omnet++](https://omnetpp.org/omnetpp). DTNSIM was tested on version 5.5.1.
* Import the DTNSIM repository from the Omnet++ IDE (File->Import->Projects from Git->Clone URI).
* Build DTNSIM project.

* Open dtnsim_demo.ini config file from the use cases folder.
* Run the simulation. If using Omnet++ IDE, the Qtenv environment will be started. 
* Results (scalar and vectorial statistics) will be stored in the results' folder.

Note: Nodes will remain static in the simulation visualization. Indeed, the dynamic of the network is captured in the "contact plans" which comprises a list of all time-bound communication opportunities between nodes. In other words, if simulating mobile network, the mobility should be captured in such contact plans and provided to DTNSIM as input.

2. Using command line
* Move to `external/lib` and pull changes from the submodule:
```sh
git pull
```
* Add to you PATH envar the path to omnetpp/bin. For example: `export PATH=/home/user/omnetpp-6.1/bin:$PATH`
* Install inet:
    * cd omnetpp-6.1/samples
    * git clone https://github.com/inet-framework/inet.git
    * cd inet
    * make makefiles
    * make 
* Add the envar INET_ROOT: For example `export INET_ROOT=/home/user/ometpp-6.1/samples/inet`
* Add the envar LD_LIBRARY_PATH. For example: `export LD_LIBRARY_PATH=$INET_ROOT/out/gcc-release/src:$LD_LIBRARY_PATH`
* Add the envar NEDPATH to include inet ned files. For example: `export NEDPATH=/home/user/dtnsim/dtnsim/src:$INET_ROOT/src`
* `cd` to /dtnsim
* Run `./compile.sh` to build the project (antop lib is built too).

## Running a simulation ##
Go to omnetpp IDE, add dtnsim project, open any of the use cases ini files from /simulations.
Finally right click on the .ini: Run->Run As->Omnet++ Simulation.

## ION Support ##

Interplanetary Overlay Network (ION) flight code is supported in the support-ion branch. Currently, ION 3.5.0 Contact Graph Routing library is supported by DTNSIM.

## Contact Us ##

If you have any comment, suggestion, or contribution you can reach us at madoerypablo@gmail.com and juanfraire@gmail.com.