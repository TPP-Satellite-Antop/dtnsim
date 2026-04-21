import json
import sys
import os
import glob
import matplotlib.pyplot as plt
import re

plt.rcParams.update({
    "font.size": 18,
    "axes.titlesize": 16,
    "axes.labelsize": 16,
    "xtick.labelsize": 16,
    "ytick.labelsize": 16,
    "legend.fontsize": 14
})

COLORS = {
    "ANTOP": "#66c2a5",
    "CGR": "#fc8d62",
    "CGR-ONE": "#8da0cb"
}

BASE = "../../experiment_results"
OUT = "./plots"

def pretty_metric_name(name: str) -> str:
    words = re.sub(r'([a-z])([A-Z])', r'\1 \2', name)

    replacements = {
        "avg": "average",
        "num": "number",
    }

    tokens = words.split()
    tokens = [replacements.get(t.lower(), t.lower()) for t in tokens]

    return tokens[0].capitalize() + " " + " ".join(tokens[1:])

def satellites_from_name(name):
    # name example: walker-53x120x12x7
    try:
        params = name.split("-")[1]
        inclination, satellites, planes, phase = params.split("x")
        return int(satellites)
    except Exception as e:
        raise ValueError(f"Cannot parse satellites from scenario name: {name}")

def short_scenario_name(name):
    try:
        params = name.split("-")[1]
        parts = params.split("x")
        return "x".join(parts[1:])
    except:
        return name

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

def delivery_ratio_plot(antop, cgr, cgr_one, faults):
    def map_ratio(scenarios):
        result = {}
        for s in scenarios:
            name = s["name"]
            expected = satellites_from_name(name) * 100
            arrived = len(s.get("bundles", []))
            result[name] = 100.0 * arrived / expected if expected > 0 else 0
        return result

    antop_map = map_ratio(antop)
    cgr_map = map_ratio(cgr)
    cgr_one_map = map_ratio(cgr_one)

    all_names = sorted(
        set(antop_map.keys()) |
        set(cgr_map.keys()) |
        set(cgr_one_map.keys())
    )

    xscenarios = [short_scenario_name(n) for n in all_names]

    x = list(range(len(all_names)))

    def values(map_):
        return [map_.get(n, None) for n in all_names]

    plt.figure()
    plt.title(f"Delivery ratio — {faults}% faults")
    plt.ylabel("Arrival (%)")
    plt.xlabel("Scenarios")
    plt.ylim(90, 101.5)
    ticks = plt.yticks()[0]
    ticks = [t for t in ticks if 90 < t <= 100]

    plt.yticks(ticks)

    plt.plot(x, values(antop_map), marker="o", label="ANTop", color=COLORS["ANTOP"], markersize=12)
    plt.plot(x, values(cgr_map), marker="s", label="CGR (per-neighbor)", color=COLORS["CGR"], fillstyle="none", markersize=12)
    plt.plot(x, values(cgr_one_map), marker="^", label="CGR (one-best)", color=COLORS["CGR-ONE"], markersize=12)

    plt.xticks(x, xscenarios)
    plt.grid(axis="y", linestyle="--", alpha=0.6)
    plt.legend()
    plt.tight_layout()

    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, f"delivery_ratio_{faults}-faults.pdf")
    plt.savefig(path, bbox_inches="tight")
    plt.close()

    print(f"Saved {path}")

def triple_boxplot(metric, antop, cgr, cgr_one, ylabel, faults):
    def map_values(scenarios):
        result = {}
        for s in scenarios:
            result[s["name"]] = [b[metric] for b in s.get("bundles", [])]
        return result

    antop_map = map_values(antop)
    cgr_map = map_values(cgr)
    cgr_one_map = map_values(cgr_one)

    all_names = sorted(
        set(antop_map.keys()) |
        set(cgr_map.keys()) |
        set(cgr_one_map.keys())
    )

    xscenarios = [short_scenario_name(n) for n in all_names]

    data = []
    positions = []
    colors = []

    offset = 0.25

    protocols = [
        ("ANTOP", antop_map, -offset),
        ("CGR-ONE", cgr_one_map, 0.0),
        ("CGR", cgr_map, +offset),
    ]

    for i, name in enumerate(all_names):
        for proto_name, proto_map, pos_offset in protocols:
            if name in proto_map:
                data.append(proto_map[name])
                positions.append(i + pos_offset)
                colors.append(COLORS[proto_name])

    plt.figure(figsize=(max(8, len(all_names)), 6))
    bp = plt.boxplot(data, positions=positions, widths=0.2, patch_artist=True)
    plt.yscale("log")

    for box, c in zip(bp["boxes"], colors):
        box.set_facecolor(c)
        box.set_edgecolor("black")

    for median in bp["medians"]:
        median.set_color("black")
        median.set_linewidth(2)

    plt.xticks(range(len(all_names)), xscenarios)
    plt.suptitle(f"{pretty_metric_name(metric)} — {faults}% faults", y=0.90)
    plt.ylabel(ylabel)
    plt.xlabel("Scenarios")
    plt.xlim(-0.5, len(all_names) - 0.5)
    plt.grid(axis="y", linestyle="--", alpha=0.6)

    plt.legend(
        handles=[
            plt.Line2D([0], [0], color=COLORS["ANTOP"], lw=6, label="ANTop"),
            plt.Line2D([0], [0], color=COLORS["CGR-ONE"], lw=6, label="CGR (one-best)"),
            plt.Line2D([0], [0], color=COLORS["CGR"], lw=6, label="CGR (per-neighbor)"),
        ],
        loc="upper center",
        bbox_to_anchor=(0.5, 1.15),
        ncol=3,
        frameon=False
    )

    plt.tight_layout(rect=[0, 0, 1, 0.985])
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, f"boxplot_{metric}_{faults}-faults.pdf")
    plt.savefig(path, bbox_inches="tight")
    plt.close()

    print(f"Saved {path}")

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

    antop = to_ms(sorted(load_scenarios("antop", faults), key=lambda s: s["name"]))
    cgr = to_ms(sorted(load_scenarios("cgr", faults), key=lambda s: s["name"]))
    cgr_one = to_ms(sorted(load_scenarios("cgr-one", faults), key=lambda s: s["name"]))

    delivery_ratio_plot(antop, cgr, cgr_one, faults)

    triple_boxplot("elapsedTime", antop, cgr, cgr_one, "Time (ms)", faults)
    triple_boxplot("arrivalTime", antop, cgr, cgr_one,"Time (ms)", faults)
    triple_boxplot("numberOfHops", antop, cgr, cgr_one, "Hops", faults)


if __name__ == "__main__":
    main()
