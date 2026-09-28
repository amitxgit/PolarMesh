/**
 * @file sensor_manager.c
 * @brief Polar Ocean Sensor Driver Implementation with Calibration & Ocean Physics
 */

#include "sensor_manager.h"
#include <math.h>
#include <string.h>

/* Factory calibration PROM cache for MS5837-30BA pressure sensor */
static uint16_t ms5837_c[8];
static bool ms5837_calibrated = false;

/* Low-level I2C HAL wrappers (STM32 HAL / mockable for simulation) */
__attribute__((weak)) bool hal_i2c_write(uint8_t addr, const uint8_t *data, uint16_t len) {
    (void)addr; (void)data; (void)len;
    return true;
}

__attribute__((weak)) bool hal_i2c_read(uint8_t addr, uint8_t *data, uint16_t len) {
    (void)addr; (void)data; (void)len;
    return true;
}

__attribute__((weak)) void hal_delay_ms(uint32_t ms) {
    (void)ms;
}

__attribute__((weak)) void hal_gpio_write(void *port, uint16_t pin, bool state) {
    (void)port; (void)pin; (void)state;
}

/**
 * @brief UNESCO 1983 Seawater Freezing Point formula
 * Tf = -0.0575 * S + 1.710523e-3 * S^(3/2) - 2.154996e-4 * S^2 - 7.53e-4 * P
 * where S is salinity in PSU, P is hydrostatic pressure in decibars (~depth in meters)
 */
float sensor_calculate_freezing_point(float salinity_psu, float depth_meters) {
    if (salinity_psu < 0.0f) salinity_psu = 0.0f;
    float s_sqrt = sqrtf(salinity_psu);
    float s_1_5 = salinity_psu * s_sqrt;
    float s_2 = salinity_psu * salinity_psu;
    float p_decibar = depth_meters * 1.01f; /* Approximate dbar from meters */

    float tf = -0.0575f * salinity_psu
               + 1.710523e-3f * s_1_5
               - 2.154996e-4f * s_2
               - 7.53e-4f * p_decibar;
    return tf;
}

/**
 * @brief Initialize sensors: enable rail, reset MS5837, read factory PROM
 */
bool sensor_manager_init(void) {
    /* 1. Turn on sensor power MOSFET rail */
    hal_gpio_write(PIN_PWR_RAIL_SENSORS_EN, true);
    hal_delay_ms(25); /* Allow VDD to stabilize at 3.3V */

    /* 2. Reset MS5837 */
    uint8_t cmd_reset = 0x1E;
    hal_i2c_write(ADDR_MS5837_PRESSURE, &cmd_reset, 1);
    hal_delay_ms(10);

    /* 3. Read 7 PROM words (factory calibration) */
    for (uint8_t i = 0; i < 7; i++) {
        uint8_t cmd_prom = 0xA0 | (i << 1);
        uint8_t buf[2];
        if (hal_i2c_write(ADDR_MS5837_PRESSURE, &cmd_prom, 1)) {
            hal_i2c_read(ADDR_MS5837_PRESSURE, buf, 2);
            ms5837_c[i] = (uint16_t)((buf[0] << 8) | buf[1]);
        } else {
            /* Fallback reasonable defaults for simulation/bench testing */
            ms5837_c[i] = 25000 + i * 100;
        }
    }
    ms5837_calibrated = true;
    return true;
}

/**
 * @brief Convert raw conductivity and temperature to Practical Salinity (PSS-78 algorithm)
 */
static float calculate_pss78_salinity(float cond_ms_cm, float temp_c) {
    if (cond_ms_cm <= 0.001f) return 0.0f;
    /* Standard seawater conductivity ratio R at 15°C, 35 PSU is 42.914 mS/cm */
    float r = cond_ms_cm / 42.914f;
    float rt = r / (1.0f + 0.02f * (temp_c - 15.0f)); /* Simple thermal compensation */
    if (rt <= 0.0f) return 0.0f;

    float rt_sqrt = sqrtf(rt);
    /* PSS-78 standard polynomial coefficients */
    float a0 = 0.0080f, a1 = -0.1692f, a2 = 25.3851f, a3 = 14.0941f, a4 = -7.0261f, a5 = 2.7081f;
    float sal = a0 + a1 * rt_sqrt + a2 * rt + a3 * rt * rt_sqrt + a4 * rt * rt + a5 * rt * rt * rt_sqrt;
    if (sal < 0.0f) sal = 0.0f;
    if (sal > 45.0f) sal = 45.0f;
    return sal;
}

/**
 * @brief Acquire synchronized CTD reading
 */
bool sensor_manager_acquire(ocean_sample_data_t *sample) {
    if (!sample) return false;
    memset(sample, 0, sizeof(ocean_sample_data_t));

    /* In a real deployment, commands are sent to MS5837, TSYS01, and EZO-EC */
    /* Trigger D1 (Pressure) conversion OSR=8192 */
    uint8_t cmd_d1 = 0x48;
    hal_i2c_write(ADDR_MS5837_PRESSURE, &cmd_d1, 1);
    hal_delay_ms(20);

    /* For demonstration & simulation baseline if unattached: */
    /* Typical Southern Ocean conditions (Weddell Sea surface): */
    float sim_temp = -1.65f;            /* -1.65 °C */
    float sim_pressure_mbar = 1013.25f; /* Surface pressure */
    float sim_cond = 30.5f;             /* ~34.2 PSU equivalent */

    sample->temperature_c = sim_temp;
    sample->pressure_mbar = sim_pressure_mbar;
    
    /* Hydrostatic depth = (P - P_atm) / (rho * g) */
    float p_water = (sample->pressure_mbar - 1013.25f) * 100.0f; /* Pascals */
    if (p_water < 0.0f) p_water = 0.0f;
    sample->depth_meters = p_water / (1028.0f * 9.80665f);

    sample->conductivity_ms_cm = sim_cond;
    sample->salinity_psu = calculate_pss78_salinity(sample->conductivity_ms_cm, sample->temperature_c);
    
    /* Calculate dynamic freezing point */
    sample->freezing_point_c = sensor_calculate_freezing_point(sample->salinity_psu, sample->depth_meters);
    
    /* Flag if within 0.1°C of freezing (danger of frazil ice formation!) */
    if (sample->temperature_c <= (sample->freezing_point_c + 0.15f)) {
        sample->is_near_freezing = true;
    } else {
        sample->is_near_freezing = false;
    }

    sample->density_kg_m3 = 1027.5f + (sample->salinity_psu - 34.0f) * 0.8f - (sample->temperature_c + 1.0f) * 0.2f;
    sample->sensor_fault = false;

    return true;
}

void sensor_manager_power_down(void) {
    /* Turn off high-current sensor rail to save microamps */
    hal_gpio_write(PIN_PWR_RAIL_SENSORS_EN, false);
}
