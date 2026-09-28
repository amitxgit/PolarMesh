# PolarMesh (SIH26065)
## Autonomous Low-Cost Ocean Observation Platform for Polar & Southern Oceans
**Smart India Hackathon 2026 | Problem Statement: SIH26065 | Hardware Category**  
**Sponsor Organization:** Ministry of Earth Sciences (MoES) / INCOIS  
**System Revision:** REV_B_UHMWPE (Target Field Prototype)

---

## 🌊 Overview

**PolarMesh** is an indigenous, extreme-environment, autonomous ocean observation buoy swarm designed specifically for the Antarctic, Arctic, and Southern Oceans. It provides persistent oceanographic data (salinity, temperature, depth/CTD, wave dynamics, ice-pack tracking) at **12x lower cost** than conventional Argo floats, while introducing breakthrough swarm LoRa mesh relaying, cryogenic power resilience down to −60°C, and Edge TinyML frazil ice detection.

---

## 🌟 Key Innovations

1. **Dual-MCU Cryogenic Fault Tolerance:**
   - **STM32L4R5ZI** (120MHz Arm Cortex-M4F) handles sensor fusion, DSP calculations, and RF packet generation.
   - **TI MSP430FR5994** (16MHz, 256KB Ferroelectric Non-Volatile FRAM) serves as a 450nA watchdog supervisor, preserving mission states and re-booting the primary MCU on cold memory faults.

2. **Ultra-Low-Power Triple Power Architecture:**
   - Primary: **Lithium Thionyl Chloride ($\text{Li-SOCl}_2$)** cells operating down to **−60°C** with 730 Wh/kg specific energy.
   - Buffer: **5.0V Supercapacitor Array (25F 5.5V)** delivering 2.5A pulse bursts for Iridium 9603 satellite transmissions without dipping the primary rail.
   - Harvester: **Eccentric Pendulum Kinetic Dynamo** converting 2–5m Southern Ocean ocean swells into 350–700 mW continuous power during the 6-month dark polar winter.

3. **Sub-GHz LoRa Mesh Swarm Relay (SX1262):**
   - Buoys communicate across 15–35 km line-of-sight maritime links.
   - Elects dynamic Cluster Head to aggregate and delta-compress water column profiles into single 340-byte Iridium SBD frames, reducing satellite transmission costs by **75–85%**.

4. **Edge TinyML Frazil Ice & Stability Engine:**
   - Real-time **UNESCO 1983 dynamic freezing point** calculation based on local salinity and pressure.
   - Preemptively detects ice accretion and switches into **Ice Hold Mode** to prevent crushing damage to antennas.
   - Flags sensor drift and physical density inversions ($\partial \rho / \partial z < 0$).

5. **UHMWPE Cryo-Resilient Spar Hull:**
   - Machined Ultra-High-Molecular-Weight Polyethylene (-260°C rated) with 10x higher abrasion resistance than HDPE and self-lubricating surface to shed sea-ice adhesion.

---

## 📁 Repository Structure

```
c:\PolarMesh\
├── firmware/                   # Embedded C/C++ firmware
│   ├── include/
│   │   ├── polarmesh_config.h  # Pinout map, power thresholds, RF params
│   │   ├── polarmesh_packet.h  # Compact 340-byte SBD & LoRa packet formats
│   │   ├── sensor_manager.h    # TSYS01, MS5837, EZO-EC CTD drivers
│   │   ├── power_fsm.h         # Finite state machine & battery protection
│   │   ├── lora_mesh.h         # SX1262 LoRa mesh routing & neighbor table
│   │   ├── iridium_sbd.h       # Iridium 9603 SBD satellite transceiver
│   │   └── edge_anomaly.h      # TinyML anomaly & ice entrapment detector
│   └── src/
│       ├── main.c              # System entry point and superloop
│       ├── sensor_manager.c    # Sensor drivers & UNESCO seawater physics
│       ├── power_fsm.c         # Multi-tier power FSM & de-passivation
│       ├── lora_mesh.c         # Mesh packet forwarding & hop management
│       ├── iridium_sbd.c       # AT command engine for 9603 SBD
│       └── edge_anomaly.c      # Physics guardrails & anomaly classifier
├── ml/                         # Machine learning & TinyML models
│   ├── dataset_generator.py    # Synthetic Southern Ocean CTD profile generator
│   └── southern_ocean_ctd_dataset.json # 600 verified profiles (Weddell/Ross Sea)
├── sim/                        # Southern Ocean fleet simulation & live feed
│   ├── buoy_fleet_simulator.py # Multi-buoy drift, kinetic energy & mesh simulator
│   └── fleet_telemetry.json    # Live synchronized telemetry stream
├── hardware/                   # Electrical, mechanical & BOM documentation
│   ├── BOM_and_Power_Budget.md # Itemized component costs (₹1.44 Lakh) & energy budget
│   └── schematic_and_pinout_spec.md # Dual-MCU schematic & power rail details
├── dashboard/                  # Next.js MoES / INCOIS Command & Control Web App
└── docs/                       # Hackathon & research documentation
    ├── MoES_SIH_Presentation_Dossier.md # SIH 2026 jury evaluation dossier
    ├── polarmesh_research.md   # Comprehensive problem, landscape & science analysis
    └── polarmesh_decisions.md  # Weighted technology decision matrices (Approved)
```

---

## ⚡ Quick Start

### 1. Run Southern Ocean Buoy Fleet Simulator
```bash
python sim/buoy_fleet_simulator.py
```

### 2. Generate New CTD Profiles for Edge AI Training
```bash
python ml/dataset_generator.py
```

### 3. Launch MoES Command & Control Web Dashboard
```bash
cd dashboard
npm run dev
```

---

## 👥 Smart India Hackathon 2026 Team Allocation
- **Firmware & Embedded Systems:** Dual-MCU architecture, FreeRTOS, sensor drivers
- **Hardware & Power Electronics:** Supercapacitor pulse buffer, de-passivation circuit, PCB layout
- **RF & Mesh Communications:** SX1262 Sub-GHz protocol, Iridium 9603 SBD compression
- **Mechanical & Hydrodynamics:** UHMWPE hull machining, ballast self-righting keel, O-ring sealing
- **Cloud & Oceanographic Software:** MoES / INCOIS live telemetry dashboard, polar map
- **Edge AI & Data Analytics:** TinyML autoencoder, UNESCO freezing algorithms, validation
