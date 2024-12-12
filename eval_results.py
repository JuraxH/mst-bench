import pandas as pd
import matplotlib.pyplot as plt
import re
from scipy.stats import linregress
import numpy as np

# Load the CSV file
file_path = "random_bench_4.csv"  # Replace with your CSV file path
df = pd.read_csv(file_path)

# Extract the number of vertices and density from the path
def extract_info(path):
    match = re.search(r"random_(\d+)_(\d+\.?\d*)", path)
    if match:
        return int(match.group(1)), float(match.group(2))
    return None, None

df["vertices"], df["density"] = zip(*df["path"].apply(extract_info))

# Calculate the number of edges
def calculate_edges(vertices, density):
    return max(vertices-1, density * (vertices * (vertices - 1) / 2))

df["edges"] = df.apply(lambda row: calculate_edges(row["vertices"], row["density"]), axis=1)

# Sort the data by density and number of vertices
df.sort_values(by=["density", "vertices"], inplace=True)

# Group data by density
densities = df["density"].unique()

# Initialize a dictionary to store statistical results
stat_results = {}

# Function to fit and return the best model
def fit_best_model(vertices, edges, time):
    # Model 1: |V| + |E|
    model1 = vertices + edges
    # Model 2: |E| * log(|E|)
    model2 = edges * np.log(vertices)
    # Model 3: |V| * log(|E|)
    model3 = vertices * np.log(vertices)
    
    # Perform linear regression for each model
    results = []
    
    for model, label in zip([model1, model2, model3], ["|V| + |E|", "|E|log(|V|)", "|V|log(|V|)"]):
        slope, intercept, r_value, p_value, std_err = linregress(model, time)
        results.append((label, slope, r_value**2, p_value))
    
    # Select the best model based on R-squared
    best_model = max(results, key=lambda x: x[2])
    
    return best_model

# Plot data for each density
for density in sorted(densities):
    subset = df[df["density"] == density]
    
    plt.figure(figsize=(10, 6))
    for alg in subset["alg"].unique():
        alg_data = subset[subset["alg"] == alg]
        
        # Fit the best model for the current algorithm
        best_model_label, best_slope, best_r_squared, best_p_value = fit_best_model(
            alg_data["vertices"], alg_data["edges"], alg_data["time"]
        )
        
        # Store the results
        if alg not in stat_results:
            stat_results[alg] = []
        stat_results[alg].append({
            "density": density,
            "best_model": best_model_label,
            "best_slope": best_slope,
            "best_r_squared": best_r_squared,
            "best_p_value": best_p_value
        })
        
        # Plot the data for the best model
        plt.plot(
            alg_data["vertices"],
            alg_data["time"],
            marker="o",
            label=f"{alg} ({best_model_label}, slope={best_slope:.2e}, R^2={best_r_squared:.2f})"
        )

    plt.title(f"Performance for Density {density}")
    plt.xlabel("Number of Vertices")
    plt.ylabel("Time (ms)")
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(f"density_{density}.png")

# Print statistical test results
for alg, results in stat_results.items():
    print(f"Algorithm: {alg}")
    for result in results:
        print(f"  Density: {result['density']:.2f}, Best Model: {result['best_model']}, Slope: {result['best_slope']:.2e}, R^2: {result['best_r_squared']:.2f}, p-value: {result['best_p_value']:.2e}")
