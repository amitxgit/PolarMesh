/**
 * @file main.c
 * @brief PolarMesh Main Embedded System Entry Point
 * @target STM32L4R5ZI (120MHz Cortex-M4)
 * @project Smart India Hackathon 2026 - Problem SIH26065
 */

#include "polarmesh_config.h"
#include "polarmesh_packet.h"
#include "sensor_manager.h"
#include "power_fsm.h"
#include "lora_mesh.h"
#include "iridium_sbd.h"
#include "edge_anomaly.h"
#include <stdio.h>
#include <string.h>

/* System tick and debug banner */
static void polarmesh_system_init(void) {
    /* Initialize clocks, power peripherals, and communication buses */
    power_fsm_init();
    lora_mesh_init(POLARMESH_DEFAULT_NODE_ID);
    edge_anomaly_init();
}

/**
 * @brief Main execution loop
 */
int main(void) {
    polarmesh_system_init();

    while (1) {
        /* Run one step of the ultra-low-power state machine */
        power_fsm_step();
    }

    return 0;
}
