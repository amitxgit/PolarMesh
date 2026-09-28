/**
 * @file polarmesh_config.h
 * @brief PolarMesh System Configuration & Hardware Pinout Map
 * @target STM32L4R5ZI (Arm Cortex-M4 @ 120MHz) + MSP430FR5994 Coprocessor
 * @hackathon Smart India Hackathon 2026 - Problem SIH26065
 * @sponsor Ministry of Earth Sciences (MoES) / INCOIS
 */

#ifndef POLARMESH_CONFIG_H
#define POLARMESH_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* System Identification & Firmware Version                                  */
/* ========================================================================= */
#define POLARMESH_FW_VERSION_MAJOR      1
#define POLARMESH_FW_VERSION_MINOR      0
#define POLARMESH_FW_VERSION_PATCH      0
#define POLARMESH_HARDWARE_REV          "REV_B_UHMWPE"

/* Unique Node ID (Can be overridden by unique device ID in flash/OTP) */
#define POLARMESH_DEFAULT_NODE_ID       0xAA01  /* Weddell Sea Buoy Alpha-1 */
#define POLARMESH_MESH_NETWORK_ID       0x504D  /* 'PM' in ASCII */

/* ========================================================================= */
/* Power Management & Operating Thresholds                                  */
/* ========================================================================= */
/* Primary battery chemistry: Li-SOCl2 (3.6V nominal, cut-off 2.8V)         */
/* Secondary buffer: LiFePO4 (3.2V nominal, charge disabled below 0°C)       */
#define V_BATT_PRIMARY_CRITICAL_MV      2850    /* mV: Force hibernate       */
#define V_BATT_PRIMARY_LOW_MV           3100    /* mV: Reduce sampling rate  */
#define V_BATT_PRIMARY_NOMINAL_MV       3600    /* mV: Healthy               */

#define V_SUPERCAP_MIN_IRIDIUM_TX_MV    4800    /* 5.0V rail needed for 2.5A pulse */
#define V_SUPERCAP_MAX_TARGET_MV        5200    /* Supercap voltage ceiling  */

#define TEMP_LIFEPO4_CHARGE_MIN_C       0.0f    /* Celsius: strictly no charging below 0C */
#define TEMP_SURFACE_FREEZE_TRIGGER_C   -1.85f  /* Seawater freezing point at 35 PSU */

/* Sampling and Duty Cycle Timers (Seconds) */
#define TIME_SLEEP_NORMAL_SEC           3600    /* 1 Hour between surface casts */
#define TIME_SLEEP_UNDER_ICE_SEC        14400   /* 4 Hours under ice to conserve power */
#define TIME_SLEEP_CRITICAL_SEC         86400   /* 24 Hours emergency beacon only */
#define TIME_LORA_BEACON_INTERVAL_SEC   300     /* 5 Minutes mesh discovery interval */
#define TIME_IRIDIUM_UPLINK_INTERVAL_SEC 21600  /* 6 Hours satellite upload batch */

/* ========================================================================= */
/* Sensor Pin Mapping (STM32L4R5ZI LQFP144)                                  */
/* ========================================================================= */
/* I2C1: Environmental & Pressure Sensors (100kHz Standard / 400kHz Fast)   */
#define PIN_I2C1_SCL                    GPIOB, GPIO_PIN_8
#define PIN_I2C1_SDA                    GPIOB, GPIO_PIN_9
#define ADDR_MS5837_PRESSURE            0x76    /* 30-bar pressure sensor */
#define ADDR_TSYS01_TEMPERATURE         0x77    /* High precision sea temp */
#define ADDR_EZO_EC_CONDUCTIVITY        0x64    /* Atlas Scientific EC Circuit */

/* SPI1: Semtech SX1262 LoRa Sub-GHz Transceiver                             */
#define PIN_SPI1_SCK                    GPIOA, GPIO_PIN_5
#define PIN_SPI1_MISO                   GPIOA, GPIO_PIN_6
#define PIN_SPI1_MOSI                   GPIOA, GPIO_PIN_7
#define PIN_SX1262_NSS                  GPIOD, GPIO_PIN_14
#define PIN_SX1262_RESET                GPIOD, GPIO_PIN_15
#define PIN_SX1262_BUSY                 GPIOC, GPIO_PIN_6
#define PIN_SX1262_DIO1                 GPIOC, GPIO_PIN_7

