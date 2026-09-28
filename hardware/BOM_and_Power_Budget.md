# PolarMesh (SIH26065) - Bill of Materials (BOM) & Power Budget

**Problem Statement:** SIH26065 — Autonomous Low-Cost Ocean Observation Platform for Polar & Southern Oceans  
**Sponsor:** Ministry of Earth Sciences (MoES) / INCOIS  
**System Revision:** REV_B_UHMWPE (Target Field Unit)

---

## 1. Comprehensive Bill of Materials (BOM)

| Subsystem | Component / Part Number | Description & Source | Unit Cost (INR) | Unit Cost (USD) | Status |
|---|---|---|---|---|---|
| **Hull & Mechanics** | UHMWPE Machined Spar & Sphere (12mm wall) | Ultra-High-Molecular-Weight Polyethylene (-260°C rated, self-lubricating, zero embrittlement) | ₹22,000 | $265 | Sourced / Custom CNC |
| | Internal 6061-T6 Anodized Skeleton | Structural chassis & battery cradle | ₹6,500 | $78 | Sourced / Local Fab |
| | EPDM Cold-Resistant O-Rings (Shore 70A) | Dual redundant hermetic sealing | ₹800 | $10 | Off-the-shelf |
| | SS316 Marine Hardware & Ballast Keel | 4.5kg counterweight for self-righting | ₹3,200 | $38 | Local Marine |
| **Compute & Logic** | STM32L4R5ZI MCU Board | 120MHz Arm Cortex-M4, 2MB Flash, 640KB RAM, Stop2 5µA | ₹2,800 | $34 | Mouser / DigiKey |
| | TI MSP430FR5994 Coprocessor | 256KB non-volatile FRAM, ultra-low-power supervisor (450nA) | ₹1,400 | $17 | TI Store / Mouser |
| | Custom 4-Layer PCB Assembly | FR-4 with conformal coating (Dow Corning 1-2577) | ₹4,500 | $54 | JLCPCB / Local PCBA |
| **Sensors** | Atlas Scientific EZO-EC + K 10.0 Probe | Conductivity & Salinity circuit (I2C) | ₹16,500 | $198 | Robu / DigiKey |
| | TE Connectivity TSYS01 | High-precision digital temperature (±0.001°C resolution) | ₹2,200 | $26 | Mouser |
| | Blue Robotics MS5837-30BA | 30-bar submersible hydrostatic pressure & depth sensor | ₹7,500 | $90 | Robu / BlueRobotics |
| | TDK InvenSense ICM-42688-P | 6-axis low-noise IMU (wave height & period estimation) | ₹1,100 | $13 | DigiKey |
| | u-blox MAX-M10Q GNSS | GPS/Galileo/BeiDou ultra-low power receiver | ₹3,200 | $38 | Robu |
| **Telemetry** | Iridium 9603 SBD Transceiver | Global pole-to-pole satellite transceiver | ₹28,000 | $337 | Ground Control |
| | Iridium Passive Patch Antenna | High-gain circularly polarized polar antenna | ₹3,800 | $46 | Taoglas / Mouser |
| | Semtech SX1262 LoRa Transceiver | 868MHz +22dBm, -148dBm sensitivity | ₹1,800 | $22 | Waveshare |
| | 868MHz High-Gain Marine Whip Antenna | Fiber-reinforced polymer radome | ₹1,200 | $14 | Local RF |
| **Power System** | Saft LS33600 D-Cell Li-SOCl2 (4x 17Ah Pack) | Primary battery (3.6V, 68Ah total = 244.8 Wh, -60°C rated) | ₹14,000 | $168 | Saft Dist. |
| | Eaton XL60 Supercapacitor Pack (2x 50F, 3.0V in series) | 25F 5.5V burst buffer for 2.5A Iridium transmission surges | ₹3,200 | $38 | DigiKey |
| | Internal Pendulum Harvester (Piezo/DC Generator) | Kinetic ocean wave oscillation energy scavenger (~400mW) | ₹5,500 | $66 | Custom prototype |
| | SunPower Gen III Flexible Monocrystalline Solar (15W) | Polar summer supplemental charging | ₹3,500 | $42 | Solbian / Robu |
| | TI BQ25570 Nano-power Boost Harvester | Energy harvesting & power rail switching IC | ₹1,600 | $19 | TI Store |
| **Total System Cost** | | | **₹1,44,800** | **$1,744** | **12x Cheaper than Argo ($20k)** |

---

## 2. 24-Hour Energy Budget Analysis

| Operating Mode | Subsystems Active | Duration per 24h | Current (mA @ 3.6V) | Power (mW) | Energy (mWh/day) |
|---|---|---|---|---|---|
| **Deep Sleep (Stop2)** | MSP430FR5994 RTC + STM32 Stop2 + Power switches off | 22 hours 45 min | 0.025 mA | 0.09 mW | **2.05 mWh** |
| **Sensor Cast (24 casts/day)** | STM32 @ 24MHz, MS5837, TSYS01, EZO-EC active | 24 min (1 min/cast) | 28 mA | 100.8 mW | **40.32 mWh** |
| **Edge TinyML Inference** | STM32 @ 120MHz Cortex-M4 DSP processing profile | 4.8 min (12 sec/cast) | 45 mA | 162.0 mW | **12.96 mWh** |
| **LoRa Mesh Broadcast** | SX1262 RX listen + 12 packet relays @ +22dBm | 40 min | 35 mA (avg) | 126.0 mW | **84.00 mWh** |
| **Iridium SBD Uplink** | 4 satellite passes/day (Supercap buffer discharges 2.5A) | 6.2 min (90s/pass) | 350 mA (avg from primary) | 1,260 mW | **130.20 mWh** |
| **Total Daily Consumption** | | **24.0 Hours** | | | **269.53 mWh/day** |

### Energy Harvest (Positive Energy Balance in Southern Ocean):
- **Average wave kinetic generation:** 350 mW continuous in Southern Ocean 2.5m swells
- **Assuming 20% conversion & storage efficiency:** $350 \text{ mW} \times 0.20 \times 24 \text{ h} = \mathbf{1,680 \text{ mWh/day}}$
- **Net Daily Energy Surplus:** $+1,410 \text{ mWh/day}$  
- **Primary Battery Lifetime (Zero Harvesting Backup):**
  $$\text{Days} = \frac{244,800 \text{ mWh (Saft Pack)}}{269.53 \text{ mWh/day}} = \mathbf{908 \text{ Days (~2.5 Years continuous autonomous mission)}}$$
