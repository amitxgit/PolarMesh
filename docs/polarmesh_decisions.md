# 🔬 PolarMesh — Technology Decision Matrix
## Every Choice Challenged, Every Alternative Weighed

---

> [!IMPORTANT]
> This document critically re-evaluates **every** technology decision from the original research. Each subsystem goes through a rigorous alternatives analysis with weighted scoring. Revised decisions are marked with 🔄.

---

## Decision 1: Main Microcontroller (MCU)

### Alternatives Considered

| MCU | Architecture | Deep Sleep | Active (max) | Flash/RAM | FPU/DSP | Price (₹) | Ecosystem |
|-----|-------------|-----------|-------------|-----------|---------|-----------|-----------|
| **STM32L4R5ZI** | Cortex-M4F | 0.03 µA (Shutdown) | 100 MHz | 2MB/640KB | ✅/✅ | ~₹600 | STM32CubeIDE, HAL |
| **MSP430FR5994** | MSP430X | 0.4 µA (LPM3.5) | 16 MHz | 256KB FRAM/8KB | ❌/❌ | ~₹350 | Energia, CCS |
| **nRF52840** | Cortex-M4F | 1.5 µA (System OFF) | 64 MHz | 1MB/256KB | ✅/❌ | ~₹400 | Zephyr, nRF SDK |
| **RP2040** | Cortex-M0+ ×2 | 180 µA (Dormant) | 133 MHz | External/264KB | ❌/❌ | ~₹100 | MicroPython, C SDK |
| **ESP32-S3** | Xtensa LX7 ×2 | 7 µA (Deep Sleep) | 240 MHz | 8MB ext/512KB | ❌/✅ | ~₹300 | Arduino, ESP-IDF |

### Scoring Matrix (1-5, weighted)

| Criterion | Weight | STM32L4R5 | MSP430FR | nRF52840 | RP2040 | ESP32-S3 |
|-----------|--------|-----------|----------|----------|--------|----------|
| Ultra-low sleep current | 25% | 5 | 5 | 4 | 1 | 2 |
| Processing power (TinyML) | 20% | 5 | 1 | 3 | 3 | 4 |
| Peripheral richness (ADC, SPI, I2C, UART) | 15% | 5 | 3 | 4 | 4 | 4 |
| Ecosystem/documentation | 15% | 5 | 3 | 4 | 5 | 5 |
| Cost | 10% | 3 | 4 | 4 | 5 | 4 |
| Cold temperature range | 10% | 5 (−40 to 85°C) | 5 (−40 to 85°C) | 4 (−40 to 85°C) | 3 (−20 to 85°C) | 3 (−40 to 85°C) |
| TinyML framework support | 5% | 5 (Cube.AI) | 1 | 3 | 2 | 4 |
| **Weighted Total** | | **4.70** | **3.15** | **3.70** | **2.95** | **3.55** |

### 🔄 Revised Decision: **DUAL MCU Architecture**

**Primary: STM32L4R5ZI** (unchanged — dominant winner)
**Watchdog/Safety: MSP430FR5994** as ultra-low-power watchdog + RTC + failsafe data logger

**Why dual?**
- STM32L4 handles heavy lifting: sensor fusion, TinyML inference, comms protocol
- MSP430FR provides independent watchdog — if STM32 crashes (cosmic ray, firmware bug), MSP430 wakes it
- MSP430's FRAM retains last-known-good state even with total power loss (no battery needed to retain data)
- This is exactly how Argo floats work: critical systems have watchdog redundancy

**Why NOT nRF52840?** BLE is useless at sea. Its radio advantage is wasted in our architecture.
**Why NOT RP2040?** 180µA deep sleep is unacceptable. Would drain battery 600× faster than STM32L4.
**Why NOT ESP32-S3?** 7µA sleep is 230× worse than STM32L4's shutdown. WiFi/BLE useless at sea.

---

## Decision 2: Satellite Communication

### Alternatives Considered

