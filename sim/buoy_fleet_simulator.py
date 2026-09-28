"""
PolarMesh Southern Ocean Buoy Fleet Simulator
Simulates multi-buoy drift, wave kinetic energy harvesting, CTD depth profiles,
SX1262 LoRa mesh clustering, and Iridium SBD transmissions.
"""

import math
import time
import json
import random
import os
from http.server import HTTPServer, BaseHTTPRequestHandler
import threading

def haversine_km(lat1, lon1, lat2, lon2):
    R = 6371.0 # Earth radius km
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlambda = math.radians(lon2 - lon1)
    a = math.sin(dphi/2)**2 + math.cos(phi1)*math.cos(phi2)*math.sin(dlambda/2)**2
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))
    return R * c

class BuoyNode:
    def __init__(self, node_id, name, lat, lon, is_gateway=False, under_ice=False):
        self.node_id = node_id
        self.name = name
        self.lat = lat
        self.lon = lon
        self.is_gateway = is_gateway
        self.under_ice = under_ice
        self.battery_primary_mv = 3600 # 3.6V Li-SOCl2
        self.battery_buffer_mv = 3280 # 3.3V LiFePO4
        self.supercap_mv = 5080 # 5.0V Supercap rail
        self.battery_soc_pct = 94
        self.wave_height_m = 2.4
        self.wave_period_s = 6.2
        self.harvest_power_mw = 420
        self.sea_surface_temp_c = -1.65
        self.salinity_psu = 34.15
        self.mesh_neighbors = []
        self.last_uplink_time = time.time() - random.randint(300, 3600)
        self.uplink_status = "READY"
        self.anomaly_flag = "NONE"
        self.drift_heading_deg = 45 # NE drift with ACC
        self.drift_speed_knots = 0.65

    def step(self, dt_minutes=5):
        # 1. Update drift
        heading_rad = math.radians(self.drift_heading_deg)
        dist_nm = self.drift_speed_knots * (dt_minutes / 60.0)
        dist_deg = dist_nm / 60.0
        self.lat += dist_deg * math.cos(heading_rad)
        self.lon += dist_deg * math.sin(heading_rad) / math.cos(math.radians(self.lat))

        # 2. Wave conditions & Energy Harvesting
        self.wave_height_m = max(1.2, min(5.5, self.wave_height_m + random.uniform(-0.15, 0.15)))
        # Kinetic power = 0.5 * m * g^2 * H^2 / T
        self.harvest_power_mw = int(85 * (self.wave_height_m ** 1.8))
        if self.under_ice:
            self.harvest_power_mw = 15 # Heavily damped under ice sheet

        # 3. Supercapacitor charging & Iridium pulse simulation
        if not self.under_ice and self.supercap_mv < 5150:
            self.supercap_mv = min(5200, self.supercap_mv + int(self.harvest_power_mw * 0.05))

        # 4. CTD variations
        self.sea_surface_temp_c = round(-1.65 + random.uniform(-0.1, 0.1), 3)
        self.salinity_psu = round(34.15 + random.uniform(-0.05, 0.05), 3)

        if self.under_ice:
            self.sea_surface_temp_c = -1.92
            self.anomaly_flag = "ICE_LOCKED"
        else:
            self.anomaly_flag = "NONE"

    def generate_ctd_profile(self):
        depths = [2, 10, 25, 50, 75, 100, 150, 200, 300, 400, 500, 600, 750, 1000]
        profile = []
        for d in depths:
            if d <= 50:
                t = self.sea_surface_temp_c
                s = self.salinity_psu
            elif d <= 150:
                t = -1.78
                s = 34.32
            elif d <= 600:
                t = 1.25 - (d - 150) * 0.001
                s = 34.68
            else:
                t = -0.38
                s = 34.66
            rho = 1027.5 + (s - 34.0) * 0.82 - (t + 1.0) * 0.22 + (d * 0.0045)
            profile.append({
                "depth_m": d,
                "temp_c": round(t, 3),
                "salinity_psu": round(s, 3),
                "density_kg_m3": round(rho, 2)
            })
        return profile

    def to_dict(self):
        return {
            "node_id": self.node_id,
            "name": self.name,
            "latitude": round(self.lat, 5),
            "longitude": round(self.lon, 5),
            "is_gateway": self.is_gateway,
            "under_ice": self.under_ice,
            "battery": {
                "primary_li_socl2_mv": self.battery_primary_mv,
                "buffer_lifepo4_mv": self.battery_buffer_mv,
                "supercap_5v_rail_mv": self.supercap_mv,
                "soc_percent": self.battery_soc_pct,
                "harvest_mw": self.harvest_power_mw
            },
            "ocean": {
                "sst_c": self.sea_surface_temp_c,
                "salinity_psu": self.salinity_psu,
                "wave_height_m": round(self.wave_height_m, 2),
                "wave_period_s": round(self.wave_period_s, 1),
                "freezing_point_c": round(-0.0575 * self.salinity_psu, 3)
            },
            "comms": {
                "mesh_neighbors": self.mesh_neighbors,
                "last_uplink_epoch": int(self.last_uplink_time),
                "uplink_status": self.uplink_status,
                "iridium_csq": 4 if not self.under_ice else 0
            },
            "status": {
                "anomaly": self.anomaly_flag,
                "operating_state": "ICE_HOLD" if self.under_ice else ("GATEWAY_ACTIVE" if self.is_gateway else "SURFACE_DRIFT")
            },
            "ctd_profile": self.generate_ctd_profile()
        }

