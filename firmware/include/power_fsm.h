/**
 * @file power_fsm.h
 * @brief PolarMesh Ultra-Low-Power Finite State Machine & Cold Battery Management
 */

#ifndef POWER_FSM_H
#define POWER_FSM_H

#include "polarmesh_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PWR_FSM_SLEEP_DEEP,         /* Stop2 mode: <5uA, waiting for RTC/watchdog */
    PWR_FSM_WAKEUP_HEALTH_CHK,  /* ADC checks on Li-SOCl2, LiFePO4, supercaps */
    PWR_FSM_DEPASSIVATION_CYCLE,/* Pulse load to break LiCl passivation layer */
    PWR_FSM_SAMPLE_OCEAN,       /* Power up CTD sensors & read data */
    PWR_FSM_EDGE_ML_EVAL,       /* TinyML inference on sensor profile */
    PWR_FSM_LORA_MESH_EXCHANGE, /* SX1262 LoRa TDMA slot execution */
    PWR_FSM_SUPERCAP_CHARGE,    /* Charge supercaps to 5.0V for Iridium burst */
    PWR_FSM_IRIDIUM_BURST,      /* Iridium SBD transmission */
    PWR_FSM_ICE_HOLD,           /* Ice trapped mode: acoustic / sleep only */
    PWR_FSM_ERROR_RECOVERY      /* Hard reset or coprocessor fallback */
} power_fsm_state_t;

typedef struct {
    power_fsm_state_t current_state;
    power_fsm_state_t previous_state;
    uint32_t state_entered_timestamp;
    uint16_t v_primary_mv;
    uint16_t v_buffer_mv;
    uint16_t v_supercap_mv;
    float    ambient_temp_c;
    bool     is_ice_trapped;
    bool     is_cluster_head;
    uint32_t total_wakeups;
    uint32_t failed_sat_attempts;
} power_system_status_t;

/**
 * @brief Initialize power management FSM and register ADC channels
 */
void power_fsm_init(void);

/**
 * @brief Main FSM tick execution (called on wakeup or state change)
 */
void power_fsm_step(void);

/**
 * @brief Check if supercapacitor has sufficient charge for 2.5A Iridium pulse
 */
bool power_is_supercap_ready_for_iridium(void);

/**
 * @brief Execute controlled resistive load pulse to remove LiCl passivation
 * from Li-SOCl2 primary battery after long cold storage
 */
void power_execute_depassivation(void);

/**
 * @brief Transition directly to ice trapped survival mode
 */
void power_enter_ice_trapped_mode(void);

/**
 * @brief Query current power telemetry status
 */
const power_system_status_t* power_fsm_get_status(void);

#ifdef __cplusplus
}
#endif

#endif /* POWER_FSM_H */