| Technology | Polar Coverage | Msg Size | Power (TX) | Module Cost | Per-Msg Cost | Bidirectional |
|-----------|---------------|----------|-----------|-------------|-------------|---------------|
| **Iridium SBD (9603N)** | ✅ Pole-to-pole | 340 bytes | 1.5W burst | ₹15,000 | ~₹8–12/msg | ✅ |
| **Iridium Certus 100** | ✅ Pole-to-pole | Larger | 2–3W | ₹25,000+ | Higher | ✅ |
| **Globalstar SIMPLEX** | ❌ (max ±70° lat) | 9 bytes | 0.5W | ₹5,000 | ~₹1–3/msg | ❌ |
| **Kinéis** | ⚠️ Limited polar | 26 bytes | 0.5W | ₹8,000 | TBD | ❌ |
| **Swarm/SpaceBee** | ❌ Defunct (Mar 2025) | — | — | — | — | — |

### Verdict: **Iridium SBD 9603N (RockBLOCK)** — NO CHANGE ✅

> [!CAUTION]
> **There is literally no alternative for polar coverage.** Globalstar stops at ±70° latitude. Swarm is dead. Kinéis has incomplete polar coverage. Iridium is the ONLY satellite constellation with true pole-to-pole coverage via cross-linked LEO satellites.

**This is a non-negotiable constraint, not a design choice.**

**Improvement:** Use Iridium **IMT (Messaging Transport)** protocol instead of legacy DirectIP for cloud-native MQTT integration. Same hardware (9603N), just better cloud-side routing.

---

## Decision 3: Inter-Buoy Communication (Mesh)

### Alternatives Considered

| Technology | Range (over water) | Power (TX) | Data Rate | Frequency | Cost | Mesh Support |
|-----------|-------------------|-----------|-----------|-----------|------|-------------|
| **LoRa SX1276** | 10–15 km | 100 mW | 5.5 kbps | 868/915 MHz | ₹500 | Via firmware |
| **LoRa SX1262** | 12–20 km | 80 mW (lower RX) | 5.5 kbps | 868/915 MHz | ₹600 | Via firmware |
| **Meshtastic (SX1262)** | 12–20 km | 80 mW | Low | 868/915 MHz | ₹1,500 (dev board) | ✅ Built-in |
| **UHF Radio (AX5043)** | 5–10 km | 150 mW | 9.6 kbps | 433 MHz | ₹800 | Custom |
| **nRF9160 (LTE-M)** | Cellular range | 200 mW | 300 kbps | Licensed | ₹2,000 | ❌ |

### 🔄 Revised Decision: **SX1262 (NOT SX1276)** + Meshtastic Protocol

**What changed:** SX1276 → **SX1262**

**Why?**
- **57% lower receive current** (4.6 mA vs 10.8 mA) — receive mode is where LoRa spends most time in mesh
- **Better sensitivity** (−148 dBm vs −137 dBm) — 11 dB improvement = roughly **3.5× longer range** or much more reliable at same distance
- **Higher TX power** (+22 dBm vs +20 dBm)
- SX1276 is a 2013 chip. SX1262 is its direct replacement. No reason to use the older part.

**Why Meshtastic protocol?** Open-source, battle-tested mesh routing, automatic hop management, encrypted. Don't reinvent mesh networking — use what works.

**Why NOT LTE-M?** No cell towers in the Southern Ocean. Dead on arrival.

---

## Decision 4: Battery Chemistry

### Alternatives Considered

| Chemistry | Type | Energy Density | Temp Range | Self-Discharge | Rechargeable | Cost (20Ah equiv) |
|-----------|------|---------------|-----------|---------------|-------------|-------------------|
| **LiFePO4** | Secondary | 90–120 Wh/kg | −20 to 60°C (needs heating below 0°C) | 3–5%/month | ✅ | ₹5,000 |
| **Li-SOCl₂ (Lithium Thionyl Chloride)** | Primary | 500–730 Wh/kg | **−60 to 85°C** | <1%/year | ❌ | ₹12,000 |
| **Li-MnO₂** | Primary | 280 Wh/kg | −40 to 60°C | <1%/year | ❌ | ₹8,000 |
| **Sodium-Ion** | Secondary | 100–160 Wh/kg | −30 to 60°C | Moderate | ✅ | ₹4,000 (limited avail.) |
| **Li-Ion (18650)** | Secondary | 250 Wh/kg | −20 to 60°C | 2–3%/month | ✅ | ₹3,000 |

