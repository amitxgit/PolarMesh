/**
 * @file sensor_manager.h
 * @brief High-Reliability Polar Ocean Sensor Interface & Calibration Math
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "polarmesh_config.h"
#include "polarmesh_packet.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature_c;        /* -2.5 to +35.0 °C (TSYS01 resolution 0.001 °C) */
    float pressure_mbar;        /* MS5837 pressure in mbar */
    float depth_meters;         /* Depth calculated from seawater density */
    float conductivity_ms_cm;   /* Atlas Scientific EZO-EC in mS/cm */
    float salinity_psu;         /* Practical Salinity Scale (PSS-78) */
    float density_kg_m3;        /* Seawater density (UNESCO formula) */
    float freezing_point_c;     /* Dynamic freezing threshold at current S & P */
    bool  is_near_freezing;     /* True if T within 0.1°C of freezing */
    bool  sensor_fault;         /* Hardware or communication error flag */
} ocean_sample_data_t;

/**
 * @brief Initialize all oceanographic sensors and verify I2C presence
 * @return true if all primary sensors respond with valid calibration constants
 */
bool sensor_manager_init(void);

/**
 * @brief Power on sensor rail, let analog circuits settle, and acquire reading
 * @param[out] sample Populated sample struct
 * @return true on success
 */
bool sensor_manager_acquire(ocean_sample_data_t *sample);

/**
 * @brief Calculate UNESCO 1983 dynamic seawater freezing point
 * @param salinity_psu Practical Salinity (typically 33.0 to 35.5 in Southern Ocean)
 * @param depth_meters Hydrostatic pressure component
 * @return Seawater freezing point in Celsius
 */
float sensor_calculate_freezing_point(float salinity_psu, float depth_meters);

/**
 * @brief Put sensors into ultra-low-power standby and de-assert sensor MOSFET power rail
 */
void sensor_manager_power_down(void);

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_MANAGER_H */
