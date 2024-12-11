import pandas as pd
import matplotlib.pyplot as plt
import re

# Load the CSV file
file_path = "random_bench_3.csv"  # Replace with your CSV file path
df = pd.read_csv(file_path)

# Extract the number of vertices and density from the path
def extract_info(path):
    match = re.search(r"random_(\d+)_(\d+\.?\d*)", path)
    if match:
        return int(match.group(1)), float(match.group(2))
    return None, None

df["vertices"], df["density"] = zip(*df["path"].apply(extract_info))

# Sort the data by density and number of vertices
df.sort_values(by=["density", "vertices"], inplace=True)

# Group data by density
densities = df["density"].unique()

# Plot data for each density
for density in sorted(densities):
    subset = df[df["density"] == density]
    
    plt.figure(figsize=(10, 6))
    for alg in subset["alg"].unique():
        alg_data = subset[subset["alg"] == alg]
        plt.plot(
            alg_data["vertices"],
            alg_data["time"],
            marker="o",
            label=alg
        )
    
    plt.title(f"Performance for Density {density}")
    plt.xlabel("Number of Vertices")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"density_{density}.png")