### 🔄 MAJOR Revision: **HYBRID Battery Architecture**

**Original:** LiFePO4 only
**Revised:** **Li-SOCl₂ primary (main) + small LiFePO4 rechargeable (buffer)**

```
┌─────────────────────────────────────────────────────┐
│              HYBRID POWER ARCHITECTURE               │
│                                                       │
│  ┌──────────────────┐    ┌──────────────────────┐   │
│  │  Li-SOCl₂ Pack   │───►│  Small LiFePO4 Pack  │   │
│  │  (Primary, 15Ah) │    │  (Buffer, 3Ah)       │   │
│  │  "Cold-proof"    │    │  "Rechargeable"      │   │
│  │  −60°C rated     │    │  Fed by wave/solar   │   │
│  │  730 Wh/kg       │    │  Handles pulse loads  │   │
│  └──────────────────┘    └──────────────────────┘   │
│           │                       ▲                   │
│           ▼                       │                   │
│  ┌──────────────────┐    ┌──────────────────────┐   │
│  │  Supercap bank   │    │  Wave + Solar         │   │
│  │  (Pulse buffer   │    │  Energy Harvesting    │   │
│  │   for Iridium)   │    │  → Charges LiFePO4   │   │
│  └──────────────────┘    └──────────────────────┘   │
└─────────────────────────────────────────────────────┘
```

**Why this is MUCH better than LiFePO4-only:**

| Problem with LiFePO4-only | Hybrid solution |
|---------------------------|----------------|
| Cannot charge below 0°C → needs heating → wastes energy | Li-SOCl₂ works natively at −60°C, no heating needed |
| 90-120 Wh/kg energy density | Li-SOCl₂ at 730 Wh/kg = **6× more energy per kg** |
| 3-5%/month self-discharge | Li-SOCl₂ at <1%/year = **36-60× lower self-discharge** |
| Needs BMS heating overhead | Primary battery needs zero thermal management |
| Single point of failure | Dual chemistry = redundancy |

**The LiFePO4 buffer (3Ah) handles:**
- High-current pulses (Iridium TX = 1.5W, too much for Li-SOCl₂ without HLC)
- Stores harvested wave/solar energy
- Inside insulated compartment where electronics self-heat it

**Supercapacitor bank (2× 10F):** Absorbs Iridium TX pulse current spikes without stressing either battery.

---

## Decision 5: Energy Harvesting

### Alternatives Considered

| Technology | Avg Output (ocean buoy) | Complexity | Cold Performance | Cost | Reliability |
|-----------|------------------------|-----------|-----------------|------|-------------|
| **Electromagnetic Pendulum** | 50–500 mW | Medium | ✅ Works in any temp | ₹3,000–5,000 | High (mechanical wear) |
| **Piezoelectric (PENG)** | 1–10 mW | Low | ✅ | ₹1,000–2,000 | Very High (no moving parts) |
| **Triboelectric (TENG)** | 5–50 mW | Medium | ⚠️ Humidity sensitive | ₹2,000–4,000 | Medium |
| **Solar Panel** | 0–5W (seasonal) | Low | ✅ | ₹1,500–2,000 | High |
| **Thermoelectric (TEG)** | 1–20 mW | Low | ✅ Needs temp gradient | ₹2,000–3,000 | Very High |

### 🔄 Revised Decision: **Electromagnetic Pendulum (primary) + Solar (secondary) + drop Piezoelectric**

**What changed:** Removed TENG and TEG from consideration.

**Why electromagnetic pendulum wins for primary:**
- Ocean waves provide **continuous** motion. Pendulum captures this reliably.
- Output (50-500 mW avg) matches or exceeds our daily consumption (~940 mWh/day ÷ 24h ≈ 39 mW avg)
- Proven in commercial ocean buoys (not experimental like TENG)
- Works in any temperature — no humidity or temperature gradient dependency

**Why drop TENG?** Humidity at sea is 95-100% — triboelectric generators are extremely sensitive to humidity (surface charge dissipation). Not reliable in marine environment.

**Why drop TEG?** The temperature gradient between ocean water (~0°C) and buoy interior (maybe +5°C from electronics) is only ~5°C. At Seebeck efficiency, this produces <5 mW. Not worth the component cost and complexity.

