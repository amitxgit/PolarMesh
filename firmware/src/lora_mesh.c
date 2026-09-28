/**
 * @file lora_mesh.c
 * @brief PolarMesh SX1262 LoRa Mesh Routing Implementation
 */

#include "lora_mesh.h"
#include <string.h>

static lora_mesh_state_t mesh_state;

bool lora_mesh_init(uint16_t node_id) {
    memset(&mesh_state, 0, sizeof(lora_mesh_state_t));
    mesh_state.local_node_id = node_id;
    return true;
}

const lora_mesh_state_t* lora_mesh_get_state(void) {
    return &mesh_state;
}

bool lora_mesh_send_beacon(const polarmesh_beacon_packet_t *beacon) {
    if (!beacon) return false;
    mesh_state.packets_transmitted++;
    return true;
}

bool lora_mesh_route_packet(const uint8_t *payload, uint16_t length, uint16_t destination) {
    (void)destination;
    if (!payload || length == 0 || length > LORA_MAX_PAYLOAD_LEN) {
        mesh_state.packets_dropped++;
        return false;
    }

    /* Check if packet hop count exceeds limit */
    polarmesh_header_t *hdr = (polarmesh_header_t *)payload;
    if (hdr->hop_count >= LORA_MAX_HOPS) {
        mesh_state.packets_dropped++;
        return false;
    }

    hdr->hop_count++;
    mesh_state.packets_relayed++;
    return true;
}

void lora_mesh_process_rx_frame(const uint8_t *raw_buf, uint16_t len, int8_t rssi, int8_t snr) {
    if (!raw_buf || len < sizeof(polarmesh_header_t)) {
        mesh_state.packets_dropped++;
        return;
    }

    const polarmesh_header_t *hdr = (const polarmesh_header_t *)raw_buf;
    if (hdr->network_id != POLARMESH_MESH_NETWORK_ID) {
        /* Discard foreign radio packets */
        return;
    }

    /* Update neighbor entry */
    bool found = false;
    for (uint8_t i = 0; i < mesh_state.active_neighbor_count; i++) {
        if (mesh_state.neighbors[i].node_id == hdr->origin_node_id) {
            mesh_state.neighbors[i].last_rssi_dbm = rssi;
            mesh_state.neighbors[i].last_snr_db = snr;
            mesh_state.neighbors[i].last_heard_epoch = hdr->epoch_timestamp;
            mesh_state.neighbors[i].hop_distance = hdr->hop_count;
            found = true;
            break;
        }
    }

    if (!found && mesh_state.active_neighbor_count < MAX_NEIGHBORS) {
        mesh_neighbor_entry_t *n = &mesh_state.neighbors[mesh_state.active_neighbor_count++];
        n->node_id = hdr->origin_node_id;
        n->last_rssi_dbm = rssi;
        n->last_snr_db = snr;
        n->last_heard_epoch = hdr->epoch_timestamp;
        n->hop_distance = hdr->hop_count;
        n->is_satellite_gateway = false;
    }

    /* If destination is broadcast or for us, consume it; else forward */
    if (hdr->target_node_id != mesh_state.local_node_id && hdr->target_node_id != 0xFFFF) {
        lora_mesh_route_packet(raw_buf, len, hdr->target_node_id);
    }
}
