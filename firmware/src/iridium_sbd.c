/**
 * @file iridium_sbd.c
 * @brief Iridium 9603 AT-Command Driver with Binary Frame Management
 */

#include "iridium_sbd.h"
#include <string.h>
#include <stdio.h>

static bool modem_powered = false;

bool iridium_sbd_power_on(void) {
    /* Assert power enable MOSFET and toggle ON/OFF pin */
    modem_powered = true;
    return true;
}

void iridium_sbd_power_off(void) {
    modem_powered = false;
}

uint8_t iridium_sbd_check_signal(void) {
    if (!modem_powered) return 0;
    /* Simulated satellite pass over Weddell Sea: healthy 4/5 bars */
    return 4;
}

iridium_status_t iridium_sbd_load_binary(const uint8_t *data, uint16_t length) {
    if (!modem_powered) return IRIDIUM_STATUS_POWER_FAULT;
    if (!data || length == 0 || length > IRIDIUM_MAX_MO_SBD_BYTES) {
        return IRIDIUM_STATUS_BUFFER_ERROR;
    }

    /* In hardware:
     * 1. Send "AT+SBDWB=<length>\r"
     * 2. Wait for "READY\r\n"
     * 3. Send raw binary stream + 2-byte MSB checksum
     * 4. Wait for response code '0' (OK)
     */
    return IRIDIUM_STATUS_OK;
}

iridium_status_t iridium_sbd_session(uint8_t *mo_status, uint16_t *mt_length) {
    if (!modem_powered) return IRIDIUM_STATUS_POWER_FAULT;

    /* In hardware: Send "AT+SBDIX\r", parse response "+SBDIX: <MO status>, <MOMSN>, <MT status>, ..." */
    if (mo_status) *mo_status = 0; /* 0 = MO message transferred successfully */
    if (mt_length) *mt_length = 0; /* No incoming config command */

    return IRIDIUM_STATUS_OK;
}
