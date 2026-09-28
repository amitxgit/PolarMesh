# PolarMesh (SIH26065) - MoES & INCOIS Technical Presentation Dossier
## Autonomous Low-Cost Ocean Observation Platform for Polar and Southern Oceans
**Smart India Hackathon 2026 | Hardware Category | Ministry of Earth Sciences (MoES)**

---

## 1. Executive Summary

The Polar and Southern Oceans represent the Earth's primary thermodynamic sink and the engine of global thermohaline circulation. However, subsea and surface observation in these regions remains critically sparse due to extreme conditions: sea surface temperatures down to −2°C, multi-meter sea-ice crushing pressures, violent Antarctic Circumpolar Current (ACC) seas, and prolonged polar winters devoid of sunlight.

Existing global observation assets (such as standard Argo floats, Ice-Tethered Profilers (ITP), and moored arrays) suffer from major shortcomings:
- **Prohibitive Cost:** Standard core Argo floats cost **₹15 Lakh to ₹25 Lakh ($18,000 to $30,000)** each; biogeochemical and ice-capable floats exceed **₹60 Lakh to ₹1 Crore ($70,000 to $120,000)**.
- **Ice Crushing / Loss:** Conventional buoyant floats that attempt to surface under sea ice crush their optical and satellite antenna radomes, resulting in total loss of asset.
- **Battery Freezing:** Conventional Li-ion and alkaline battery chemistries suffer massive capacity collapse (>70% drop) below 0°C, and cannot be recharged below freezing without dendritic short-circuits.
- **Telemetry Blindspots:** Standard floats rely exclusively on direct satellite uplinks, which are completely blocked when trapped beneath sea-ice sheets or in heavy blizzard conditions.

**PolarMesh** directly addresses Problem Statement **SIH26065** by introducing an indigenous, ultra-low-cost (**₹1.44 Lakh / ~$1,740**), swarm-capable autonomous drifter platform engineered specifically for the extreme conditions of the Antarctic Weddell & Ross Seas.

---

## 2. Key Architectural Innovations

```
 +-------------------------------------------------------------------------+
 |                            POLARMESH PLATFORM                           |
 +-------------------------------------------------------------------------+
 |                                                                         |
 |  [UHMWPE Cryo-Hull]           [Dual-MCU Core]      [Triple Power Tier]  |
 |  - Tough down to -260°C       - STM32L4R5ZI (120M) - Li-SOCl2 Primary   |
 |  - Self-lubricating ice-slip  - MSP430FR5994 FRAM  - Supercapacitor 5V  |
 |  - 10x abrasion vs HDPE       - 450nA Sleep Guard  - Kinetic Pendulum   |
 |                                                                         |
 |  [Hybrid Comms Mesh]          [Ocean Sensor Suite] [Edge TinyML Engine] |
 |  - SX1262 LoRa Swarm Relay    - EZO-EC (Salinity)  - UNESCO Freezing Eq |
 |  - Iridium 9603 Pole-to-Pole  - TSYS01 (±0.001°C)  - Frazil Ice Sensing |
 |  - Sub-GHz Cooperative Uplink - MS5837-30BA Depth  - Density Stability  |
 |                                                                         |
 +-------------------------------------------------------------------------+
```

### 1. Dual-MCU Cryogenic Fault Tolerance
A dual-MCU layout pairs an Arm Cortex-M4 (STM32L4R5ZI) with an ultra-low-power Texas Instruments MSP430FR5994 ferroelectric RAM (FRAM) supervisor:
- MSP430 consumes only **450 nA** in standby while maintaining real-time system state in non-volatile FRAM.
- If the primary MCU encounters a brownout, freezing glitch, or memory fault at −40°C, the supervisor cleanly resets it and logs the event without losing mission logs or calibration tables.

