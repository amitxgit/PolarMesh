"use client";

import React, { useState, useEffect } from "react";
import {
  Radio,
  Satellite,
  Compass,
  Zap,
  Activity,
  AlertTriangle,
  Layers,
  Thermometer,
  ShieldCheck,
  Send,
  RotateCw,
  Anchor,
  Play,
  Pause,
  Server
} from "lucide-react";

interface BuoyData {
  node_id: string;
  name: string;
  latitude: number;
  longitude: number;
  is_gateway: boolean;
  under_ice: boolean;
  battery: {
    primary_li_socl2_mv: number;
    buffer_lifepo4_mv: number;
    supercap_5v_rail_mv: number;
    soc_percent: number;
    harvest_mw: number;
  };
  ocean: {
    sst_c: number;
    salinity_psu: number;
    wave_height_m: number;
    wave_period_s: number;
    freezing_point_c: number;
  };
  comms: {
    mesh_neighbors: Array<{
      node_id: string;
      distance_km: number;
      rssi_dbm: number;
      snr_db: number;
    }>;
    last_uplink_epoch: number;
    uplink_status: string;
    iridium_csq: number;
  };
  status: {
    anomaly: string;
    operating_state: string;
  };
  ctd_profile: Array<{
    depth_m: number;
    temp_c: number;
    salinity_psu: number;
    density_kg_m3: number;
  }>;
}

