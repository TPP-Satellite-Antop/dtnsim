import json
import sys
import os
import glob
import matplotlib.pyplot as plt

BASE = "../../simulations/experiment_results"
OUT = "plots"

def load_result(path):
    with open(path, "r") as f:
        data = json.load(f)
    return {
        "name": os.path.splitext(os.path.basename(path))[0],
        "avgElapsedTime": data.get("avgElapsedTime", None),
        "avgArrivalTime": data.get("avgArrivalTime", None),
        "avgNumberOfHops": data.get("avgNumberOfHops", None)
    }


def load_scenarios(algorithm, faults):
    folder = os.path.join(BASE, algorithm, f"{faults}-faults")
    pattern = os.path.join(folder, "walker-*.json")

    scenarios = []
    for path in glob.glob(pattern):
        scenarios.append(load_result(path))

    return scenarios


def paired_bar_plot(metric, antop_scenarios, cgr_scenarios, ylabel, faults):
    names = [s["name"] for s in antop_scenarios]

    antop_values = [s[metric] for s in antop_scenarios]
    cgr_values = [s[metric] for s in cgr_scenarios]

    x = range(len(names))

    plt.figure()
    plt.title(f"{metric} — {faults}-faults")
    plt.ylabel(ylabel)

    offset = 0.35

    plt.bar([i - offset/2 for i in x], antop_values, width=offset, label="ANTOP", color="tab:blue")
    plt.bar([i + offset/2 for i in x], cgr_values,   width=offset, label="CGR",   color="tab:orange")

    plt.xticks(list(x), names, rotation=45, ha="right")
    plt.grid(axis="y")
    plt.legend()
    plt.tight_layout()

    os.makedirs(OUT, exist_ok=True)
    out_path = os.path.join(OUT, f"{metric}_{faults}-faults.png")
    plt.savefig(out_path, dpi=300)
    plt.close()

    print(f"Saved {out_path}")


def main():
    if len(sys.argv) != 2:
        print("Usage: python plot_scenarios.py <faults_number>")
        sys.exit(1)

    faults = sys.argv[1]

    antop_scenarios = load_scenarios("antop", faults)
    cgr_scenarios = load_scenarios("cgr", faults)

    print(f"Loaded {len(antop_scenarios)} ANTOP scenarios and {len(cgr_scenarios)} CGR scenarios for {faults} faults.")

    antop_scenarios = sorted(antop_scenarios, key=lambda s: s["name"])
    cgr_scenarios = sorted(cgr_scenarios, key=lambda s: s["name"])

    paired_bar_plot("avgElapsedTime", antop_scenarios, cgr_scenarios, "Time", faults)
    paired_bar_plot("avgArrivalTime", antop_scenarios, cgr_scenarios, "Time", faults)
    paired_bar_plot("avgNumberOfHops", antop_scenarios, cgr_scenarios, "Hops", faults)

if __name__ == "__main__":
    main()
