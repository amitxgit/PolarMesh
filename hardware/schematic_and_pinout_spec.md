# PolarMesh (SIH26065) - Electrical Schematic & Hardware Specification

**Target Platform:** Dual-MCU Fault-Tolerant Polar Ocean Buoy  
**Primary MCU:** STM32L4R5ZI (120MHz Arm Cortex-M4F, 2MB Flash, 640KB RAM, Stop2 5µA)  
**Safety Watchdog & RTC:** TI MSP430FR5994 (16MHz, 256KB Non-Volatile FRAM, 450nA LPM3.5)  
**PCB Stackup:** 4-Layer High-TG FR-4 (170°C), immersion gold (ENIG), silicone conformal coated

---

## 1. Dual-MCU Heartbeat & Fail-Safe Architecture

```
                   +----------------------------------+
                   |     MSP430FR5994 Co-processor    |
                   |   (450 nA LPM3.5 RTC Monitor)   |
                   +----------------+-----------------+
                                    |
                    Heartbeat Ping  |  Hard Reset Trigger
                    (GPIO Interrupt)|  (NRST Line)
                                    v
                   +----------------+-----------------+
                   |       STM32L4R5ZI Main MCU       |
                   |  (Sensor Fusion, ML, RF Engine)  |
                   +----------------+-----------------+
                                    |
       +-------------+--------------+--------------+-------------+
       |             |              |              |             |
       v             v              v              v             v
 [I2C1 Bus]    [SPI1 Bus]     [USART1 Bus]   [USART2 Bus]   [ADC Rails]
  MS5837-30BA    SX1262 LoRa    Iridium 9603   MAX-M10Q GNSS  V_batt_primary
  TSYS01 Temp    (+22dBm Sub-G) (Global SBD)   (Ultra-low)    V_buffer
  EZO-EC Salinity                                             V_supercap
```

### Heartbeat Operation
- The STM32 pulses `PIN_COPROC_HEARTBEAT_OUT` (PE3) on every scheduled task cycle.
- If the STM32 freezes due to a firmware fault, hard fault, or flash corruption at -40°C:
  - The MSP430 timer expires (configurable 15 to 60 minutes).
  - The MSP430 asserts the STM32 `NRST` pin low for 50ms, forcing a clean hardware reboot.
  - The MSP430 increments a persistent reset counter stored in its non-volatile ferroelectric RAM (FRAM), which survives complete power collapse without battery backup.

---

## 2. Power Rail Switching & Supercapacitor Surge Protection

```
 +-------------------------+
 | Saft LS33600 (3.6V)     |
 | Primary Li-SOCl2        |----+
 +-------------------------+    |
                                v
               [Reverse Diode / Ideal Diode Controller]
                                |
 +-------------------------+    |    +-----------------------------+
 | Kinetic / Solar Harvester|---+--->| TI BQ25570 Boost Regulator  |
 | (Pendulum + PV Cells)   |         | Ultra-Low Power PMIC        |
 +-------------------------+         +--------------+--------------+
                                                    |
                                    +---------------+---------------+
                                    |                               |
                                    v                               v
                       [3.3V Low-Noise LDO Rail]        [5.0V Supercap Rail]
                       - STM32L4R5ZI & MSP430           - 2x 50F Series (25F 5.5V)
                       - Sensors via P-MOSFET Switch    - Max pulse: 2.5A
                       - SX1262 LoRa via P-MOSFET       - Dedicated to Iridium 9603
```

### Passivation Breaker Circuit
Li-SOCl2 primary batteries form an insulating passivation layer of lithium chloride crystals on the anode during prolonged cold shelf life.  
- **Circuit:** A logic-level P-channel MOSFET (AO3401A) switches a 36-ohm, 1W pulse load resistor across the battery terminals for 60 ms.
- **Trigger:** Controlled by STM32 pin `PF4` upon cold boot or whenever loaded open-circuit voltage tests below 3.2V.

---

## 3. Sensor Probe Waterproofing & Penetration Details

| Sensor | Subsea Penetration Method | Hydrostatic Rating |
|---|---|---|
| **MS5837-30BA** | Blue Robotics M10 Threaded Bulkhead with double O-Ring | 300 meters (30 bar) |
| **TSYS01** | Marine grade SS316 4mm thermowell filled with thermally conductive epoxy | 500 meters |
| **Atlas Scientific EC** | K 10.0 platinum probe inserted through watertight Cable Gland PG-9 | 100 meters |
| **Hull Material** | CNC Machined Ultra-High-Molecular-Weight Polyethylene (UHMWPE) | 12mm thickness, impact rated to -260°C |
