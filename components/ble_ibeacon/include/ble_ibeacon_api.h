#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"

#define IBEACON_SENDER 0
#define IBEACON_RECEIVER 1
#define IBEACON_MODE CONFIG_IBEACON_MODE

#define ENDIAN_CHANGE_U16(x) ((((x)&0xFF00) >> 8) + (((x)&0xFF) << 8))

#define ESP_UUID                                                                                       \
    {                                                                                                  \
        0xFD, 0xA5, 0x06, 0x93, 0xA4, 0xE2, 0x4F, 0xB1, 0xAF, 0xCF, 0xC6, 0xEB, 0x07, 0x64, 0x78, 0x25 \
    }
#define ESP_MAJOR 10167
#define ESP_MINOR 61958

typedef struct
{
    uint8_t flags[3];
    uint8_t length;
    uint8_t type;
    uint16_t company_id;
    uint16_t beacon_type;
} __attribute__((packed)) esp_ble_ibeacon_head_t;

typedef struct
{
    uint8_t proximity_uuid[16];
    uint16_t major;
    uint16_t minor;
    int8_t measured_power;
} __attribute__((packed)) esp_ble_ibeacon_vendor_t;

typedef struct
{
    esp_ble_ibeacon_head_t ibeacon_head;
    esp_ble_ibeacon_vendor_t ibeacon_vendor;
} __attribute__((packed)) esp_ble_ibeacon_t;

extern esp_ble_ibeacon_head_t ibeacon_common_head;

bool esp_ble_is_ibeacon_packet(uint8_t *adv_data, uint8_t adv_data_len);

esp_err_t esp_ble_config_ibeacon_data(esp_ble_ibeacon_vendor_t *vendor_config, esp_ble_ibeacon_t *ibeacon_adv_data);