**Solar stays as summer supplement** — during polar summer (24h daylight for months), even a small 2W panel provides excellent supplementary charging. During polar winter, it contributes nothing, and that's OK because the Li-SOCl₂ primary carries the load.

---

## Decision 6: Conductivity/Salinity Sensor

### Alternatives Considered

| Sensor Type | Method | Power | Fouling Resistance | DIY Feasibility | Accuracy | Cost |
|-------------|--------|-------|--------------------|----------------|----------|------|
| **Custom inductive coil** | Toroidal induction | 10 mW | ✅ Excellent | ❌ Very Hard | ±0.01 mS/cm (if built well) | ₹2,000 (materials) |
| **Atlas Scientific EZO-EC** | Resistive (K 10.0) | 15 mW | ⚠️ Medium | ✅ Easy (I2C) | ±2% | ₹6,000 |
| **OpenCTD custom** | Resistive | 10 mW | ⚠️ Medium | ⚠️ Medium | ±5% | ₹1,500 |
| **Aanderaa 4319** | Inductive | 5 mW | ✅ Excellent | ❌ (commercial) | ±0.003 mS/cm | ₹2,00,000+ |

### 🔄 Revised Decision: **Atlas Scientific EZO-EC (K 10.0) for prototype** + custom inductive as v2 stretch goal

**What changed:** Dropped "custom inductive coil" from the initial build.

**Why?**
- Building a precision inductive conductivity sensor from scratch requires precision-wound toroidal coils, careful electromagnetic shielding, and complex lock-in amplifier signal processing
- This alone could consume the entire hackathon timeline
- Atlas Scientific EZO-EC is a proven, calibrated, I2C-interfaced module that gives us reliable data *right now*
- For the SIH demo, reliable data from a known-good sensor is far more impressive than a half-working custom sensor
- Custom inductive sensor remains a clear v2 goal that adds genuine innovation value

**Anti-fouling for resistive sensor:** Copper tape + mechanical wiper on a micro-servo (activated once per day).

---

## Decision 7: Pressure/Depth Sensor

### Alternatives Considered

| Sensor | Depth Rating | Interface | Long-term Immersion | Accuracy | Cost |
|--------|-------------|-----------|--------------------|---------|----- |
| **MS5837-30BA (Bar30)** | 300m | I2C | ⚠️ Needs maintenance | 0.2% FS | ₹1,500 |
| **MS5837-02BA (Bar02)** | 20m | I2C | ⚠️ Needs maintenance | 0.01% FS | ₹1,800 |
| **BlueRobotics BarXT** | 300m | I2C | ✅ Designed for long-term | 0.1% FS | ₹5,000 |
| **MS5803-14BA** | 140m | I2C/SPI | ❌ Needs custom housing | 0.1% FS | ₹2,000 |
| **Keller PA-7** | 300m+ | 4-20mA | ✅ Industrial grade | 0.05% FS | ₹15,000+ |

### 🔄 Revised Decision: **MS5837-30BA for prototype, BarXT for field version**

**Why MS5837-30BA for now:**
- Proven, cheap, available — BlueRobotics ships globally
- Our prototype won't be submerged for months (it's a hackathon demo)
- I2C interface is simple to integrate

**Why BarXT for field deployment:**
- Designed specifically for extended submersion (months/years)
- Titanium + epoxy construction survives polar conditions
- Same I2C interface — firmware change = zero

---

## Decision 8: Hull Material

### Alternatives Considered

| Material | Impact Resistance | Cold Performance | Machinability | UV Stability | Density | Cost |
|----------|------------------|-----------------|---------------|-------------|---------|------|
| **HDPE** | Good | Good (−46°C) | ✅ Easy | ✅ (with stabilizer) | 0.95 g/cm³ | Low |
| **UHMWPE** | **Excellent** | **Excellent** (−260°C) | ⚠️ Harder | ✅ | 0.93 g/cm³ | Medium |
| **Glass Sphere** | ❌ Brittle | ✅ | ❌ | ✅ | Variable | High |
| **Titanium** | ✅ Excellent | ✅ | ❌ Expensive | ✅ | 4.5 g/cm³ (sinks) | Very High |
| **Delrin (POM)** | Good | Good (−40°C) | ✅ Easy | ⚠️ Degrades | 1.41 g/cm³ (sinks) | Medium |

