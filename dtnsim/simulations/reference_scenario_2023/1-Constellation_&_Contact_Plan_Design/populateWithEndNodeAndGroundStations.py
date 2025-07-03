# STK library imports
from agi.stk12.stkdesktop import STKDesktop
from agi.stk12.stkengine import STKEngine
from agi.stk12.stkobjects import *
from agi.stk12.stkutil import *
from agi.stk12.vgt import *
import numpy as np
import pandas as pd



##################### ATTATCH TO DESKTOP APPLICATION ###########################
# # Get The application ( woks only if a single instance is running, if mulitple instances are running, refer to the docs)
# stk = STKDesktop.AttachToApplication()
# # Get the stk object root
# root = stk.Root

######################### LOAD DIRECTLY FROM FILE ##############################
DEFAULT_SCENARIO_PATH = None
# Starting STK
print("Starting STK")
stk = STKEngine.StartApplication(noGraphics=True) # optionally, noGraphics = True
# Load the scenario to the root
print("Loading {}".format(DEFAULT_SCENARIO_PATH))
root = stk.NewObjectRoot()
root.LoadScenario(path=DEFAULT_SCENARIO_PATH)
print("Loaded {} scenario".format(root.CurrentScenario.InstanceName))


# Constants - Change as needed to pupolate your scenario
# Number of EndNodes
EN_COUNT = 0
# number of GroundSations
GS_COUNT = 0

# This is used to import facilities from a filechange csv filename as needed, the CSV file needs to repect the format :
# GS-name,Laltitude,Longitude,Radius
FACILITIES_FILENAME = None
# Override the GS name in the CSV file, to match the expected format of the contact plan generator.
OVERRIDE_GS_NAME = True




# placing them at random points on the sphere:
for i in range(EN_COUNT):
    # calculate random coordinates
    longitude = np.random.uniform(-180,180)
    v = np.random.uniform(0,1)
    latitude = float(np.arccos(2*v -1)*(180/np.pi) - 90)
    # Add them to scenario
    endnode = root.CurrentScenario.Children.New(AgESTKObjectType.eFacility,"EndNode{}".format(i+1))
    # Set the position
    endnode = AgFacility(endnode)
    endnode.Position.AssignGeodetic(latitude,longitude,.0)
    endnode.UseTerrain = True
    endnode.HeightAboveGround = .001 #km

for i in range(GS_COUNT):
    # calculate random coordinates
    longitude = np.random.uniform(-180,180)
    v = np.random.uniform(0,1)
    latitude = float(np.arccos(2*v -1)*(180/np.pi) - 90)
    # Add them to scenario
    groundStation = root.CurrentScenario.Children.New(AgESTKObjectType.eFacility,"GroundStation{}".format(i+1))
    # Set the position
    groundStation = AgFacility(groundStation)
    groundStation.Position.AssignGeodetic(latitude,longitude,.0)
    groundStation.UseTerrain = True
    groundStation.HeightAboveGround = .001 #km*

# Import facilities from filename
if (FACILITIES_FILENAME is not None):
    facilitiesDF = pd.read_csv(FACILITIES_FILENAME,header=0,delimiter=';')
    for index,row in facilitiesDF.iterrows():
        name = row['GS-name']
        if (OVERRIDE_GS_NAME):
            name = "GroundStation_" + row['GS-name']
        groundStation = root.CurrentScenario.Children.New(AgESTKObjectType.eFacility,name)
        groundStation = AgFacility(groundStation)
        groundStation.Position.AssignSpherical(float(row['Latitude']),float(row['Longitude']),float(row['Radius']))

# Save the scenario
root.SaveScenario()
print("Done!")