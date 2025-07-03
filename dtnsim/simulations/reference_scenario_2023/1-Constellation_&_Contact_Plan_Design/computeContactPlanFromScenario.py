# STK library imports
from agi.stk12.stkengine import STKEngine
from agi.stk12.stkdesktop import STKDesktop
from agi.stk12.stkobjects import *
from agi.stk12.stkutil import *
from agi.stk12.vgt import *
# Imports
import json

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


# Constants
# Objects name includes 
GS_NAME_INCLUDES = "GroundStation"
EN_NAME_INCLUDES = "EndNode"
ISL_NAME_INCLUDES = "ISL"
GSL_NAME_INCLUDES = "GSL"
DATA_RATE = 1000 #B/s : 1000 = 1kb/s

# Set the unit so that a date will be displayed as an epoch second
root.UnitPreferences.SetCurrentUnit('DateFormat','EpSec')

# Get all the statallites:
satellites = root.CurrentScenario.Children.GetElements(AgESTKObjectType.eSatellite)
# Get all the facilities (GS + EN)
facilities = root.CurrentScenario.Children.GetElements(AgESTKObjectType.eFacility)

# Compute total number of computed access
nbSat = satellites.Count
nbFacilities = facilities.Count
print("{} satellites, {} facilities".format(nbSat, nbFacilities))
nbTotalAccess = nbSat * (nbSat-1) * 4 + nbSat * nbFacilities
nbComputedAcess = 0


print("Saving scenario informations to json files")
# We also represent the GS and EndNodes in a GEOJson format, as specified here: https://datatracker.ietf.org/doc/html/rfc7946
GEOJsonData = {
    "type" : "FeatureCollection",
    "features" : []
}
# Storing the simulation informations in a json 
ExtractedData = {
    "Satellites" : [],
    "GroundStations": [],
    "EndNodes" : [],
}
# We save :
# Satellites sensors and orbit informations
# Ground stations Informations
for i in range(nbSat):
    # Gather the satellite informations
    satellite = AgSatellite(satellites.Item(i))
    keplerian = AgOrbitStateClassical(satellite.Propagator.InitialState.Representation.ConvertTo(AgEOrbitStateType.eOrbitStateClassical))
    ApogeeAltitude = keplerian.SizeShape.ApogeeAltitude
    PerigeeAltitude = keplerian.SizeShape.PerigeeAltitude
    Inclinaison = keplerian.Orientation.Inclination
    ArgOfPerigee = keplerian.Orientation.ArgOfPerigee
    TrueAnomaly = keplerian.Location.Value
    Raan = keplerian.Orientation.AscNode.Value
    satelliteInformation = {
        "ApogeeAltitude" : ApogeeAltitude,
        "PerigeeAltitude" : PerigeeAltitude,
        "Inclinaison" : Inclinaison,
        "ArgumentOfPerigee": ArgOfPerigee,
        "TrueAnomaly":TrueAnomaly,
        "RAAN" : Raan,
        "Sensors" : []
    }
    # Get Sensor Informations
    sensors = satellite.Children.GetElements(AgESTKObjectType.eSensor)
    for j in range(sensors.Count):
        sensor = AgSensor(satellite.Children.Item(j))
        sensorInfo = {
            "Name" : sensor.InstanceName
        }

        if sensor.PointingType == AgESnPointing.eSnPtFixed:
            Pointing = AgSnPtFixed(sensor.Pointing)
            AzimuthElevation = Pointing.Orientation.QueryAzElArray()
            sensorInfo["Pointing"] = {
                "Azimuth": AzimuthElevation[0],
                "Elevation": AzimuthElevation[1]
            }
        if sensor.PatternType == AgESnPattern.eSnSimpleConic:
            ConeAngle = AgSnSimpleConicPattern(sensor.Pattern).ConeAngle
            sensorInfo["ConicAngle"] = ConeAngle
        if sensor.AccessConstraints.IsConstraintActive(AgEAccessConstraints.eCstrRange):
            minRange = sensor.AccessConstraints.GetActiveConstraint(AgEAccessConstraints.eCstrRange).Min
            maxRange = sensor.AccessConstraints.GetActiveConstraint(AgEAccessConstraints.eCstrRange).Max
            sensorInfo["Range"] = {
                "Min": minRange,
                "Max": maxRange
            }
        satelliteInformation["Sensors"].append(sensorInfo)
    ExtractedData["Satellites"].append(satelliteInformation)

