import json
import sys
import os
import matplotlib.pyplot as plt


def load_result(path):
    with open(path, "r") as f:
        data = json.load(f)
    return {
        "name": os.path.splitext(os.path.basename(path))[0],
        "avgElapsedTime": data.get("avgElapsedTime", None),
        "avgArrivalTime": data.get("avgArrivalTime", None),
        "avgNumberOfHops": data.get("avgNumberOfHops", None)
    }


def bar_plot(metric_name, scenarios, ylabel):
    plt.figure()
    plt.title(metric_name)
    plt.ylabel(ylabel)

    labels = [s["name"] for s in scenarios]
    values = [s[metric_name] for s in scenarios]

    plt.bar(labels, values)
    plt.xticks(rotation=45, ha="right")
    plt.tight_layout()
    plt.grid(axis="y")


def main():
    if len(sys.argv) < 2:
        print("Usage: python plot_scenarios.py <file1.json> <file2.json> ...")
        sys.exit(1)

    files = sys.argv[1:]

    scenarios = [load_result(f) for f in files]

    bar_plot("avgElapsedTime", scenarios, "Time")
    bar_plot("avgArrivalTime", scenarios, "Time")
    bar_plot("avgNumberOfHops", scenarios, "Hops")

    plt.show()


if __name__ == "__main__":
    main()
