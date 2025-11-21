#!/usr/bin/env python3
import random

def generate_ini(num_sats):
    output_file = f"antop/antop-{num_sats}-sats.ini"
    with open(output_file, "w") as f:
        sat_per_plane = random.randint(1, max(1, num_sats // 2))
        inclination = round(random.uniform(30.0, 98.0), 2)
        altitude = random.randint(350, 1200)

        f.write(f"""[General]
network = src.dtnsim										
repeat = 1
allow-object-stealing-on-deletion = true

# Simulation end time
sim-time-limit = 48h

# Nodes quantity (identifiers (EiDs) matches their index, EiD=0 is ignored)			
dtnsim.nodesNumber = {num_sats}
dtnsim.node[1..{num_sats}].icon = "satellite"

# Mobility
dtnsim.node[*].hasMobility = true
dtnsim.node[*].noradModule.satIndex = parentIndex()
dtnsim.node[*].noradModule.satName = "sat"
**.updateInterval = 20s
**.numOfSats = {num_sats}
**.satPerPlane = {sat_per_plane}
**.inclination = {inclination}
**.altitude = {altitude}
**.planes = {max(1, num_sats // sat_per_plane)}

dtnsim.node[*].dtn.routing = "antop"
dtnsim.node[*].dtn.printRoutingDebug = true

# --- Random Traffic Configuration ---
""")

        # Random bundle generation
        # Tweakable params:
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

    print(f"Generated: {output_file}")


if __name__ == "__main__":
    num_sats = int(input("Enter number of satellites: "))
    generate_ini(num_sats)
