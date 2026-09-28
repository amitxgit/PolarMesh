/**
 * @file iridium_sbd.h
 * @brief PolarMesh Iridium 9603 Short Burst Data (SBD) Driver
 */

#ifndef IRIDIUM_SBD_H
#define IRIDIUM_SBD_H

#include "polarmesh_config.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    IRIDIUM_STATUS_OK = 0,
    IRIDIUM_STATUS_NO_NETWORK,
    IRIDIUM_STATUS_BUFFER_ERROR,
    IRIDIUM_STATUS_TIMEOUT,
    IRIDIUM_STATUS_CHECKSUM_FAIL,
    IRIDIUM_STATUS_POWER_FAULT
} iridium_status_t;

/**
 * @brief Initialize USART1 for 9603 SBD transceiver, assert power gate
 */
bool iridium_sbd_power_on(void);

/**
 * @brief Power off modem to conserve quiescent current
 */
void iridium_sbd_power_off(void);

/**
 * @brief Check Iridium constellation signal strength (AT+CSQ)
 * @return 0 to 5 bars (5 = best, requires >= 2 for reliable link)
 */
uint8_t iridium_sbd_check_signal(void);

/**
 * @brief Write binary payload into modem Mobile Originated (MO) buffer (AT+SBDWB)
 * @param data Byte buffer (max 340 bytes)
 * @param length Byte length
 * @return IRIDIUM_STATUS_OK on success
 */
iridium_status_t iridium_sbd_load_binary(const uint8_t *data, uint16_t length);

/**
 * @brief Execute satellite session exchange (AT+SBDIX)
 * Transmits MO buffer and retrieves any incoming MT buffer
 * @param[out] mo_status 0-2 indicates success
 * @param[out] mt_length Bytes in received MT buffer
 * @return IRIDIUM_STATUS_OK on success
 */
iridium_status_t iridium_sbd_session(uint8_t *mo_status, uint16_t *mt_length);

#ifdef __cplusplus
}
#endif

#endif /* IRIDIUM_SBD_H */