export default function PolarMeshDashboard() {
  const [fleet, setFleet] = useState<BuoyData[]>([]);
  const [selectedNodeId, setSelectedNodeId] = useState<string>("PM-01");
  const [activeTab, setActiveTab] = useState<"ctd" | "mesh" | "iridium" | "power">("ctd");
  const [ctdMode, setCtdMode] = useState<"temp" | "sal" | "density">("temp");
  const [utcTime, setUtcTime] = useState<string>("");
  const [isSimulating, setIsSimulating] = useState<boolean>(true);
  const [injectedAnomaly, setInjectedAnomaly] = useState<boolean>(false);
  const [terminalLogs, setTerminalLogs] = useState<string[]>([
    "[SYSTEM_INIT] PolarMesh MoES / INCOIS Telemetry Gateway Connected",
    "[SX1262] Sub-GHz Mesh synchronized on 868.100 MHz, SF11, BW125",
    "[IRIDIUM_9603] Constellation link confirmed (AT+CSQ: 4/5 bars)",
    "[CLUSTER_HEAD] Node PM-01 elected gateway for Weddell Sea cluster"
  ]);

  // Fetch telemetry
  const fetchTelemetry = async () => {
    try {
      const res = await fetch("/api/telemetry");
      const data = await res.json();
      if (data && data.fleet) {
        setFleet(data.fleet);
      }
    } catch (e) {
      console.error("Telemetry fetch error:", e);
    }
  };

  useEffect(() => {
    fetchTelemetry();
    const clockInterval = setInterval(() => {
      setUtcTime(new Date().toUTCString().replace("GMT", "UTC"));
    }, 1000);

    const simInterval = setInterval(() => {
      if (isSimulating) {
        fetchTelemetry();
      }
    }, 4000);

    return () => {
      clearInterval(clockInterval);
      clearInterval(simInterval);
    };
  }, [isSimulating]);

  const selectedBuoy = fleet.find((b) => b.node_id === selectedNodeId) || fleet[0];

  // Helper to trigger simulated anomaly
  const handleTriggerAnomaly = () => {
    setInjectedAnomaly(!injectedAnomaly);
    if (!injectedAnomaly) {
      setTerminalLogs((prev) => [
        `[ALERT_TRIGGER] Frazil ice accretion detected on ${selectedBuoy?.name || "buoy"}!`,
        `[EDGE_AI] UNESCO equation alert: Water temp reached freezing point (-1.96°C)`,
        `[FSM_TRANSITION] Power state switched to ICE_HOLD mode to protect antenna`,
        ...prev.slice(0, 8)
      ]);
    } else {
      setTerminalLogs((prev) => [
        `[RECOVERY] Thermal recovery confirmed, resuming normal surface drift cast`,
        ...prev.slice(0, 8)
      ]);
    }
  };

  const handleManualUplink = () => {
    setTerminalLogs((prev) => [
      `[IRIDIUM_TX] AT+SBDIX session initiated for ${selectedNodeId}`,
      `[SUPERCAP_DISCHARGE] 2.5A pulse absorbed by 5.0V supercap rail (V_cap: 4.88V)`,
      `[SBD_ACK] Satellite MO-SBD status: 0 (Message delivered to MoES Gateway)`,
      ...prev.slice(0, 8)
    ]);
  };

  return (
    <div style={{ minHeight: "100vh", display: "flex", flexDirection: "column" }}>
      {/* Top Header Bar */}
      <header className="app-header">
        <div className="brand-section">
          <div style={{ display: "flex", alignItems: "center", gap: 10 }}>
            <Anchor size={24} color="#00f2fe" />
            <div>
              <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
                <span style={{ fontSize: 18, fontWeight: 800, letterSpacing: "-0.02em" }}>
                  POLARMESH
                </span>
                <span className="brand-badge">SIH26065</span>
              </div>
              <div style={{ fontSize: 11, color: "var(--text-muted)" }}>
                Ministry of Earth Sciences (MoES) & INCOIS Polar Platform
              </div>
            </div>
          </div>
        </div>

        {/* Center Indicators */}
        <div style={{ display: "flex", alignItems: "center", gap: 14 }}>
          <div className="header-status-pill">
            <Satellite size={16} color="#00f2fe" />
            <span style={{ fontSize: 12 }}>Iridium 9603 SBD:</span>
            <span className="mono" style={{ color: "#00f5d4", fontWeight: 700 }}>
              ONLINE (4/5)
            </span>
          </div>

          <div className="header-status-pill">
            <Radio size={16} color="#4facfe" />
            <span style={{ fontSize: 12 }}>SX1262 LoRa Swarm:</span>
            <span className="mono" style={{ color: "#00f2fe", fontWeight: 700 }}>
              {fleet.length} BUOYS ACTIVE
            </span>
          </div>

          <div className="header-status-pill">
            <div className="pulse-indicator" />
            <span className="mono" style={{ fontSize: 12, color: "var(--text-main)" }}>
              {utcTime || "SYNCING UTC..."}
            </span>
          </div>
        </div>

        {/* Action Controls */}
        <div style={{ display: "flex", alignItems: "center", gap: 10 }}>
          <button
            id="btn-toggle-sim"
            className="btn-action"
            onClick={() => setIsSimulating(!isSimulating)}
          >
            {isSimulating ? <Pause size={14} /> : <Play size={14} />}
            <span>{isSimulating ? "Live Feed" : "Paused"}</span>
          </button>

          <button
            id="btn-inject-anomaly"
            className={`btn-action ${injectedAnomaly ? "btn-danger" : ""}`}
            onClick={handleTriggerAnomaly}
          >
            <AlertTriangle size={14} />
            <span>{injectedAnomaly ? "Clear Ice Lock" : "Simulate Ice Lock"}</span>
          </button>
        </div>
      </header>

      {/* Main Workspace Layout */}
      <main className="dashboard-container">
        {/* Left Column: Interactive Polar Map & Buoy Selector */}
        <div style={{ display: "flex", flexDirection: "column", gap: 20 }}>
          {/* Buoy Selection Pills */}
          <div className="buoy-tab-list">
            {fleet.map((b) => {
              const isSel = b.node_id === selectedNodeId;
              return (
                <div
                  key={b.node_id}
                  id={`tab-${b.node_id}`}
                  className={`buoy-tab ${isSel ? "active" : ""}`}
                  onClick={() => setSelectedNodeId(b.node_id)}
                >
                  <div
                    className={`pulse-indicator ${
                      b.under_ice || injectedAnomaly && isSel ? "crit" : b.is_gateway ? "" : "warn"
                    }`}
                  />
                  <div>
                    <div style={{ fontSize: 13, fontWeight: 700 }}>{b.node_id}</div>
                    <div style={{ fontSize: 10, color: "var(--text-muted)" }}>
                      {b.under_ice ? "Under Ice" : b.is_gateway ? "Cluster Gateway" : "Drifting"}
                    </div>
                  </div>
                </div>
              );
            })}
          </div>

          {/* Interactive Antarctic Polar Map */}
          <div className="glass-panel" style={{ padding: 18 }}>
            <div
              style={{
                display: "flex",
                justifyContent: "space-between",
                alignItems: "center",
                marginBottom: 12
              }}
            >
              <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
                <Compass size={18} color="#00f2fe" />
                <span style={{ fontSize: 14, fontWeight: 700, letterSpacing: "0.04em" }}>
                  SOUTHERN OCEAN POLAR STEREOGRAPHIC TOPOLOGY
                </span>
              </div>
              <div style={{ fontSize: 12, color: "var(--text-muted)" }}>
                Weddell & Ross Sea Swarm Relays (Lat: -60°S to -78°S)
              </div>
            </div>

            {/* SVG Polar Map Container */}
            <div className="map-canvas-container">
              <svg width="100%" height="100%" viewBox="0 0 800 480" preserveAspectRatio="xMidYMid meet">
                <defs>
                  <radialGradient id="oceanGlow" cx="50%" cy="50%" r="50%">
                    <stop offset="0%" stopColor="#0d234d" stopOpacity="0.4" />
                    <stop offset="100%" stopColor="#050a16" stopOpacity="0.95" />
                  </radialGradient>
                  <filter id="glow">
                    <feGaussianBlur stdDeviation="3" result="coloredBlur" />
                    <feMerge>
                      <feMergeNode in="coloredBlur" />
                      <feMergeNode in="SourceGraphic" />
                    </feMerge>
                  </filter>
                </defs>

                {/* Background Bathymetry Grid Lines */}
                <circle cx="400" cy="240" r="210" fill="none" stroke="rgba(79, 172, 254, 0.12)" strokeDasharray="3 3" />
                <circle cx="400" cy="240" r="140" fill="none" stroke="rgba(79, 172, 254, 0.18)" strokeDasharray="4 2" />
                <circle cx="400" cy="240" r="70" fill="none" stroke="rgba(79, 172, 254, 0.25)" />
                <line x1="400" y1="30" x2="400" y2="450" stroke="rgba(79, 172, 254, 0.08)" />
                <line x1="190" y1="240" x2="610" y2="240" stroke="rgba(79, 172, 254, 0.08)" />

                {/* Continental Margin & Ice Shelf Contours (Weddell Sea / Antarctic Peninsula) */}
                <path
                  d="M 120,40 C 180,120 220,180 270,250 C 310,310 330,380 430,420 C 510,430 620,380 680,310 C 730,230 750,150 780,90 L 800,480 L 0,480 Z"
                  className="ice-shelf-contour"
                />
                <text x="500" y="440" fill="rgba(255,255,255,0.3)" fontSize="11" fontFamily="sans-serif">
                  FILCHNER-RONNE ICE SHELF
                </text>
                <text x="210" y="270" fill="rgba(255,255,255,0.35)" fontSize="11" fontFamily="sans-serif">
                  LARSEN C ICE SHELF
                </text>
                <text x="140" y="100" fill="rgba(0,242,254,0.4)" fontSize="11" fontFamily="sans-serif">
                  DRAKE PASSAGE (ACC FLOW)
                </text>

                {/* Animated Mesh Links between Buoys */}
                <line x1="280" y1="180" x2="350" y2="220" className="mesh-link-line" />
                <line x1="350" y1="220" x2="420" y2="280" className="mesh-link-line" />
                <text x="315" y="195" fill="#00f2fe" fontSize="10" fontFamily="var(--font-mono)">
                  18.2 km (-102 dBm)
                </text>
                <text x="385" y="245" fill="#00f2fe" fontSize="10" fontFamily="var(--font-mono)">
                  32.5 km (-118 dBm)
                </text>

                {/* Render Buoy Markers */}
                {/* PM-04 (Drake Passage) */}
                <g className="buoy-node-marker" onClick={() => setSelectedNodeId("PM-04")}>
                  <circle cx="180" cy="110" r="14" fill="rgba(79, 172, 254, 0.2)" stroke="#4facfe" strokeWidth="1.5" />
                  <circle cx="180" cy="110" r="5" fill="#4facfe" />
                  <text x="198" y="114" fill="#f1f5f9" fontSize="12" fontWeight="700">
                    PM-04
                  </text>
                </g>

                {/* PM-01 (Weddell Gateway) */}
                <g className="buoy-node-marker" onClick={() => setSelectedNodeId("PM-01")}>
                  <circle cx="280" cy="180" r="20" fill="rgba(0, 242, 254, 0.25)" stroke="#00f2fe" strokeWidth="2" filter="url(#glow)" />
                  <circle cx="280" cy="180" r="7" fill="#00f5d4" />
                  <text x="295" y="175" fill="#00f2fe" fontSize="13" fontWeight="800">
                    PM-01 [GATEWAY]
                  </text>
                </g>

                {/* PM-02 (Weddell Central) */}
                <g className="buoy-node-marker" onClick={() => setSelectedNodeId("PM-02")}>
                  <circle cx="350" cy="220" r="14" fill="rgba(79, 172, 254, 0.2)" stroke="#4facfe" strokeWidth="1.5" />
                  <circle cx="350" cy="220" r="5" fill="#4facfe" />
                  <text x="368" y="224" fill="#f1f5f9" fontSize="12" fontWeight="700">
                    PM-02
                  </text>
                </g>

                {/* PM-03 (Larsen C Margin) */}
                <g className="buoy-node-marker" onClick={() => setSelectedNodeId("PM-03")}>
                  <circle cx="420" cy="280" r="15" fill={injectedAnomaly && selectedNodeId === "PM-03" ? "rgba(239, 68, 68, 0.3)" : "rgba(245, 158, 11, 0.2)"} stroke={injectedAnomaly && selectedNodeId === "PM-03" ? "#ef4444" : "#f59e0b"} strokeWidth="1.5" />
                  <circle cx="420" cy="280" r="5" fill={injectedAnomaly && selectedNodeId === "PM-03" ? "#ef4444" : "#f59e0b"} />
                  <text x="438" y="284" fill="#f1f5f9" fontSize="12" fontWeight="700">
                    PM-03
                  </text>
                </g>

                {/* PM-05 (Under Ice Sentinel) */}
                <g className="buoy-node-marker" onClick={() => setSelectedNodeId("PM-05")}>
                  <circle cx="560" cy="390" r="15" fill="rgba(239, 68, 68, 0.25)" stroke="#ef4444" strokeWidth="1.5" strokeDasharray="3 3" />
                  <circle cx="560" cy="390" r="5" fill="#ef4444" />
                  <text x="578" y="394" fill="#fca5a5" fontSize="12" fontWeight="700">
                    PM-05 [ICE HOLD]
                  </text>
                </g>
              </svg>
            </div>
          </div>

          {/* Bottom Tabs: CTD Cast / Mesh / Satellite / Power */}
          <div className="glass-panel" style={{ padding: 20 }}>
            {/* Tab Navigation */}
            <div style={{ display: "flex", gap: 12, borderBottom: "1px solid var(--border-subtle)", paddingBottom: 12, marginBottom: 16 }}>
              <button
                className={`btn-action ${activeTab === "ctd" ? "" : "btn-pill"}`}
                style={{ background: activeTab === "ctd" ? undefined : "transparent", borderColor: activeTab === "ctd" ? undefined : "transparent" }}
                onClick={() => setActiveTab("ctd")}
              >
                <Activity size={16} /> CTD Water Column Cast
              </button>
              <button
                className={`btn-action ${activeTab === "mesh" ? "" : "btn-pill"}`}
                style={{ background: activeTab === "mesh" ? undefined : "transparent", borderColor: activeTab === "mesh" ? undefined : "transparent" }}
                onClick={() => setActiveTab("mesh")}
              >
                <Radio size={16} /> LoRa Swarm Topology
              </button>
              <button
                className={`btn-action ${activeTab === "iridium" ? "" : "btn-pill"}`}
                style={{ background: activeTab === "iridium" ? undefined : "transparent", borderColor: activeTab === "iridium" ? undefined : "transparent" }}
                onClick={() => setActiveTab("iridium")}
              >
                <Satellite size={16} /> Iridium SBD Packet Terminal
              </button>
              <button
                className={`btn-action ${activeTab === "power" ? "" : "btn-pill"}`}
                style={{ background: activeTab === "power" ? undefined : "transparent", borderColor: activeTab === "power" ? undefined : "transparent" }}
                onClick={() => setActiveTab("power")}
              >
                <Zap size={16} /> Cryo-Power Telemetry
              </button>
            </div>

            {/* TAB CONTENT: CTD PROFILE */}
            {activeTab === "ctd" && (
              <div>
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 14 }}>
                  <div style={{ fontSize: 13, color: "var(--text-muted)" }}>
                    Deep vertical CTD cast for <strong style={{ color: "#00f2fe" }}>{selectedBuoy?.name}</strong> (Surface to 1000m)
                  </div>
                  <div style={{ display: "flex", gap: 6 }}>
                    <button
                      className="btn-action"
                      style={{ fontSize: 11, padding: "4px 10px", background: ctdMode === "temp" ? undefined : "rgba(255,255,255,0.05)" }}
                      onClick={() => setCtdMode("temp")}
                    >
                      Temperature (°C)
                    </button>
                    <button
                      className="btn-action"
                      style={{ fontSize: 11, padding: "4px 10px", background: ctdMode === "sal" ? undefined : "rgba(255,255,255,0.05)" }}
                      onClick={() => setCtdMode("sal")}
                    >
                      Salinity (PSU)
                    </button>
                    <button
                      className="btn-action"
                      style={{ fontSize: 11, padding: "4px 10px", background: ctdMode === "density" ? undefined : "rgba(255,255,255,0.05)" }}
                      onClick={() => setCtdMode("density")}
                    >
                      Density (kg/m³)
                    </button>
                  </div>
                </div>

                {/* SVG CTD Depth Chart */}
                <div style={{ height: 260, background: "rgba(0,0,0,0.25)", borderRadius: 8, padding: 12, position: "relative" }}>
                  <svg width="100%" height="100%" viewBox="0 0 650 220" preserveAspectRatio="none">
                    {/* Depth grid lines */}
                    {[0, 50, 100, 150, 200].map((y, i) => (
                      <g key={i}>
                        <line x1="50" y1={y + 10} x2="630" y2={y + 10} stroke="rgba(255,255,255,0.07)" strokeDasharray="3 3" />
                        <text x="10" y={y + 14} fill="var(--text-dim)" fontSize="10" fontFamily="var(--font-mono)">
                          {i * 250}m
                        </text>
                      </g>
                    ))}

                    {/* CTD Curve */}
                    <path
                      d={
                        ctdMode === "temp"
                          ? "M 120,10 L 118,30 L 115,55 L 110,80 L 450,120 L 430,165 L 260,210"
                          : ctdMode === "sal"
                          ? "M 150,10 L 155,30 L 170,55 L 220,80 L 480,120 L 510,165 L 490,210"
                          : "M 100,10 L 110,30 L 130,55 L 180,80 L 320,120 L 450,165 L 600,210"
                      }
                      fill="none"
                      stroke={ctdMode === "temp" ? "#00f2fe" : ctdMode === "sal" ? "#00f5d4" : "#a855f7"}
                      strokeWidth="3"
                    />

                    {/* Water Mass Zones */}
                    <rect x="520" y="15" width="100" height="22" rx="4" fill="rgba(0, 242, 254, 0.15)" />
                    <text x="526" y="30" fill="#00f2fe" fontSize="9" fontWeight="700">
                      SURFACE MIXED (0-50m)
                    </text>

                    <rect x="520" y="60" width="100" height="22" rx="4" fill="rgba(79, 172, 254, 0.15)" />
                    <text x="526" y="75" fill="#4facfe" fontSize="9" fontWeight="700">
                      WINTER WATER (50-150m)
                    </text>

                    <rect x="520" y="110" width="100" height="22" rx="4" fill="rgba(245, 158, 11, 0.15)" />
                    <text x="526" y="125" fill="#f59e0b" fontSize="9" fontWeight="700">
                      CDW WARM LAYER
                    </text>

                    <rect x="520" y="180" width="100" height="22" rx="4" fill="rgba(168, 85, 247, 0.15)" />
                    <text x="526" y="195" fill="#a855f7" fontSize="9" fontWeight="700">
                      ANTARCTIC BOTTOM (AABW)
                    </text>
                  </svg>
                </div>
              </div>
            )}

            {/* TAB CONTENT: LORA SWARM TOPOLOGY */}
            {activeTab === "mesh" && (
              <div>
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 12 }}>
                  <div style={{ fontSize: 13, color: "var(--text-muted)" }}>
                    Dynamic SX1262 LoRa Swarm routing table & neighbor discovery metrics
                  </div>
                  <div className="mono" style={{ fontSize: 12, color: "#00f5d4" }}>
                    Network ID: 0x504D (POLARMESH)
                  </div>
                </div>

                <div style={{ overflowX: "auto" }}>
                  <table style={{ width: "100%", borderCollapse: "collapse", fontSize: 12 }}>
                    <thead>
                      <tr style={{ borderBottom: "1px solid var(--border-subtle)", textAlign: "left", color: "var(--text-muted)" }}>
                        <th style={{ padding: "8px 12px" }}>Peer Node</th>
                        <th style={{ padding: "8px 12px" }}>Distance</th>
                        <th style={{ padding: "8px 12px" }}>RSSI (dBm)</th>
                        <th style={{ padding: "8px 12px" }}>SNR (dB)</th>
                        <th style={{ padding: "8px 12px" }}>Link Quality</th>
                        <th style={{ padding: "8px 12px" }}>Gateway Route</th>
                      </tr>
                    </thead>
                    <tbody>
                      <tr style={{ borderBottom: "1px solid rgba(255,255,255,0.05)" }}>
                        <td style={{ padding: "10px 12px", fontWeight: 700 }}>PM-01 (Weddell Gateway)</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>0.0 km (Local)</td>
                        <td className="mono" style={{ padding: "10px 12px", color: "#00f5d4" }}>-45 dBm</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>+9 dB</td>
                        <td style={{ padding: "10px 12px" }}><span style={{ color: "#10b981" }}>EXCELLENT (100%)</span></td>
                        <td style={{ padding: "10px 12px" }}><span className="brand-badge">DIRECT SATELLITE</span></td>
                      </tr>
                      <tr style={{ borderBottom: "1px solid rgba(255,255,255,0.05)" }}>
                        <td style={{ padding: "10px 12px", fontWeight: 700 }}>PM-02 (Weddell Central)</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>18.2 km</td>
                        <td className="mono" style={{ padding: "10px 12px", color: "#00f2fe" }}>-102 dBm</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>+4 dB</td>
                        <td style={{ padding: "10px 12px" }}><span style={{ color: "#10b981" }}>STRONG (98.4%)</span></td>
                        <td style={{ padding: "10px 12px" }}>1 Hop to PM-01</td>
                      </tr>
                      <tr style={{ borderBottom: "1px solid rgba(255,255,255,0.05)" }}>
                        <td style={{ padding: "10px 12px", fontWeight: 700 }}>PM-03 (Larsen C Margin)</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>32.5 km</td>
                        <td className="mono" style={{ padding: "10px 12px", color: "#f59e0b" }}>-118 dBm</td>
                        <td className="mono" style={{ padding: "10px 12px" }}>-2 dB</td>
                        <td style={{ padding: "10px 12px" }}><span style={{ color: "#f59e0b" }}>MODERATE (91.2%)</span></td>
                        <td style={{ padding: "10px 12px" }}>1 Hop to PM-01</td>
                      </tr>
                    </tbody>
                  </table>
                </div>
              </div>
            )}

            {/* TAB CONTENT: IRIDIUM PACKET TERMINAL */}
            {activeTab === "iridium" && (
              <div>
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 10 }}>
                  <div style={{ fontSize: 13, color: "var(--text-muted)" }}>
                    Iridium 9603 SBD 340-byte Binary Payload Stream & Diagnostic Logs
                  </div>
                  <button className="btn-action" onClick={handleManualUplink}>
                    <Send size={14} /> Send Manual SBD Ping
                  </button>
                </div>

                <div
                  className="mono"
                  style={{
                    height: 180,
                    overflowY: "auto",
                    background: "rgba(0,0,0,0.5)",
                    border: "1px solid var(--border-subtle)",
                    borderRadius: 8,
                    padding: 12,
                    fontSize: 11,
                    lineHeight: 1.6,
                    color: "#94a3b8"
                  }}
                >
                  <div style={{ color: "#00f2fe", fontWeight: 700 }}>
                    HEX PAYLOAD (MO-SBD 166 BYTES):
                  </div>
                  <div style={{ color: "#38bdf8", wordBreak: "break-all", marginBottom: 10 }}>
                    504D AA01 FFFF 66F81200 02 01 FCD81230 FB490214 0004 07 01 ... [CRC16: 0xA4F2]
                  </div>
                  {terminalLogs.map((log, idx) => (
                    <div key={idx} style={{ color: log.includes("ALERT") ? "#ef4444" : log.includes("IRIDIUM") ? "#00f5d4" : "#cbd5e1" }}>
                      {log}
                    </div>
                  ))}
                </div>
              </div>
            )}

            {/* TAB CONTENT: POWER & ENERGY BUDGET */}
            {activeTab === "power" && (
              <div>
                <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 14 }}>
                  <div style={{ fontSize: 13, color: "var(--text-muted)" }}>
                    Triple-Tier Cold Battery & Wave Kinetic Energy Harvesting Subsystem
                  </div>
                  <span className="brand-badge">POSITIVE ENERGY SURPLUS</span>
                </div>

                <div className="metric-grid">
                  <div className="metric-card">
                    <div className="metric-label"><Zap size={14} color="#00f2fe" /> Primary Li-SOCl2</div>
                    <div className="metric-val">{selectedBuoy?.battery.primary_li_socl2_mv} mV</div>
                    <div className="metric-sub">Saft LS33600 (3.6V nominal, -60°C)</div>
                  </div>
                  <div className="metric-card">
                    <div className="metric-label"><ShieldCheck size={14} color="#10b981" /> Supercap Rail</div>
                    <div className="metric-val" style={{ color: "#10b981" }}>
                      {selectedBuoy?.battery.supercap_5v_rail_mv} mV
                    </div>
                    <div className="metric-sub">25F 5.5V (Iridium 2.5A Pulse Ready)</div>
                  </div>
                  <div className="metric-card">
                    <div className="metric-label"><RotateCw size={14} color="#00f5d4" /> Wave Kinetic Harvest</div>
                    <div className="metric-val" style={{ color: "#00f5d4" }}>
                      {selectedBuoy?.battery.harvest_mw} mW
                    </div>
                    <div className="metric-sub">Pendulum Dynamo (2.4m Swells)</div>
                  </div>
                  <div className="metric-card">
                    <div className="metric-label"><Activity size={14} color="#4facfe" /> Battery SOC</div>
                    <div className="metric-val">{selectedBuoy?.battery.soc_percent}%</div>
                    <div className="metric-sub">Estimated Lifetime: ~2.5 Years</div>
                  </div>
                </div>
              </div>
            )}
          </div>
        </div>

        {/* Right Column: Buoy Telemetry & Health Gauges */}
        <div style={{ display: "flex", flexDirection: "column", gap: 20 }}>
          {/* Active Buoy Card */}
          <div className="glass-panel-glow" style={{ padding: 22 }}>
            <div style={{ display: "flex", justifyContent: "space-between", alignItems: "flex-start", marginBottom: 16 }}>
              <div>
                <div style={{ display: "flex", alignItems: "center", gap: 8 }}>
                  <span style={{ fontSize: 20, fontWeight: 800 }}>{selectedBuoy?.name}</span>
                  {selectedBuoy?.is_gateway && <span className="brand-badge">GATEWAY</span>}
                </div>
                <div className="mono" style={{ fontSize: 12, color: "var(--text-muted)", marginTop: 4 }}>
                  ID: {selectedBuoy?.node_id} | State: {injectedAnomaly ? "ICE_HOLD" : selectedBuoy?.status.operating_state}
                </div>
              </div>
              <div
                className={`pulse-indicator ${
                  selectedBuoy?.under_ice || injectedAnomaly ? "crit" : "ok"
                }`}
                style={{ width: 12, height: 12 }}
              />
            </div>

            {/* GPS & Heading Box */}
            <div
              style={{
                background: "rgba(0,0,0,0.3)",
                borderRadius: 8,
                padding: "10px 14px",
                display: "flex",
                justifyContent: "space-between",
                marginBottom: 16
              }}
            >
              <div>
                <div style={{ fontSize: 10, color: "var(--text-muted)", textTransform: "uppercase" }}>Position</div>
                <div className="mono" style={{ fontSize: 13, fontWeight: 700, color: "var(--cyan-ice)" }}>
                  {selectedBuoy?.latitude.toFixed(4)}°S, {Math.abs(selectedBuoy?.longitude || 0).toFixed(4)}°W
                </div>
              </div>
              <div>
                <div style={{ fontSize: 10, color: "var(--text-muted)", textTransform: "uppercase" }}>Drift Vector</div>
                <div className="mono" style={{ fontSize: 13, fontWeight: 700 }}>
                  0.65 kts @ 045° (ACC)
                </div>
              </div>
            </div>

            {/* Oceanographic Measurements */}
            <div style={{ fontSize: 12, fontWeight: 700, color: "var(--text-muted)", marginBottom: 10, textTransform: "uppercase", letterSpacing: "0.05em" }}>
              Oceanographic Telemetry
            </div>

            <div className="metric-grid" style={{ marginBottom: 16 }}>
              <div className="metric-card">
                <div className="metric-label"><Thermometer size={14} color="#00f2fe" /> Sea Surface Temp</div>
                <div className="metric-val">
                  {injectedAnomaly ? "-1.98" : selectedBuoy?.ocean.sst_c}°C
                </div>
                <div className="metric-sub">TSYS01 High Precision</div>
              </div>

              <div className="metric-card">
                <div className="metric-label"><Layers size={14} color="#00f5d4" /> Salinity (PSS-78)</div>
                <div className="metric-val">{selectedBuoy?.ocean.salinity_psu} PSU</div>
                <div className="metric-sub">Atlas EZO-EC K 10.0</div>
              </div>
            </div>

            {/* UNESCO Dynamic Freezing Gauge */}
            <div
              style={{
                background: "rgba(255,255,255,0.03)",
                border: "1px solid var(--border-subtle)",
                borderRadius: 10,
                padding: 14,
                marginBottom: 16
              }}
            >
              <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: 6 }}>
                <span style={{ fontSize: 12, fontWeight: 600 }}>UNESCO Dynamic Freezing Point</span>
                <span className="mono" style={{ fontSize: 13, fontWeight: 700, color: "#f59e0b" }}>
                  {selectedBuoy?.ocean.freezing_point_c}°C
                </span>
              </div>
              <div style={{ height: 6, background: "rgba(255,255,255,0.1)", borderRadius: 3, overflow: "hidden", marginBottom: 6 }}>
                <div
                  style={{
                    width: injectedAnomaly ? "96%" : "68%",
                    height: "100%",
                    background: injectedAnomaly ? "var(--accent-crit)" : "linear-gradient(90deg, #00f2fe, #f59e0b)"
                  }}
                />
              </div>
              <div style={{ display: "flex", justifyContent: "space-between", fontSize: 10, color: "var(--text-dim)" }}>
                <span>Delta to Ice Formation: {injectedAnomaly ? "0.02°C (CRITICAL)" : "0.31°C (SAFE)"}</span>
                <span>T_f Equation 1983</span>
              </div>
            </div>

            {/* Edge AI Status Banner */}
            <div
              style={{
                background: injectedAnomaly
                  ? "rgba(239, 68, 68, 0.15)"
                  : "rgba(16, 185, 129, 0.1)",
                border: `1px solid ${injectedAnomaly ? "rgba(239, 68, 68, 0.4)" : "rgba(16, 185, 129, 0.3)"}`,
                borderRadius: 8,
                padding: 12,
                display: "flex",
                alignItems: "center",
                gap: 12
              }}
            >
              <ShieldCheck size={20} color={injectedAnomaly ? "#ef4444" : "#10b981"} />
              <div>
                <div style={{ fontSize: 12, fontWeight: 700, color: injectedAnomaly ? "#fca5a5" : "#6ee7b7" }}>
                  {injectedAnomaly ? "EDGE AI: FRAZIL ICE DETECTED" : "EDGE AI: WATER COLUMN REALISTIC"}
                </div>
                <div style={{ fontSize: 10, color: "var(--text-dim)", marginTop: 2 }}>
                  {injectedAnomaly
                    ? "Autoencoder MSE: 0.320 > 0.185 threshold. Switched to Ice Hold mode."
                    : "TinyML Autoencoder reconstruction MSE: 0.042 (Normal Stratification)"}
                </div>
              </div>
            </div>
          </div>

          {/* Quick Mission Specifications */}
          <div className="glass-panel" style={{ padding: 18 }}>
            <div style={{ display: "flex", alignItems: "center", gap: 8, marginBottom: 12 }}>
              <Server size={16} color="#00f2fe" />
              <span style={{ fontSize: 13, fontWeight: 700 }}>MoES System Specs (REV_B)</span>
            </div>
            <div style={{ display: "flex", flexDirection: "column", gap: 8, fontSize: 11 }}>
              <div style={{ display: "flex", justifyContent: "space-between", borderBottom: "1px solid rgba(255,255,255,0.05)", paddingBottom: 4 }}>
                <span style={{ color: "var(--text-muted)" }}>Hull Material:</span>
                <span style={{ fontWeight: 600 }}>UHMWPE (-260°C rated)</span>
              </div>
              <div style={{ display: "flex", justifyContent: "space-between", borderBottom: "1px solid rgba(255,255,255,0.05)", paddingBottom: 4 }}>
                <span style={{ color: "var(--text-muted)" }}>Dual-MCU:</span>
                <span style={{ fontWeight: 600 }}>STM32L4R5ZI + MSP430FR5994</span>
              </div>
              <div style={{ display: "flex", justifyContent: "space-between", borderBottom: "1px solid rgba(255,255,255,0.05)", paddingBottom: 4 }}>
                <span style={{ color: "var(--text-muted)" }}>Satellite Uplink:</span>
                <span style={{ fontWeight: 600 }}>Iridium 9603 SBD (Pole-to-Pole)</span>
              </div>
              <div style={{ display: "flex", justifyContent: "space-between", borderBottom: "1px solid rgba(255,255,255,0.05)", paddingBottom: 4 }}>
                <span style={{ color: "var(--text-muted)" }}>Swarm Mesh:</span>
                <span style={{ fontWeight: 600 }}>SX1262 LoRa 868MHz (+22 dBm)</span>
              </div>
              <div style={{ display: "flex", justifyContent: "space-between" }}>
                <span style={{ color: "var(--text-muted)" }}>Target Unit Cost:</span>
                <span style={{ fontWeight: 700, color: "#10b981" }}>₹1.44 Lakh (12x lower than Argo)</span>
              </div>
            </div>
          </div>
        </div>
      </main>
    </div>
  );
}
