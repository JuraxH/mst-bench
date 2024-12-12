import pandas as pd
import matplotlib.pyplot as plt
import re
from scipy.stats import linregress
import numpy as np

# Load the CSV file
file_path = "final_bench_2.csv"  # Replace with your CSV file path
df = pd.read_csv(file_path)

# Extract the number of vertices and density from the path
def extract_info(path):
    match = re.search(r"random_(\d+)_(\d+\.?\d*)", path)
    if match:
        return int(match.group(1)), float(match.group(2))
    return None, None

df["vertices"], df["density"] = zip(*df["path"].apply(extract_info))

#remove boost implementations for final graphs
df = df[df["alg"] != "kruskal_boost"]
df = df[df["alg"] != "prim_boost"]

df["time"] = df["time"]/1000

# Calculate the number of edges
def calculate_edges(vertices, density):
    return max(vertices-1, density * (vertices * (vertices - 1) / 2))

df["edges"] = df.apply(lambda row: calculate_edges(row["vertices"], row["density"]), axis=1)

# Sort the data by density and number of vertices
df.sort_values(by=["density", "vertices"], inplace=True)

# Group data by density
densities = df["density"].unique()

# Plot data for each density
for density in sorted(densities):
    subset = df[df["density"] == density]
    
    plt.figure(figsize=(6, 4.5))
    for alg in subset["alg"].unique():
        alg_data = subset[subset["alg"] == alg]
        
        plt.plot(
            alg_data["vertices"],
            alg_data["time"],
            marker="o",
            label=f"{alg}"
        )

    #plt.title(f"Performance for Density {density}")
    plt.xlabel("Number of Vertices")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"density_{density}.png")

# Plot Density vs Time for 1200 vertices
df_1200 = df[df["vertices"] == 1000]
plt.figure()
for alg in df_1200["alg"].unique():
    alg_data = df_1200[df_1200["alg"] == alg]
    
    # Plot density vs time for the largest graph (1200 vertices)
    plt.plot(
        alg_data["density"],
        alg_data["time"],
        marker="o",
        label=f"{alg}"
    )

#plt.title("Density vs Time for 1200 Vertices")
plt.xlabel("Density")
plt.ylabel("Time (ms)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("density_vs_time_1200_vertices.png")