/**
 * @file edge_anomaly.h
 * @brief PolarMesh Edge TinyML Sensor Anomaly & Ice Accretion Detector
 */

#ifndef EDGE_ANOMALY_H
#define EDGE_ANOMALY_H

#include "polarmesh_packet.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ANOMALY_TYPE_NONE               = 0,
    ANOMALY_TYPE_ICE_ACCRETION      = 1, /* Temperature at freezing + high tilt damping */
    ANOMALY_TYPE_DENSITY_INVERSION  = 2, /* Physical stability violation: density decrease with depth */
    ANOMALY_TYPE_BIOFOULING_DRIFT   = 3, /* Gradual unphysical conductivity offset */
    ANOMALY_TYPE_PRESSURE_SPIKE     = 4  /* Crushing pressure from pack ice ridging */
} anomaly_classification_t;

typedef struct {
    anomaly_classification_t classification;
    float   reconstruction_error;       /* Autoencoder mean squared error */
    float   confidence;                 /* 0.0 to 1.0 */
    bool    requires_urgent_uplink;     /* If true, overrides 6-hour sleep and sends emergency SBD */
} anomaly_result_t;

/**
 * @brief Initialize TinyML model weights and normalization factors
 */
bool edge_anomaly_init(void);

/**
 * @brief Run inference on a 16-point depth profile
 */
anomaly_result_t edge_anomaly_evaluate_profile(const polarmesh_ctd_packet_t *profile);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_ANOMALY_H */
