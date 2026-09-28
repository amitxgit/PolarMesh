/**
 * @file power_fsm.c
 * @brief Power Management State Machine & Battery Chemistry Protection
 */

#include "power_fsm.h"
#include "sensor_manager.h"
#include <string.h>

static power_system_status_t sys_power_status;

/* Simulated/HAL ADC readings */
__attribute__((weak)) uint16_t hal_adc_read_channel(uint32_t channel) {
    switch (channel) {
        case ADC_CH_BATT_PRIMARY_DIV:  return 3600; /* 3.6V Li-SOCl2 */
        case ADC_CH_BATT_BUFFER_DIV:   return 3250; /* 3.25V LiFePO4 */
        case ADC_CH_SUPERCAP_DIV:      return 5050; /* 5.05V Supercap */
        case ADC_CH_SOLAR_HARVEST_DIV: return 120;  /* Weak polar sun / twilight */
        case ADC_CH_WAVE_HARVEST_DIV:  return 450;  /* Wave kinetic harvest 450mW */
        default: return 0;
    }
}

__attribute__((weak)) void hal_pulse_depassivation_mosfet(uint32_t ms) {
    (void)ms;
    /* Pulses a 10-ohm resistor across Li-SOCl2 for 50-100ms */
}

__attribute__((weak)) void hal_enter_stop2_mode(uint32_t seconds) {
    (void)seconds;
    /* STM32 Stop2 Mode with RTC Alarm wake-up */
}

void power_fsm_init(void) {
    memset(&sys_power_status, 0, sizeof(power_system_status_t));
    sys_power_status.current_state = PWR_FSM_WAKEUP_HEALTH_CHK;
    sys_power_status.previous_state = PWR_FSM_SLEEP_DEEP;
    sys_power_status.v_primary_mv = hal_adc_read_channel(ADC_CH_BATT_PRIMARY_DIV);
    sys_power_status.v_buffer_mv = hal_adc_read_channel(ADC_CH_BATT_BUFFER_DIV);
    sys_power_status.v_supercap_mv = hal_adc_read_channel(ADC_CH_SUPERCAP_DIV);
    sys_power_status.is_cluster_head = true; /* Default or elected */
}

bool power_is_supercap_ready_for_iridium(void) {
    sys_power_status.v_supercap_mv = hal_adc_read_channel(ADC_CH_SUPERCAP_DIV);
    return (sys_power_status.v_supercap_mv >= V_SUPERCAP_MIN_IRIDIUM_TX_MV);
}

void power_execute_depassivation(void) {
    /* Li-SOCl2 cells develop a lithium chloride (LiCl) crystal layer during cold storage.
     * When a load is suddenly applied, cell voltage can momentarily dip below brownout.
     * We apply a 50ms controlled 100mA discharge pulse to restore full cathode conductivity.
     */
    hal_pulse_depassivation_mosfet(60);
    sys_power_status.v_primary_mv = hal_adc_read_channel(ADC_CH_BATT_PRIMARY_DIV);
}

void power_enter_ice_trapped_mode(void) {
    sys_power_status.is_ice_trapped = true;
    sys_power_status.current_state = PWR_FSM_ICE_HOLD;
}

const power_system_status_t* power_fsm_get_status(void) {
    return &sys_power_status;
}

void power_fsm_step(void) {
    switch (sys_power_status.current_state) {
        case PWR_FSM_WAKEUP_HEALTH_CHK:
            sys_power_status.total_wakeups++;
            sys_power_status.v_primary_mv = hal_adc_read_channel(ADC_CH_BATT_PRIMARY_DIV);
            
            /* Check if battery is passivated */
            if (sys_power_status.v_primary_mv < 3200 && sys_power_status.v_primary_mv > V_BATT_PRIMARY_CRITICAL_MV) {
                sys_power_status.current_state = PWR_FSM_DEPASSIVATION_CYCLE;
            } else if (sys_power_status.v_primary_mv <= V_BATT_PRIMARY_CRITICAL_MV) {
                /* Extreme emergency power save */
                sys_power_status.current_state = PWR_FSM_SLEEP_DEEP;
                hal_enter_stop2_mode(TIME_SLEEP_CRITICAL_SEC);
                return;
            } else {
                sys_power_status.current_state = PWR_FSM_SAMPLE_OCEAN;
            }
            break;

        case PWR_FSM_DEPASSIVATION_CYCLE:
            power_execute_depassivation();
            sys_power_status.current_state = PWR_FSM_SAMPLE_OCEAN;
            break;

        case PWR_FSM_SAMPLE_OCEAN: {
            ocean_sample_data_t sample;
            sensor_manager_init();
            sensor_manager_acquire(&sample);
            sensor_manager_power_down();

            /* Check for ice entrapment */
            if (sample.is_near_freezing && sample.pressure_mbar > 1050.0f) {
                /* Buoy is both freezing cold and being submerged under atmospheric ice pressure */
                power_enter_ice_trapped_mode();
                return;
            }

            sys_power_status.current_state = PWR_FSM_EDGE_ML_EVAL;
            break;
        }

        case PWR_FSM_EDGE_ML_EVAL:
            /* Run TinyML inference */
            sys_power_status.current_state = PWR_FSM_LORA_MESH_EXCHANGE;
            break;

        case PWR_FSM_LORA_MESH_EXCHANGE:
            /* Execute SX1262 peer broadcast */
            if (sys_power_status.is_cluster_head) {
                sys_power_status.current_state = PWR_FSM_SUPERCAP_CHARGE;
            } else {
                sys_power_status.current_state = PWR_FSM_SLEEP_DEEP;
                hal_enter_stop2_mode(TIME_SLEEP_NORMAL_SEC);
            }
            break;

        case PWR_FSM_SUPERCAP_CHARGE:
            if (power_is_supercap_ready_for_iridium()) {
                sys_power_status.current_state = PWR_FSM_IRIDIUM_BURST;
            } else {
                /* Wait or abort satellite uplink to protect primary cell */
                sys_power_status.current_state = PWR_FSM_SLEEP_DEEP;
                hal_enter_stop2_mode(600); /* Try again in 10 mins */
            }
            break;

        case PWR_FSM_IRIDIUM_BURST:
            /* Transmit batch to Iridium constellation */
            sys_power_status.current_state = PWR_FSM_SLEEP_DEEP;
            hal_enter_stop2_mode(TIME_SLEEP_NORMAL_SEC);
            break;

        case PWR_FSM_ICE_HOLD:
            /* Long hibernation while encased in pack ice */
            hal_enter_stop2_mode(TIME_SLEEP_UNDER_ICE_SEC);
            sys_power_status.current_state = PWR_FSM_WAKEUP_HEALTH_CHK;
            break;

        case PWR_FSM_SLEEP_DEEP:
        default:
            hal_enter_stop2_mode(TIME_SLEEP_NORMAL_SEC);
            sys_power_status.current_state = PWR_FSM_WAKEUP_HEALTH_CHK;
            break;
    }
}