# Get informations on the Ground stations and end nodes
for i in range(nbFacilities):
    facility = facilities.Item(i)
    position = facility.Position.QuerySphericalArray() # Returns Latitude (index 0),Longitude (index 1) and Radius(index 2)
    facilityInfo = {
        "Name": facility.InstanceName,
        "Position" : {
            "Latitude": position[0],
            "Longitude": position[1],
            "Radius": position[2]
        }
    }
    GEOJsonFeature = {
        "type":"Feature",
        "geometry": {
            "type":"Point",
            "coordinates":[position[1],position[0]] # [Longitude,Latitude]
        },
        "properties": {
            "name": facility.InstanceName,
        }
    }
    if any([gsName in facility.InstanceName for gsName in ["GS",GS_NAME_INCLUDES]]):
        # Add to the Standard Json Object
        ExtractedData["GroundStations"].append(facilityInfo)
        # Add to the GEOJson Object
        GEOJsonFeature["properties"]["facilityType"] = "Ground Station"
        GEOJsonData["features"].append(GEOJsonFeature)
    
    else:
        # Add to the Standard Json File
        ExtractedData["EndNodes"].append(facilityInfo)
        # Add to the GEOJson Object
        GEOJsonFeature["properties"]["facilityType"] = "End Node"
        GEOJsonData["features"].append(GEOJsonFeature)

# Write the json data to a file
infoJsonFile = open("{}-contact-plan-info.json".format(root.CurrentScenario.InstanceName), 'w')
infoJsonFile.write(json.dumps(ExtractedData,indent=4))
infoJsonFile.close()

GEOJsonFile = open("{}-GEO.json".format(root.CurrentScenario.InstanceName), 'w')
GEOJsonFile.write(json.dumps(GEOJsonData,indent=4))
GEOJsonFile.close()


# Contact plan files
contactPlanFileFloat = open("{}-float-contact-plan.txt".format(root.CurrentScenario.InstanceName), 'w')
contactPlanFileInt = open("{}-int-contact-plan.txt".format(root.CurrentScenario.InstanceName), 'w')


# We need to calculate acces from all of the satellite sensors to all the other satellites and ground stations
print("Starting access computations")
# Update the display of % completion
print("Progress : {percentage}%".format(percentage = (nbComputedAcess/nbTotalAccess)*100.0),end='\r')

for i in range(satellites.Count):

    satellite = satellites.Item(i)
    satelliteName = satellite.InstanceName
    sensors = satellite.Children.GetElements(AgESTKObjectType.eSensor)

    for sensor in sensors:
        # if we have a GSL, compute the acces between the GSL and all the ground stations and endNodes
        if GSL_NAME_INCLUDES in sensor.InstanceName:
            for j in range(facilities.Count):
                targetFacility = facilities.Item(j)
                # Index the facility ID after the satellites
                facilityID = satellites.Count + j
                src,dest = 0,0
                # Change the source and dest on weather the facility is an End node or a Ground Station.
                if any([gsName in facility.InstanceName for gsName in ["GS",GS_NAME_INCLUDES]]):
                    # sat -> ground station
                    src = i+1
                    dest = facilityID+1
                if EN_NAME_INCLUDES in targetFacility.InstanceName:
                    # end node -> sat
                    src = facilityID+1
                    dest = i+1
                access = sensor.GetAccessToObject(targetFacility)
                access.ComputeAccess()
                nbComputedAcess += 1

                intervalCount = access.ComputedAccessIntervalTimes.Count
                if intervalCount == 0:
                    continue
                for k in range(intervalCount):
                    interval = access.ComputedAccessIntervalTimes.GetInterval(k)

                    contactPlanFileFloat.write('a contact +{startTime} +{endTime} {src} {dest} {dataRate}\n'.format(
                        src=src,
                        dest=dest,
                        startTime = interval[0],
                        endTime = interval[1],
                        dataRate = DATA_RATE
                    ))

                    contactPlanFileInt.write('a contact +{startTime} +{endTime} {src} {dest} {dataRate}\n'.format(
                        src=src,
                        dest=dest,
                        startTime = round(interval[0]),
                        endTime = round(interval[1]),
                        dataRate = DATA_RATE
                    ))
        else :
            # If the sensor is not GLS, aka, it's ISL, 
            for j in range(satellites.Count):
                targetSatellite = satellites.Item(j)
                # Skip if targetSatellite is current satellie
                if targetSatellite.InstanceName == satelliteName:
                    continue
                # Compute the acces between the current satellite and the target satellite sensor
                access = sensor.GetAccessToObject(targetSatellite)
                access.ComputeAccess()
                nbComputedAcess += 1

                intervalCount = access.ComputedAccessIntervalTimes.Count
                if intervalCount == 0:
                    continue
                for k in range(intervalCount):
                    interval = access.ComputedAccessIntervalTimes.GetInterval(k)
                    contactPlanFileFloat.write('a contact +{startTime} +{endTime} {src} {dest} {dataRate}\n'.format(
                        src=i+1,
                        dest=j+1,
                        startTime = interval[0],
                        endTime = interval[1],
                        dataRate = DATA_RATE
                    ))

                    contactPlanFileInt.write('a contact +{startTime} +{endTime} {src} {dest} {dataRate}\n'.format(
                        src=i+1,
                        dest=j+1,
                        startTime = round(interval[0]),
                        endTime = round(interval[1]),
                        dataRate = DATA_RATE
                        ))
            # Update the display of % completion
            print("Progress : {percentage:.1f}%".format(percentage = (nbComputedAcess/nbTotalAccess)*100.0),end='\r')

contactPlanFileFloat.close()
contactPlanFileInt.close()
print("\nDone!")