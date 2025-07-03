# Constellation & Contact Plan design

The reference scenario were done using STK-12, and explanations about how they were made can me found in the [how-to](#stk-12-how-to) section.

Contact plan generation is done with python and STK, in the [Generating the contact plan](#generating-the-contact-plan) section.

Some general rules on scenario design :

### Ground objects and poles

When randomly distributing objects on the surface of a sphere (in our case, the earth), you need to properly consider the distribution to not end up with a higher object density at the poles, which can skew your plan to have more or less contact than normal depending on the constellation. Some examples and solutions [here](https://mathworld.wolfram.com/SpherePointPicking.html).

As a general rule of thumb, a ground object will see more satellites on average the closer it is to the poles.

### Satellite contacts

When designing your constellation, you want to make sure that you don't have isolated orbits (or groups of isolated orbits) which will increase the time packets spend in the network.

# Generating the contact plan

## Installation

Before you can use the provided scripts, you need to have STK with at least one of theses licenses :
- STK Pro
- STK Premium (Air)
- STK Premium (Space)
- STK Enterprise

To add the STK libraries to your python environment, run :
```
pip install "C:\Program Files\AGI\STK 12\bin\AgPythonAPI\agi.stk<..ver..>-py3-none-any.whl" 
```
change <..ver..> to your installed STK version

Specific extra steps or installation procedures will be described atop the scripts if needed.

## Workflow

You can compute contacts between STK objects using STK's [API](https://help.agi.com/stkdevkit/index.htm)

Here is the general workflow we used:

- Create the constellation directly within STK, described in the [constellation generation](#constellation-generation-with-stk-12) section
- Populate the STK scenario with end nodes and ground stations using a Python script interfaced with the STK API
- Compute the contacts using another Python script, save them in the format that suits dtnsim, alongside a json file containing all of the relevant informations of the scenario

## Populate with end node and ground stations
This is done with [populateWithAndNodeAndGroundStations.py](populateWithEndNodeAndGroundStations.py)

You can modify the following constants :
- `EN_COUNT` : Number of added EndNodes
- `GS_COUNT` : Number of added GroundStations

If you want to import facilities from a `csv` file, you can use :
- `FACILITIES_FILENAME` : Name of the file containing facilities
- `OVERRIDE_GS_NAME` : if true, adds `'GroundStation_'` in front of the facility name provided in the csv file

Here is the expected csv file format:

|GS-Name|Latitude|Longitude|Radius|
|-|-|-|-|

Before you run the file, make sure you have only one instance of STK running, with the scenario you want to modify in it.

## Compute contact plan from Scenario

This is done with either 

- ~~[computeContactPlanFromScenario.py](computeContactPlanFromScenario.py)~~. It uses an open STK instance and is slow
- [computeContactPlanFromScenarioFile.py](computeContactPlanFromScenarioFile.py). It requires a scenario filepath and is much faster

Here are the assumptions the script makes on the STK scenario:

* Ground stations and EndNodes are facilities
  * Ground Stations include `GroundStation` or `GS` in their name
  * End Nodes include `EndNode` in their name
* The sensors are childrens of the satellites
  * ISL sensors include `ISL` in their name
  * GSL sensors include `GSL` in their name
  * GSL sensors access both ground stations and end node
* All the facilities and satellites of the scenario are direct children of the scenario root

Before running the script, make sure that you have only one instance of STK running

The script will generate 3 files:

- a text file with the contact plan (e.g. [Star.txt](../2-Constellation_Walker_Star/contactPlan/Star.txt))
- a json file with ground stations and satellite information (e.g. [Star.json](Star.json))
- a geosjon file to check the positions of ground devices including End-Nodes + Ground Stations ([StarFacilitiesAsGeoJSON.json](StarFacilitiesAsGeoJSON.json))
  - <https://geojson.io/> > import the geojson file and look at the positions

# STK-12 How to

Ansys Systems Tool Kit (STK) is a physics-based simulation environment for digital mission engineering. We will be using it to model and propagate satellite orbits.

It is also used to plan and follow space missions, predict communication windows between satellites and earth ground stations, and evaluate satellite systems' and mission tools' performances. 

We will be using it to generate LEO satellite constellations and to predict communication windows, between both satellites and ground stations.

## Constellation generation with STK-12

A constellation is generated from a satellite. To add a satellite into the scenario :

![Untitled](media/Untitled.png)

![Untitled](media/Untitled%201.png)
You can add a satellite from [TLE](https://fr.wikipedia.org/wiki/Param%C3%A8tres_orbitaux_%C3%A0_deux_lignes) (two line element set). It comprises of a set of orbital parameters, which completely defines an orbit.

******************************TLE exemple:******************************

```
1 51087U 22002DH  23158.15562884  .00023454  00000-0  89877-3 0  9995
2 51087  97.4457 225.9328 0008886 272.3249  87.6972 15.26487724 77233
```

Loading the TLE file will add the satellite to the STK database, a window will open, allowing us to add the satellite to the scenario.

![Untitled](media/Untitled%202.png)

![Untitled](media/Untitled%203.png)

To compute the constellation, right click the satellite in the object browser:

![Untitled](media/Untitled%204.png)

![Untitled](media/Untitled%205.png)


The **walker tool** allows generation of walker-type constellations. There are two base types which can be selected in the **type** menue:

- Delta
- Star

Among many things , you can customize :

- The number of planes : **number of planes**
- The number of satellites per plane (or the total number of satellite in the constellation)

The constellation can then be created by clicking on the **create / modify walker** button.


## Satellite customisation

To generate a contact plan, you need to be able to compute contacts. With STK, this can be done rather simply by adding sensors to the satellites, which will then sense when an object is within their sensing volume.

For this exemple, we want to detect ground objects, as well as other satellites in the constellation. We will then add 3 sensors to the satellite :

- 1 pointing towards the ground
- 2 on the plane perpendicular to the satellite's orbit

To add the sensors:

![Untitled](media/Untitled%206.png)

![Untitled](media/Untitled%207.png)

A window will open, in which we can define the sensor's properties.

We are interested in :

- Basic → Definition : Simple conic - cone half angle : shape of the sensor's volume
- Basic → Pointing : Azimuth, elevation : direction in which the sensor will point
- Constraints → basic : Range : setting up the sensor's min and max range

We create 3 sensors:

- GSL :
  - Cone half angle : 60 deg
  - Elevation : -90 deg
- ISL-1
  - Cone half angle : 20 deg
  - Elevation : 0 deg
  - Azimuth : 90 deg
  - Max range : 2000 km
- ISL-2
  - Cone half angle : 20 deg
  - Elevation : 0 deg
  - Azimuth : -90 deg
  - Max range : 2000 km

(To change the sensor name: right click → rename)

![Untitled](media/Untitled%208.png)

![Untitled](media/Untitled%209.png)

From this satellite, we can generate a constellation. And all the satellites within will have the same sensors as the root satellite. This is explained in [Constellation generation with STK-12](#constellation-generation-with-stk-12)

![Walker star with 11 planes and 6 satellites on each plane](media/Untitled%2010.png)
Walker star with 11 planes and 6 satellites on each plane

## STK API

You can interact with STK via an API. The following examples are done in Python , but [other languages](https://www.agi.com/tech-tips/Automating-STK-a-quick-guide#:~:text=You%20can%20use%20Java%2C%20C,languages%20to%20deploy%20your%20project) are supported.

The installation is detailed in AGI's documentation [here](https://help.agi.com/stk/12.5.0/index.htm#training/StartPython.htm)

All the STK API documentation is available [online](https://help.agi.com/stkdevkit/index.htm)

With the API, we have access to the entire object structure of the simulation. You can add & modify you scenario via the API.

In Python, the API is accessible as a library, you need to import it to get started:

```python
# STK library imports
from agi.stk12.stkdesktop import STKDesktop
from agi.stk12.stkobjects import *
from agi.stk12.stkutil import *
from agi.stk12.vgt import *
# Get The application ( woks only if a single instance is running, if multiple instances are running, refer to the docs)
stk = STKDesktop.AttachToApplication()
# Get the object root
root = stk.Root
```