### 🔄 Revised Decision: **UHMWPE outer hull** (was HDPE)

**What changed:** HDPE → UHMWPE

**Why upgrade?**
- UHMWPE has **10× better abrasion resistance** than HDPE — critical when grinding against sea ice floes
- Retains impact toughness down to −260°C (HDPE only −46°C) — UHMWPE wins by a massive margin
- Lighter than HDPE (0.93 vs 0.95 g/cm³) — more buoyancy
- Self-lubricating — ice slides off rather than gripping
- Only moderately more expensive than HDPE

**For SIH prototype:** 3D-printed PETG shell (for speed) with UHMWPE documented as the production material.

---

## Decision 9: TinyML Framework

### Alternatives Considered

| Framework | Learning Curve | STM32 Support | Model Optimization | End-to-End | Output |
|-----------|---------------|--------------|-------------------|-----------|--------|
| **TensorFlow Lite Micro** | Medium | ✅ | Manual quantization | ❌ (code-only) | C library |
| **Edge Impulse** | Low | ✅ | ✅ EON Compiler | ✅ (data → deploy) | C++ library |
| **STM32Cube.AI** | Medium | ✅✅ (optimized) | ✅ Auto-compression | ⚠️ (inference only) | Optimized C |
| **microTVM** | High | ⚠️ | ✅ Auto-tuning | ❌ | C library |

### 🔄 Revised Decision: **Edge Impulse (training) → STM32Cube.AI (deployment)**

**What changed:** TF Lite Micro → **Edge Impulse + STM32Cube.AI pipeline**

**Why?**
1. **Edge Impulse** handles the entire training pipeline: data collection, feature engineering, AutoML model selection, INT8 quantization — all through a web GUI
2. **STM32Cube.AI** takes the Edge Impulse model and generates STM32-optimized C code that leverages CMSIS-NN hardware acceleration on our Cortex-M4
3. This combo gives us the fastest development time (Edge Impulse) with the best on-target performance (Cube.AI)
4. TF Lite Micro is lower-level — we'd spend weeks on quantization and optimization that Edge Impulse does automatically
5. **Edge Impulse's EON Compiler** can reduce model RAM usage by 50-75% compared to vanilla TFLM

**Models to deploy:**
| Model | Purpose | Estimated Size |
|-------|---------|---------------|
| Autoencoder (anomaly detection) | Flag unusual sensor readings | ~15KB Flash, ~4KB RAM |
| Temp gradient classifier (ISA) | Ice detection from subsurface temp profile | ~8KB Flash, ~2KB RAM |
| Adaptive sampling regressor | Predict optimal sample rate | ~5KB Flash, ~1KB RAM |

---

## Decision 10: Cloud / Dashboard Stack

### Alternatives Considered

| Stack | Complexity | Real-time | Cost | SIH Demo Suitability |
|-------|-----------|-----------|------|---------------------|
| **AWS IoT Core + DynamoDB + Grafana** | High | ✅ | $$ (free tier limited) | ❌ Overkill for demo |
| **Firebase Realtime DB + Cloud Functions** | Low | ✅ | Free tier generous | ✅ |
| **Supabase + custom frontend** | Medium | ✅ | Free tier generous | ✅ |
| **Custom MQTT broker + TimescaleDB + Next.js** | High | ✅ | $$ | ❌ Too complex for timeline |
| **ThingSpeak (MathWorks)** | Very Low | ✅ | Free | ⚠️ Limited customization |

### 🔄 Revised Decision: **Firebase Realtime DB + Next.js dashboard** (was full AWS stack)

**What changed:** Dropped AWS IoT Core / TimescaleDB / heavyweight architecture

**Why Firebase for SIH:**
- **Free tier is generous** — we don't need to worry about cloud costs during development
- **Realtime Database** gives us WebSocket-based live updates without setting up MQTT brokers
- **Cloud Functions** (Node.js) can handle Iridium webhook ingestion
- **Hosting** for the dashboard is included
- Setup time: hours, not days

