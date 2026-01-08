#!/usr/bin/env python3
import random

def generate_ini(num_sats, sat_per_plane, num_planes, phaseOffset=0, faultOn=False, faultMeanTTF=0, faultMeanTTR=0, failure_pct=0):
    suffix = f"-{failure_pct}-faults" if faultOn else ""
    output_file = f"antop/final/antop-{num_sats}-sats{suffix}.ini"

    with open(output_file, "w") as f:
        f.write(f"""[General]
network = src.dtnsim										
repeat = 1
allow-object-stealing-on-deletion = true

# Simulation end time
sim-time-limit = 84600s

# Nodes quantity (identifiers (EiDs) matches their index, EiD=0 is ignored)			
dtnsim.nodesNumber = {num_sats}
dtnsim.node[*].icon = "satellite"

# Nodes's failure rates
dtnsim.node[*].fault.enable = {str(faultOn).lower()}
dtnsim.node[*].fault.meanTTF = {faultMeanTTF}s
dtnsim.node[*].fault.meanTTR = {faultMeanTTR}s

# Mobility
dtnsim.node[*].hasMobility = true
dtnsim.node[*].noradModule.satIndex = parentIndex()
dtnsim.node[*].noradModule.satName = "sat"
**.updateInterval = 20s
**.numOfSats = {num_sats}
**.satPerPlane = {sat_per_plane}
**.altitude = 540
**.planes = {num_planes}
**.phaseOffset = {phaseOffset}

dtnsim.node[*].dtn.routing = "antop"
dtnsim.node[*].dtn.printRoutingDebug = true

# --- Random Traffic Configuration ---
""")
        MAX_START_TIME = 20   # seconds

        for src in range(1, num_sats + 1):
            num_flows = 100

            bundles_vec = []
            start_vec = []
            dest_vec = []
            size_vec = []

            for _ in range(num_flows):
                dest = random.randint(1, num_sats)
                while dest == src:
                    dest = random.randint(1, num_sats)

                start_time = random.randint(1, MAX_START_TIME)
                size = random.choice([50, 100, 200, 500])

                bundles_vec.append("1")          # 1 bundle por flow
                start_vec.append(str(start_time))
                dest_vec.append(str(dest))
                size_vec.append(str(size))

            f.write(
f"""# Node {src} random traffic
dtnsim.node[{src}].app.enable = true
dtnsim.node[{src}].app.bundlesNumber = "{",".join(bundles_vec)}"
dtnsim.node[{src}].app.start = "{",".join(start_vec)}"
dtnsim.node[{src}].app.destinationEid = "{",".join(dest_vec)}"
dtnsim.node[{src}].app.size = "{",".join(size_vec)}"
        """)


        f.write("""
# Submodule types
dtnsim.node[*].com.typename = "Com"
dtnsim.node[*].dtn.typename = "ContactlessDtn"
dtnsim.central.typename = "ContactlessCentral"
""")

        f.write(f"""
#Metrics
dtnsim.central.collectorPath = "../../experiment_results/antop/{failure_pct}-faults/walker-53x{num_sats}x{num_planes}x{phaseOffset}.json"
""")

    print(f"Generated: {output_file}")


if __name__ == "__main__":
    num_planes = int(input("Enter number of planes: "))
    num_sat_per_plane = int(input("Enter number of satellites per plane: "))
    num_sats = num_planes * num_sat_per_plane
    phaseOffset = int(input("Enter phase offset between planes (must be equal or less than the number of planes): "))

    failureOn = input("Enable node failures? (y/n): ").strip().lower() == 'y'

    faultMeanTTF = 0
    faultMeanTTR = 0

    if failureOn:
        print("Select failure percentage:")
        print("  5  -> 5%")
        print(" 10  -> 10%")
        print(" 20  -> 20%")

        failure_pct = int(input("Enter failure percentage (5/10/20): "))

        faultMeanTTR = 10

        if failure_pct == 5:
            faultMeanTTF = 190
        elif failure_pct == 10:
            faultMeanTTF = 90
        elif failure_pct == 20:
            faultMeanTTF = 40
        else:
            raise ValueError("Invalid failure percentage. Use 5, 10, or 20.")

    print(f"Generating configuration for {num_sats} satellites...")

    generate_ini(
        num_sats,
        num_sat_per_plane,
        num_planes,
        phaseOffset,
        failureOn,
        faultMeanTTF,
        faultMeanTTR,
        failure_pct
    )