/* USART1: Iridium 9603 SBD Transceiver (19200 baud, 8N1, RTS/CTS flow)     */
#define PIN_USART1_TX                   GPIOA, GPIO_PIN_9
#define PIN_USART1_RX                   GPIOA, GPIO_PIN_10
#define PIN_USART1_CTS                  GPIOA, GPIO_PIN_11
#define PIN_USART1_RTS                  GPIOA, GPIO_PIN_12
#define PIN_IRIDIUM_ON_OFF              GPIOD, GPIO_PIN_8
#define PIN_IRIDIUM_NETWORK_AVAIL       GPIOD, GPIO_PIN_9

/* USART2: GNSS Module (u-blox MAX-M10Q GPS/Galileo/BeiDou @ 9600 baud)      */
#define PIN_USART2_TX                   GPIOA, GPIO_PIN_2
#define PIN_USART2_RX                   GPIOA, GPIO_PIN_3
#define PIN_GNSS_EXTINT                 GPIOD, GPIO_PIN_10
#define PIN_GNSS_POWER_EN               GPIOD, GPIO_PIN_11

/* SPI2 / UART3: MSP430FR5994 FRAM Co-processor Interconnect                 */
#define PIN_COPROC_HEARTBEAT_IN         GPIOE, GPIO_PIN_2
#define PIN_COPROC_HEARTBEAT_OUT        GPIOE, GPIO_PIN_3
#define PIN_COPROC_IRQ                  GPIOE, GPIO_PIN_4

/* Power Rail Switches (Low-loss P-channel MOSFET gates)                     */
#define PIN_PWR_RAIL_SENSORS_EN         GPIOF, GPIO_PIN_0
#define PIN_PWR_RAIL_SX1262_EN          GPIOF, GPIO_PIN_1
#define PIN_PWR_RAIL_IRIDIUM_EN         GPIOF, GPIO_PIN_2
#define PIN_PWR_RAIL_GNSS_EN            GPIOF, GPIO_PIN_3
#define PIN_SUPERCAP_CHARGER_EN         GPIOF, GPIO_PIN_4

/* ADC Channels: Analog Voltage Monitors & Temperature                       */
#define ADC_CH_BATT_PRIMARY_DIV         ADC_CHANNEL_1   /* Resistor divider 1:2 */
#define ADC_CH_BATT_BUFFER_DIV          ADC_CHANNEL_2   /* Resistor divider 1:2 */
#define ADC_CH_SUPERCAP_DIV             ADC_CHANNEL_3   /* Resistor divider 1:2 */
#define ADC_CH_SOLAR_HARVEST_DIV        ADC_CHANNEL_4   /* Photovoltaic monitor */
#define ADC_CH_WAVE_HARVEST_DIV         ADC_CHANNEL_5   /* Pendulum piezo/dynamo*/

/* ========================================================================= */
/* LoRa Mesh Radio Parameters (SX1262)                                       */
/* ========================================================================= */
#define LORA_FREQUENCY_HZ               868000000       /* 868 MHz ISM / MoES lic */
#define LORA_SPREADING_FACTOR           11              /* SF11 for extreme range */
#define LORA_BANDWIDTH_KHZ              125             /* 125 kHz */
#define LORA_CODING_RATE                4               /* 4/8 for high error corr */
#define LORA_TX_POWER_DBM               22              /* Max legal output (+22dBm) */
#define LORA_PREAMBLE_LENGTH            12
#define LORA_MAX_PAYLOAD_LEN            220
#define LORA_MAX_HOPS                   4               /* Max mesh relay depth */

/* ========================================================================= */
/* Iridium SBD Constraints                                                   */
/* ========================================================================= */
#define IRIDIUM_MAX_MO_SBD_BYTES        340             /* Mobile Originated limit */
#define IRIDIUM_MAX_MT_SBD_BYTES        270             /* Mobile Terminated limit */
#define IRIDIUM_REGISTRATION_TIMEOUT_S  90
#define IRIDIUM_TX_BURST_MAX_RETRIES    3

/* ========================================================================= */
/* Data Packet Definitions & Buffer Capacities                              */
/* ========================================================================= */
#define MAX_SENSORS_PER_PROFILE         16              /* Discrete depth layers */
#define FLASH_SECTOR_DATA_LOG           0x08080000      /* STM32 onboard Flash */
#define COPROC_FRAM_RING_BUFFER_SIZE    4096            /* 4KB ultra-safe journal */

#ifdef __cplusplus
}
#endif

#endif /* POLARMESH_CONFIG_H */