class PolarFleetSimulator:
    def __init__(self):
        self.buoys = [
            BuoyNode("PM-01", "Weddell Gateway Alpha", -64.48, -48.15, is_gateway=True),
            BuoyNode("PM-02", "Weddell Sea Central", -64.82, -48.65, is_gateway=False),
            BuoyNode("PM-03", "Larsen C Margin Sentinel", -65.25, -49.30, is_gateway=False),
            BuoyNode("PM-04", "Drake Passage Profiler", -61.12, -56.40, is_gateway=False),
            BuoyNode("PM-05", "Filchner-Ronne Under-Ice Sentinel", -76.85, -42.50, is_gateway=False, under_ice=True),
        ]

    def update_mesh_links(self):
        for b1 in self.buoys:
            b1.mesh_neighbors = []
            if b1.under_ice:
                continue
            for b2 in self.buoys:
                if b1.node_id == b2.node_id or b2.under_ice:
                    continue
                dist = haversine_km(b1.lat, b1.lon, b2.lat, b2.lon)
                # SX1262 SF11 maximum line-of-sight maritime range ~15 km
                if dist <= 35.0: # Close enough for LoRa maritime propagation
                    rssi = max(-138, int(-65 - 20 * math.log10(dist + 0.1) - random.uniform(0, 5)))
                    snr = max(-18, int(8 - dist * 0.4))
                    b1.mesh_neighbors.append({
                        "node_id": b2.node_id,
                        "distance_km": round(dist, 1),
                        "rssi_dbm": rssi,
                        "snr_db": snr
                    })

    def tick(self):
        for b in self.buoys:
            b.step(dt_minutes=2)
        self.update_mesh_links()

    def snapshot(self):
        return {
            "system": "PolarMesh Autonomous Southern Ocean Observation Platform",
            "sponsor": "Ministry of Earth Sciences (MoES) / INCOIS",
            "timestamp": int(time.time()),
            "active_buoys": len(self.buoys),
            "fleet": [b.to_dict() for b in self.buoys]
        }

def save_snapshot(sim, path="sim/fleet_telemetry.json"):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        json.dump(sim.snapshot(), f, indent=2)

if __name__ == "__main__":
    simulator = PolarFleetSimulator()
    simulator.tick()
    save_snapshot(simulator)
    print("Fleet simulator initialized and snapshot saved to sim/fleet_telemetry.json")
