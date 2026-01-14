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
dtnsim.node[*].norad.satIndex = parentIndex()
dtnsim.node[*].norad.satName = "sat"
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
        MAX_START_TIME = 100   # seconds
        MAX_BUNDLES_PER_TIME = 5
        MAX_DELAY = 10        # seconds after question
        ANSWER_PROB = 0.6     # 60% of questions get answered
        MAX_BUNDLES_PER_NODE = 100

        node_data = {
            i: {
                "bundles": [],
                "start": [],
                "dest": [],
                "size": []
            }
            for i in range(1, num_sats + 1)
        }

        def node_bundle_count(node):
            return len(node_data[node]["bundles"])

        for src in range(1, num_sats + 1):
            for dst in range(src + 1, num_sats + 1):

                # keep creating paired conversational opportunities
                while (
                    node_bundle_count(src) < MAX_BUNDLES_PER_NODE
                    or node_bundle_count(dst) < MAX_BUNDLES_PER_NODE
                ):

                    # stop if both nodes are full
                    if (
                        node_bundle_count(src) >= MAX_BUNDLES_PER_NODE
                        and node_bundle_count(dst) >= MAX_BUNDLES_PER_NODE
                    ):
                        break

                    # ------------ QUESTION src -> dst ------------
                    if node_bundle_count(src) < MAX_BUNDLES_PER_NODE:
                        q_start = random.randint(1, MAX_START_TIME)
                        q_size = random.choice([50, 100, 200, 500])

                        node_data[src]["bundles"].append(str(random.randint(1, MAX_BUNDLES_PER_TIME)))
                        node_data[src]["start"].append(str(q_start))
                        node_data[src]["dest"].append(str(dst))
                        node_data[src]["size"].append(str(q_size))
                    else:
                        # src full, skip making question
                        break

                    # decide whether dst answers
                    will_answer = random.random() < ANSWER_PROB

                    if not will_answer:
                        continue  # unanswered question, move on

                    # ------------ ANSWER dst -> src ------------
                    if node_bundle_count(dst) < MAX_BUNDLES_PER_NODE:
                        min_answer_time = q_start + 1
                        max_answer_time = min(q_start + MAX_DELAY, MAX_START_TIME)

                        if min_answer_time > MAX_START_TIME:
                            min_answer_time = MAX_START_TIME

                        if max_answer_time < min_answer_time:
                            max_answer_time = min_answer_time

                        a_start = random.randint(min_answer_time, max_answer_time)
                        a_size = random.choice([50, 100, 200, 500])

                        node_data[dst]["bundles"].append(str(random.randint(1, MAX_BUNDLES_PER_TIME)))
                        node_data[dst]["start"].append(str(a_start))
                        node_data[dst]["dest"].append(str(src))
                        node_data[dst]["size"].append(str(a_size))
                    # else dst is full → cannot answer, skip

        for src in range(1, num_sats + 1):
            f.write(
f"""# Node {src} random conversational traffic
dtnsim.node[{src}].app.enable = true
dtnsim.node[{src}].app.bundlesNumber = "{','.join(node_data[src]['bundles'])}"
dtnsim.node[{src}].app.start = "{','.join(node_data[src]['start'])}"
dtnsim.node[{src}].app.destinationEid = "{','.join(node_data[src]['dest'])}"
dtnsim.node[{src}].app.size = "{','.join(node_data[src]['size'])}"
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
    failure_pct = 0

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
