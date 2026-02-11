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

    data["name"] = os.path.splitext(os.path.basename(path))[0]
    return data


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
    plt.title(f"{metric} — {faults}% faults")
    plt.ylabel(ylabel)

    offset = 0.35

    plt.bar([i - offset/2 for i in x], antop_values, width=offset,
            label="ANTOP", color="tab:blue")
    plt.bar([i + offset/2 for i in x], cgr_values, width=offset,
            label="CGR", color="tab:orange")

    plt.yscale("log")

    plt.xticks(list(x), names, rotation=45, ha="right")
    plt.grid(axis="y", which="both", linestyle="--", alpha=0.6)
    plt.legend()
    plt.tight_layout()

    os.makedirs(OUT, exist_ok=True)
    out_path = os.path.join(OUT, f"{metric}_{faults}-faults.png")
    plt.savefig(out_path, dpi=300)
    plt.close()

    print(f"Saved {out_path}")

def paired_boxplot(metric, antop_scenarios, cgr_scenarios, ylabel, faults):

    def map_name_to_values(scenarios):
        result = {}
        for s in scenarios:
            name = s["name"]
            vals = []
            for b in s["bundles"]:
                vals.append(b[metric])
            result[name] = vals
        return result

    antop_map = map_name_to_values(antop_scenarios)
    cgr_map = map_name_to_values(cgr_scenarios)

    # common scenario names (intersection)
    names = sorted(set(antop_map.keys()) & set(cgr_map.keys()))

    # build data in paired order
    data = []
    labels = []
    colors = []

    for name in names:
        data.append(antop_map[name])
        labels.append(f"{name}\nANTOP")
        colors.append("#4CAF50")  # green

        data.append(cgr_map[name])
        labels.append(f"{name}\nCGR")
        colors.append("#2196F3")  # blue

    plt.figure(figsize=(max(8, len(names)), 6))

    bp = plt.boxplot(
        data,
        labels=labels,
        patch_artist=True
    )

    # apply colors
    for box, c in zip(bp["boxes"], colors):
        box.set_facecolor(c)
        box.set_edgecolor("black")

    # median emphasis
    for med in bp["medians"]:
        med.set_color("black")
        med.set_linewidth(2)

    plt.title(f"{metric} — {faults}% faults")
    plt.ylabel(ylabel)
    plt.grid(axis="y")
    plt.xticks(rotation=45, ha="right")
    plt.tight_layout()

    os.makedirs(OUT, exist_ok=True)
    out_path = os.path.join(OUT, f"boxplot_{metric}_{faults}-faults.png")
    plt.savefig(out_path, dpi=300)
    plt.close()

    print(f"Saved {out_path}")

def to_ms(scenarios):
    for s in scenarios:
        if "avgElapsedTime" in s:
            s["avgElapsedTime"] *= 1000
        if "avgArrivalTime" in s:
            s["avgArrivalTime"] *= 1000

        if "bundles" in s:
            for b in s["bundles"]:
                if "elapsedTime" in b:
                    b["elapsedTime"] *= 1000
                if "arrivalTime" in b:
                    b["arrivalTime"] *= 1000

    return scenarios


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 jsonPlots.py <faults_number>")
        sys.exit(1)

    faults = sys.argv[1]

    antop_scenarios = load_scenarios("antop", faults)
    cgr_scenarios = load_scenarios("cgr", faults)

    antop_scenarios = sorted(antop_scenarios, key=lambda s: s["name"])
    cgr_scenarios = sorted(cgr_scenarios, key=lambda s: s["name"])

    antop_scenarios = to_ms(antop_scenarios)
    cgr_scenarios = to_ms(cgr_scenarios)
    paired_bar_plot("avgElapsedTime", antop_scenarios, cgr_scenarios, "Time (ms)", faults)
    paired_bar_plot("avgArrivalTime", antop_scenarios, cgr_scenarios, "Time (ms)", faults)
    paired_bar_plot("avgNumberOfHops", antop_scenarios, cgr_scenarios, "Hops", faults)
    
    paired_boxplot("elapsedTime", antop_scenarios, cgr_scenarios, "Time (ms)", faults)
    paired_boxplot("arrivalTime", antop_scenarios, cgr_scenarios, "Time (ms)", faults)
    paired_boxplot("numberOfHops", antop_scenarios, cgr_scenarios, "Hops", faults)


if __name__ == "__main__":
    main()
