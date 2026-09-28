import { NextResponse } from "next/server";
import fs from "fs";
import path from "path";

export async function GET() {
  try {
    // Look for simulation file in PolarMesh sim directory
    const simPath = path.resolve(process.cwd(), "..", "sim", "fleet_telemetry.json");
    
    if (fs.existsSync(simPath)) {
      const data = fs.readFileSync(simPath, "utf-8");
      return NextResponse.json(JSON.parse(data));
    }
  } catch (err) {
    console.warn("Could not read simulation file directly, generating fallback telemetry:", err);
  }

  // High-fidelity fallback telemetry if sim file not yet written
  const fallback = {
    system: "PolarMesh Autonomous Southern Ocean Observation Platform",
    sponsor: "Ministry of Earth Sciences (MoES) / INCOIS",
    timestamp: Math.floor(Date.now() / 1000),
    active_buoys: 5,
    fleet: [
      {
        node_id: "PM-01",
        name: "Weddell Gateway Alpha",
        latitude: -64.482,
        longitude: -48.154,
        is_gateway: true,
        under_ice: false,
        battery: {
          primary_li_socl2_mv: 3610,
          buffer_lifepo4_mv: 3280,
          supercap_5v_rail_mv: 5080,
          soc_percent: 94,
          harvest_mw: 460
        },
        ocean: {
          sst_c: -1.65,
          salinity_psu: 34.15,
          wave_height_m: 2.45,
          wave_period_s: 6.2,
          freezing_point_c: -1.96
        },
        comms: {
          mesh_neighbors: [
            { node_id: "PM-02", distance_km: 18.2, rssi_dbm: -102, snr_db: 4 },
            { node_id: "PM-03", distance_km: 32.5, rssi_dbm: -118, snr_db: -2 }
          ],
          last_uplink_epoch: Math.floor(Date.now() / 1000) - 420,
          uplink_status: "SUCCESS_200",
          iridium_csq: 4
        },
        status: {
          anomaly: "NONE",
          operating_state: "GATEWAY_ACTIVE"
        },
        ctd_profile: [
          { depth_m: 2, temp_c: -1.65, salinity_psu: 34.15, density_kg_m3: 1027.65 },
          { depth_m: 25, temp_c: -1.68, salinity_psu: 34.18, density_kg_m3: 1027.72 },
          { depth_m: 50, temp_c: -1.72, salinity_psu: 34.22, density_kg_m3: 1027.81 },
          { depth_m: 100, temp_c: -1.78, salinity_psu: 34.34, density_kg_m3: 1028.02 },
          { depth_m: 200, temp_c: 1.12, salinity_psu: 34.65, density_kg_m3: 1028.45 },
          { depth_m: 500, temp_c: 0.95, salinity_psu: 34.71, density_kg_m3: 1029.85 },
          { depth_m: 1000, temp_c: -0.38, salinity_psu: 34.66, density_kg_m3: 1032.10 }
        ]
      },
      {
        node_id: "PM-02",
        name: "Weddell Sea Central",
        latitude: -64.821,
        longitude: -48.652,
        is_gateway: false,
        under_ice: false,
        battery: {
          primary_li_socl2_mv: 3580,
          buffer_lifepo4_mv: 3260,
          supercap_5v_rail_mv: 4980,
          soc_percent: 91,
          harvest_mw: 420
        },
        ocean: {
          sst_c: -1.62,
          salinity_psu: 34.12,
          wave_height_m: 2.2,
          wave_period_s: 5.9,
          freezing_point_c: -1.96
        },
        comms: {
          mesh_neighbors: [
            { node_id: "PM-01", distance_km: 18.2, rssi_dbm: -104, snr_db: 3 }
          ],
          last_uplink_epoch: Math.floor(Date.now() / 1000) - 1800,
          uplink_status: "RELAYED_VIA_PM01",
          iridium_csq: 0
        },
        status: {
          anomaly: "NONE",
          operating_state: "SURFACE_DRIFT"
        },
        ctd_profile: [
          { depth_m: 2, temp_c: -1.62, salinity_psu: 34.12, density_kg_m3: 1027.62 },
          { depth_m: 100, temp_c: -1.75, salinity_psu: 34.30, density_kg_m3: 1027.98 },
          { depth_m: 500, temp_c: 1.02, salinity_psu: 34.68, density_kg_m3: 1029.82 }
        ]
      },
      {
        node_id: "PM-03",
        name: "Larsen C Margin Sentinel",
        latitude: -65.250,
        longitude: -49.300,
        is_gateway: false,
        under_ice: false,
        battery: {
          primary_li_socl2_mv: 3590,
          buffer_lifepo4_mv: 3250,
          supercap_5v_rail_mv: 5010,
          soc_percent: 88,
          harvest_mw: 380
        },
        ocean: {
          sst_c: -1.82,
          salinity_psu: 34.20,
          wave_height_m: 1.8,
          wave_period_s: 5.4,
          freezing_point_c: -1.97
        },
        comms: {
          mesh_neighbors: [
            { node_id: "PM-01", distance_km: 32.5, rssi_dbm: -119, snr_db: -3 }
          ],
          last_uplink_epoch: Math.floor(Date.now() / 1000) - 2400,
          uplink_status: "RELAYED_VIA_PM01",
          iridium_csq: 0
        },
        status: {
          anomaly: "ICE_FORMING_WARNING",
          operating_state: "NEAR_FREEZING_SURFACE"
        },
        ctd_profile: [
          { depth_m: 2, temp_c: -1.82, salinity_psu: 34.20, density_kg_m3: 1027.75 },
          { depth_m: 100, temp_c: -1.84, salinity_psu: 34.35, density_kg_m3: 1028.06 },
          { depth_m: 500, temp_c: 0.85, salinity_psu: 34.66, density_kg_m3: 1029.78 }
        ]
      },
      {
        node_id: "PM-04",
        name: "Drake Passage Profiler",
        latitude: -61.120,
        longitude: -56.400,
        is_gateway: false,
        under_ice: false,
        battery: {
          primary_li_socl2_mv: 3620,
          buffer_lifepo4_mv: 3310,
          supercap_5v_rail_mv: 5120,
          soc_percent: 96,
          harvest_mw: 640
        },
        ocean: {
          sst_c: 0.45,
          salinity_psu: 33.95,
          wave_height_m: 4.1,
          wave_period_s: 7.8,
          freezing_point_c: -1.95
        },
        comms: {
          mesh_neighbors: [],
          last_uplink_epoch: Math.floor(Date.now() / 1000) - 950,
          uplink_status: "STANDALONE_SBD",
          iridium_csq: 5
        },
        status: {
          anomaly: "NONE",
          operating_state: "SURFACE_DRIFT"
        },
        ctd_profile: [
          { depth_m: 2, temp_c: 0.45, salinity_psu: 33.95, density_kg_m3: 1027.12 },
          { depth_m: 100, temp_c: -1.10, salinity_psu: 34.25, density_kg_m3: 1027.85 },
          { depth_m: 500, temp_c: 1.45, salinity_psu: 34.72, density_kg_m3: 1029.80 }
        ]
      },
      {
        node_id: "PM-05",
        name: "Filchner-Ronne Under-Ice Sentinel",
        latitude: -76.850,
        longitude: -42.500,
        is_gateway: false,
        under_ice: true,
        battery: {
          primary_li_socl2_mv: 3510,
          buffer_lifepo4_mv: 3190,
          supercap_5v_rail_mv: 4820,
          soc_percent: 79,
          harvest_mw: 20
        },
        ocean: {
          sst_c: -1.98,
          salinity_psu: 34.42,
          wave_height_m: 0.1,
          wave_period_s: 1.0,
          freezing_point_c: -1.98
        },
        comms: {
          mesh_neighbors: [],
          last_uplink_epoch: Math.floor(Date.now() / 1000) - 28800,
          uplink_status: "ACOUSTIC_ONLY_ICE_HOLD",
          iridium_csq: 0
        },
        status: {
          anomaly: "ICE_LOCKED",
          operating_state: "ICE_HOLD"
        },
        ctd_profile: [
          { depth_m: 2, temp_c: -1.98, salinity_psu: 34.42, density_kg_m3: 1027.98 },
          { depth_m: 50, temp_c: -1.97, salinity_psu: 34.45, density_kg_m3: 1028.02 },
          { depth_m: 200, temp_c: -1.95, salinity_psu: 34.50, density_kg_m3: 1028.18 }
        ]
      }
    ]
  };

  return NextResponse.json(fallback);
}
