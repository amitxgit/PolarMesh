"""
PolarMesh Synthetic Oceanographic Dataset Generator
Generates physically grounded Southern Ocean CTD profiles for TinyML Anomaly Detection.
Based on World Ocean Atlas (WOA) and Argo float observations in the Weddell & Ross Seas.
"""

import numpy as np
import json
import csv
import os

def generate_southern_ocean_profile(depths_m, anomaly_type="NONE"):
    """
    Simulates vertical temperature, salinity, and density profile in the Southern Ocean.
    Typical Southern Ocean structure:
    - Surface Mixed Layer (0-50m): Cold (-1.8 to +0.5°C), low salinity (33.8-34.2 PSU from ice melt)
    - Winter Water (50-150m): Near freezing (-1.7°C) remnant of winter convection
    - Circumpolar Deep Water (CDW, 200-800m): Warmer (+1.0 to +1.8°C), saltier (34.65-34.72 PSU)
    - Antarctic Bottom Water (AABW, >800m): Cold (-0.4°C), salty (34.66 PSU), densest ocean water
    """
    temps = []
    salinities = []
    densities = []
    
    for d in depths_m:
        if d <= 50:
            # Surface layer
            t = -1.2 + np.random.normal(0, 0.05)
            s = 34.05 + np.random.normal(0, 0.02)
        elif d <= 150:
            # Winter water core
            t = -1.75 + np.random.normal(0, 0.03)
            s = 34.30 + np.random.normal(0, 0.02)
        elif d <= 600:
            # Circumpolar Deep Water (CDW intrusion)
            t = 1.35 - (d - 150) * 0.001 + np.random.normal(0, 0.05)
            s = 34.68 + (d - 150) * 0.0001 + np.random.normal(0, 0.01)
        else:
            # Deep bottom water
            t = -0.35 + np.random.normal(0, 0.02)
            s = 34.66 + np.random.normal(0, 0.01)

        # UNESCO equation approximation for seawater density
        rho = 1027.5 + (s - 34.0) * 0.82 - (t + 1.0) * 0.22 + (d * 0.0045)
        
        temps.append(round(t, 4))
        salinities.append(round(s, 4))
        densities.append(round(rho, 4))

    # Inject deliberate physical anomalies if requested
    if anomaly_type == "ICE_ACCRETION":
        # Surface layer supercooled below freezing point (-1.85°C), frazil ice
        temps[0] = -2.15
        temps[1] = -2.05
    elif anomaly_type == "DENSITY_INVERSION":
        # Unphysical density drop at depth (sensor failure or severe sensor clogging)
        salinities[5] -= 1.8
        densities[5] -= 1.5
    elif anomaly_type == "SENSOR_DRIFT":
        # Conductivity calibration offset ramping up
        salinities = [s + (i * 0.15) for i, s in enumerate(salinities)]

    return {
        "depths": depths_m,
        "temperatures_c": temps,
        "salinities_psu": salinities,
        "densities_kg_m3": densities,
        "anomaly": anomaly_type
    }

def build_dataset(num_samples=500, output_path="ml/southern_ocean_ctd_dataset.json"):
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    depths = [2, 10, 25, 50, 75, 100, 150, 200, 300, 400, 500, 600, 750, 1000, 1250, 1500]
    
    dataset = []
    anomaly_options = ["NONE", "NONE", "NONE", "NONE", "ICE_ACCRETION", "DENSITY_INVERSION", "SENSOR_DRIFT"]
    
    for i in range(num_samples):
        anomaly = np.random.choice(anomaly_options)
        profile = generate_southern_ocean_profile(depths, anomaly)
        profile["sample_id"] = i + 1
        dataset.append(profile)

    with open(output_path, "w") as f:
        json.dump(dataset, f, indent=2)

    print(f"Generated {num_samples} Southern Ocean CTD profiles -> {output_path}")

if __name__ == "__main__":
    build_dataset(600)