### 2. Multi-Tier Cold Battery & Wave Kinetic Harvesting
- **Primary Power:** High-density Lithium Thionyl Chloride ($\text{Li-SOCl}_2$) cells rated for continuous discharge at **−60°C to +85°C** with **730 Wh/kg** energy density.
- **De-Passivation Circuit:** An automated MOSFET-switched resistive load pulse safely removes the insulating $\text{LiCl}$ crystal layer that forms during prolonged polar storage.
- **Supercapacitor 5V Burst Buffer:** Two 50F series electric double-layer capacitors absorb the heavy 2.5A pulse required for Iridium satellite bursts, eliminating voltage drop across the cold battery.
- **Pendulum Kinetic Harvester:** A sealed eccentric pendulum dynamo harvests the relentless kinetic motion of 2-5m Southern Ocean swells, generating **350–700 mW** continuous power even through the 6-month dark polar winter.

### 3. Cooperative LoRa Mesh Swarm Relay
Rather than forcing every individual buoy to burn heavy battery power on high-cost satellite transmissions:
- Up to 8 buoys form a dynamic **SX1262 Sub-GHz LoRa mesh** (868 MHz ISM / MoES frequencies) across 15–35 km maritime line-of-sight hops.
- The buoy with the strongest supercapacitor charge and clear sky view is elected **Cluster Head**, aggregating and delta-compressing the water column profiles of neighboring buoys into a single 340-byte Iridium SBD transmission.
- **Cost Reduction:** Reduces monthly Iridium satellite transmission costs by **75% to 85%**.

### 4. Edge TinyML Ice Entrapment & Sensor Anomaly Detection
Embedded physics-informed algorithms run on the Cortex-M4 DSP:
- **UNESCO Dynamic Freezing Calculation:** Continuously calculates seawater freezing point $T_f$ as a function of instantaneous salinity and pressure:
  $$T_f = -0.0575 \cdot S + 1.710523 \times 10^{-3} \cdot S^{1.5} - 2.154996 \times 10^{-4} \cdot S^2 - 7.53 \times 10^{-4} \cdot P$$
- **Ice Accretion Warning:** When water temperature approaches within 0.15°C of $T_f$ and IMU tilt damping indicates frazil ice adhesion, the buoy preemptively enters **Ice Hold Mode**—retracting antennas and preventing destructive ice-crushing attempts.
- **Water Column Stability Validation:** Automatically flags biofouling drift or internal density inversion anomalies before data is uplinked.

---

## 3. Financial & Indigenous Impact (Make in India)

| Metric | Conventional Imported Profiler (Argo/ITP) | PolarMesh Platform |
|---|---|---|
| **Unit Capital Cost** | ₹18,00,000 – ₹25,00,000 | **₹1,44,800 (~$1,740)** |
| **Fleet Deployment (10 Units)** | ₹2.0 – ₹2.5 Crore | **₹14.4 Lakh** |
| **Monthly Satellite Data Cost** | ₹8,500 / buoy | **₹1,800 / buoy (via LoRa Mesh aggregation)** |
| **Cold Survival Limit** | −5°C to 0°C | **−60°C with UHMWPE hull & Li-SOCl2** |
| **Under-Ice Strategy** | None (Risk of crushing & antenna loss) | **Automated Ice Hold & Swarm Acoustic/Relay** |
| **Indigenous Component Content** | 0% (Fully imported from US/France) | **>70% fabricated and assembled in India** |

---

## 4. Live Demonstration Strategy for SIH 2026 Grand Finale

1. **Physical Prototype & Ice Water Immersion Test:**
   - PolarMesh buoy prototype placed in a temperature-controlled cold saline water tank (−1.5°C with ice blocks).
   - Real-time demonstration of TSYS01 temperature, MS5837 pressure, and Atlas Scientific salinity acquisition.
2. **LoRa Mesh Swarm Relay Live Demo:**
   - Two hardware nodes communicating over SX1262 LoRa: Node B in the cold tank transmits its CTD cast to Node A (acting as gateway).
3. **MoES / INCOIS Command & Control Web Dashboard:**
   - Live interactive polar stereographic map showing Southern Ocean buoy drift tracks, real-time CTD water column stratification graphs, battery & supercap health, and live Iridium SBD packet decoders.
4. **Triggered Ice-Lock Demonstration:**
   - Freezing cold stimulus injected to trigger the Edge TinyML ice-lock algorithm, displaying instant emergency alerts on the dashboard and switching the hardware into protected Ice-Hold mode.
