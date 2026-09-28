/**
 * @file polarmesh_packet.h
 * @brief PolarMesh Compact Binary Telemetry Packet Definitions
 * @note Optimized for 340-byte Iridium SBD frames & 220-byte SX1262 LoRa frames.
 */

#ifndef POLARMESH_PACKET_H
#define POLARMESH_PACKET_H

#include <stdint.h>
#include <stdbool.h>

#pragma pack(push, 1)

/**
 * @brief Packet type identifiers
 */
typedef enum {
    PKT_TYPE_BEACON           = 0x01,  /* Periodic surface discovery beacon */
    PKT_TYPE_CTD_PROFILE      = 0x02,  /* Full CTD water column cast */
    PKT_TYPE_ALERT_ICE        = 0x03,  /* Urgent: Surface ice entrapment detected */
    PKT_TYPE_ALERT_ANOMALY    = 0x04,  /* Edge ML flagged sensor anomaly */
    PKT_TYPE_MESH_ROUTED      = 0x05,  /* Multi-hop encapsulated message */
    PKT_TYPE_COMMAND_ACK      = 0x06,  /* Uplink command confirmation */
    PKT_TYPE_DIAGNOSTIC       = 0x07   /* Hardware & power subsystem report */
} polarmesh_packet_type_t;

/**
 * @brief System Operating Mode & State
 */
typedef enum {
    SYS_STATE_COLD_BOOT       = 0,
    SYS_STATE_SURFACE_DRIFT   = 1,
    SYS_STATE_PROFILING_CAST  = 2,
    SYS_STATE_ICE_TRAPPED     = 3,
    SYS_STATE_LOW_POWER_REST  = 4,
    SYS_STATE_EMERGENCY_LOC   = 5
} polarmesh_sys_state_t;

/**
 * @brief Fixed-size Header for all PolarMesh transmissions (12 bytes)
 */
typedef struct {
    uint16_t network_id;        /* 0x504D ('PM') */
    uint16_t origin_node_id;    /* Originator Buoy ID (e.g. 0xAA01) */
    uint16_t target_node_id;    /* Target (0xFFFF for broadcast / gateway) */
    uint32_t epoch_timestamp;   /* UTC seconds from GPS */
    uint8_t  packet_type;       /* polarmesh_packet_type_t */
    uint8_t  hop_count;         /* Incremented on each LoRa relay hop */
} polarmesh_header_t;

/**
 * @brief Single CTD observation point (8 bytes compressed)
 * - Depth: 0..3000m (resolution: 0.1m, fits in 16-bit uint)
 * - Temp: -4.000°C .. +35.000°C (offset +4.000, 0.001°C step, fits in 16-bit uint)
 * - Salinity: 0.00 .. 45.00 PSU (0.01 PSU step, fits in 16-bit uint)
 * - Dissolved Oxygen / Turbidity: 0..65535 raw ADC / counts (16-bit uint)
 */
typedef struct {
    uint16_t depth_decimeters;      /* Depth in 0.1 m (0 to 6553.5m) */
    int16_t  temp_millicelsius;     /* Temperature in 0.001 °C (-4000 to +35000) */
    uint16_t salinity_centi_psu;    /* Salinity in 0.01 PSU (e.g., 3485 = 34.85 PSU) */
    uint16_t turbidity_raw;         /* Optical backscatter / fluorometer counts */
} polarmesh_ctd_sample_t;

/**
 * @brief Full CTD Profile Packet (Fits comfortably in 340-byte Iridium SBD limit)
 * Header (12B) + Metadata (16B) + 16 Depth Layers (128B) + Energy (8B) + CRC (2B) = 166 Bytes
 */
typedef struct {
    polarmesh_header_t header;

    /* GPS Coordinates (Scaled integers for compactness) */
    int32_t  latitude_microdeg;     /* Lat * 10^6 (-90.000000 to +90.000000) */
    int32_t  longitude_microdeg;    /* Lon * 10^6 (-180.000000 to +180.000000) */
    uint16_t profile_index;         /* Sequential cast number */
    uint8_t  total_samples;         /* Number of active depth points in payload */
    uint8_t  system_state;          /* Current polarmesh_sys_state_t */

    /* Array of discrete depth layer measurements */
    polarmesh_ctd_sample_t samples[MAX_SENSORS_PER_PROFILE];

    /* Power Subsystem Telemetry */
    uint16_t v_primary_mv;          /* Li-SOCl2 primary battery voltage */
    uint16_t v_supercap_mv;         /* 5V supercap rail voltage */
    int16_t  internal_temp_c;       /* Electronics bay temperature (0.1 °C) */
    uint16_t harvested_energy_mw;   /* Wave + solar instantaneous input */

    /* Integrity check */
    uint16_t crc16;                 /* CCITT CRC-16 */
} polarmesh_ctd_packet_t;

/**
 * @brief Rapid Surface Beacon & Mesh Alert Packet (36 bytes)
 * Ideal for high-speed SX1262 LoRa mesh broadcasts
 */
typedef struct {
    polarmesh_header_t header;
    int32_t  latitude_microdeg;
    int32_t  longitude_microdeg;
    int16_t  surface_temp_mdeg;     /* Surface sea water temperature */
    uint16_t surface_salinity_cpsu; /* Surface salinity */
    uint16_t wave_height_cm;        /* Peak-to-peak wave displacement */
    uint16_t wave_period_ms;        /* Dominant wave period */
    uint16_t v_primary_mv;
    uint8_t  battery_soc_pct;       /* Estimated state of charge (0-100%) */
    uint8_t  alert_flags;           /* Bit 0: Ice trapped, Bit 1: Low cap, Bit 2: Tilt lock */
    uint16_t crc16;
} polarmesh_beacon_packet_t;

/**
 * @brief Alert Flags Bitmask
 */
#define ALERT_FLAG_ICE_LOCK         (1 << 0)
#define ALERT_FLAG_TILT_CAPSIZED    (1 << 1)
#define ALERT_FLAG_LEAK_DETECTED    (1 << 2)
#define ALERT_FLAG_TEMP_CRITICAL    (1 << 3)
#define ALERT_FLAG_SENSOR_DEGRADE   (1 << 4)
#define ALERT_FLAG_SATELLITE_FAIL   (1 << 5)

#pragma pack(pop)

#endif /* POLARMESH_PACKET_H */
