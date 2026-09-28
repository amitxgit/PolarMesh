/**
 * @file lora_mesh.h
 * @brief PolarMesh SX1262 Ultra-Reliable Sub-GHz Mesh Protocol
 */

#ifndef LORA_MESH_H
#define LORA_MESH_H

#include "polarmesh_config.h"
#include "polarmesh_packet.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NEIGHBORS           8
#define ROUTING_CACHE_SIZE      16

typedef struct {
    uint16_t node_id;
    int8_t   last_rssi_dbm;     /* -140 to 0 dBm */
    int8_t   last_snr_db;       /* -20 to +10 dB */
    uint32_t last_heard_epoch;  /* Timestamp */
    uint8_t  hop_distance;      /* Hops to this node */
    bool     is_satellite_gateway; /* Can this node uplink to Iridium? */
} mesh_neighbor_entry_t;

typedef struct {
    uint16_t local_node_id;
    uint8_t  active_neighbor_count;
    mesh_neighbor_entry_t neighbors[MAX_NEIGHBORS];
    uint32_t packets_transmitted;
    uint32_t packets_relayed;
    uint32_t packets_dropped;
} lora_mesh_state_t;

/**
 * @brief Initialize SX1262 SPI, configure RF parameters (868MHz, SF11, BW125, +22dBm)
 */
bool lora_mesh_init(uint16_t node_id);

/**
 * @brief Broadcast periodic surface discovery beacon
 */
bool lora_mesh_send_beacon(const polarmesh_beacon_packet_t *beacon);

/**
 * @brief Route a packet towards the nearest satellite gateway buoy
 */
bool lora_mesh_route_packet(const uint8_t *payload, uint16_t length, uint16_t destination);

/**
 * @brief Process an incoming raw LoRa frame received from SX1262 DIO1 interrupt
 */
void lora_mesh_process_rx_frame(const uint8_t *raw_buf, uint16_t len, int8_t rssi, int8_t snr);

/**
 * @brief Get mesh link status and statistics
 */
const lora_mesh_state_t* lora_mesh_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* LORA_MESH_H */