**Dashboard stack (for demo):**
- **Next.js** with **Mapbox GL JS** — interactive map showing buoy positions
- **Recharts** — real-time sensor data plots (T, S, P, depth profiles)
- **Firebase Realtime DB** — live data sync
- **Simulated Iridium** — for demo, use MQTT to simulate satellite messages

**Post-SIH migration path:** Move to AWS IoT Core + TimescaleDB when deploying real Iridium hardware. The frontend doesn't change.

---

## Summary of All Revisions

| Subsystem | Original Choice | Revised Choice | Why Changed |
|-----------|----------------|---------------|-------------|
| MCU | STM32L4R5 only | **STM32L4R5 + MSP430FR watchdog** | Redundancy for polar reliability |
| Satellite | Iridium SBD | **Iridium SBD** (no change possible) | Only option for polar |
| Inter-buoy | LoRa SX1276 | **LoRa SX1262 + Meshtastic** | 57% lower RX current, 11dB better sensitivity |
| Battery | LiFePO4 only | **Li-SOCl₂ + LiFePO4 hybrid** | 6× energy density, works at −60°C natively |
| Energy harvesting | Wave + Solar + TENG | **Wave + Solar only** | TENG fails in high humidity, TEG too low output |
| Conductivity | Custom inductive | **Atlas Scientific EZO-EC** | Realistic for hackathon; custom inductive is v2 |
| Pressure | MS5837-30BA | **MS5837 (proto) / BarXT (field)** | BarXT for long-term submersion |
| Hull | HDPE | **UHMWPE** | 10× abrasion resistance, works to −260°C |
| TinyML | TF Lite Micro | **Edge Impulse + STM32Cube.AI** | Faster development, better optimization |
| Cloud | AWS IoT + TimescaleDB | **Firebase + Next.js** | Simpler for SIH demo, free tier |

---

## Revised BOM (Post-Analysis)

| Category | Components | Original (₹) | Revised (₹) | Change |
|----------|-----------|-------------|-------------|--------|
| **Sensors** | EZO-EC, MS5837, PT1000, GPS, IMU, BME280, optical, acoustic | 8,400 | 14,000 | ↑ (EZO-EC costs more but works) |
| **Compute** | STM32L4R5 + MSP430FR + ESP32-S3 + PCBs | 5,000 | 6,500 | ↑ (added watchdog MCU) |
| **Communication** | RockBLOCK 9603N + LoRa SX1262 + Acoustic modem | 19,000 | 19,500 | ≈ same |
| **Power** | Li-SOCl₂ pack + LiFePO4 buffer + supercaps + BMS + wave harvester + solar | 18,000 | 25,000 | ↑ (hybrid battery) |
| **Structure** | UHMWPE hull, seals, connectors, antennas | 15,000 | 18,000 | ↑ (UHMWPE > HDPE) |
| **Miscellaneous** | Wiring, insulation, anti-fouling, conformal coating | 5,000 | 5,000 | = |
| **Assembly & Testing** | Labor, calibration, 3D printing | 10,000 | 10,000 | = |
| **Contingency (15%)** | | 12,000 | 14,700 | ↑ |
| **TOTAL** | | **₹92,400** | **₹1,12,700 (~$1,350)** | ↑20% |

> [!TIP]
> **Even at the revised ₹1.13 Lakh, PolarMesh is still 11–18× cheaper than an Argo float ($15,000–$25,000).** The 20% cost increase buys us dramatically better cold performance (Li-SOCl₂), more reliable sensors (Atlas Scientific), and longer hull life (UHMWPE).

---

## What To Build First (Priority Order)

1. **Firmware scaffold** — FreeRTOS on STM32L4, sensor driver framework, power management FSM
2. **Cloud dashboard** — Firebase + Next.js + Mapbox (can develop in parallel with firmware)
3. **LoRa mesh prototype** — Two SX1262 modules talking Meshtastic
4. **Sensor integration** — Wire up EZO-EC, MS5837, PT1000, BME280 on breadboard
5. **Edge AI pipeline** — Edge Impulse project with simulated oceanographic data
6. **Power system** — BMS board design, wave harvester prototype
7. **Hull** — 3D print PETG prototype shell
8. **Integration** — Everything into the shell, waterproofing, tank test
