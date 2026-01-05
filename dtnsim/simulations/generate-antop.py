#!/usr/bin/env python3
import random

def generate_ini(num_sats, sat_per_plane, num_planes, faultOn=False, faultMeanTTF=0, faultMeanTTR=0):
    suffix = "-faults" if faultOn else ""
    output_file = f"antop/antop-{num_sats}-sats{suffix}.ini"

    with open(output_file, "w") as f:

        f.write(f"""[General]
network = src.dtnsim										
repeat = 1
allow-object-stealing-on-deletion = true

# Simulation end time
sim-time-limit = 48h

# Nodes quantity (identifiers (EiDs) matches their index, EiD=0 is ignored)			
dtnsim.nodesNumber = {num_sats}
dtnsim.node[*].icon = "satellite"

# Nodes's failure rates
dtnsim.node[*].fault.enable = {str(faultOn).lower()}
dtnsim.node[*].fault.meanTTF = {faultMeanTTF}s
dtnsim.node[*].fault.meanTTR = {faultMeanTTR}s

# Mobility
dtnsim.node[*].hasMobility = true
dtnsim.node[*].norad.satIndex = parentIndex()
dtnsim.node[*].norad.satName = "sat"
**.updateInterval = 20s
**.numOfSats = {num_sats}
**.satPerPlane = {sat_per_plane}
**.altitude = 540
**.planes = {num_planes}

dtnsim.node[*].dtn.routing = "antop"
dtnsim.node[*].dtn.printRoutingDebug = true

# --- Random Traffic Configuration ---
""")
        MIN_BUNDLES = 1
        MAX_BUNDLES = 5
        MAX_START_TIME = 20   # seconds

        for src in range(1, num_sats + 1):
            num_bundles = random.randint(MIN_BUNDLES, MAX_BUNDLES)

            for _ in range(num_bundles):
                dest = random.randint(1, num_sats)
                while dest == src:
                    dest = random.randint(1, num_sats)

                start_time = random.randint(1, MAX_START_TIME)
                size = random.choice([50, 100, 200, 500])

                f.write(f"""# Node {src} -> Node {dest}
dtnsim.node[{src}].app.enable = true
dtnsim.node[{src}].app.bundlesNumber = "1"
dtnsim.node[{src}].app.start = "{start_time}"
dtnsim.node[{src}].app.destinationEid = "{dest}"
dtnsim.node[{src}].app.size = "{size}"
""")

        f.write("""
# Submodule types
dtnsim.node[*].com.typename = "Com"
dtnsim.node[*].dtn.typename = "ContactlessDtn"
dtnsim.central.typename = "ContactlessCentral"
""")

        f.write(f"""
#Metrics
dtnsim.central.collectorPath = "../../experiment_results"
""")

    print(f"Generated: {output_file}")


if __name__ == "__main__":
    num_planes = int(input("Enter number of planes: "))
    num_sat_per_plane = int(input("Enter number of satellites per plane: "))
    num_sats = num_planes * num_sat_per_plane
    failureOn = input("Enable node failures? (y/n): ").strip().lower() == 'y'
    faultMeanTTF = 0
    faultMeanTTR = 0
    if failureOn:
        faultMeanTTF = int(input("Enter mean Time To Failure (in seconds): "))
        faultMeanTTR = int(input("Enter mean Time To Repair (in seconds): "))
    print(f"Generating configuration for {num_sats} satellites...")

    generate_ini(num_sats, num_sat_per_plane, num_planes, failureOn, faultMeanTTF, faultMeanTTR)
