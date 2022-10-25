#ifndef __BLE_GATTS_TABLE_H__
#define __BLE_GATTS_TABLE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_gatt_defs.h"

    enum
    {
        IDX_SERVICE,

        IDX_CHAR_READ_WRITE,
        IDX_CHAR_VAL_READ_WRITE,
        IDX_CHAR_CFG_READ_WRITE,

        IDX_CHAR_READ,
        IDX_CHAR_VAL_READ,

        IDX_CHAR_WRITE,
        IDX_CHAR_VAL_WRITE,

        IDX_ATTR_IDX_NUM
    };

#define GATTS_CHAR_VAL_LEN_MAX 500
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

    static const uint16_t SERVICE_UUID = 0x00BB;
    static const uint16_t CHAR_UUID_READ = 0xBB01;
    static const uint16_t CHAR_UUID_WRITE = 0xBB02;
    static const uint16_t CHAR_UUID_READ_WRITE = 0xBB03;

    /*
     * The layout of the services, characteristics and descriptors is as follows:
     * - Service: Service UUID
     *    	- Characteristic: Read/Write Characteristic - Declaration
     * 			-Descriptor: Read/Write Characteristic - Value
     * 			-Descriptor: Read/Write Characteristic - Client Characteristic Configuration
     * 		- Characteristic: Read Characteristic - Declaration
     * 			-Descriptor: Read Characteristic - Value
     * 			-Descriptor: Read Characteristic - Client Characteristic Configuration
     *      - Characteristic: Write Characteristic - Declaration
     * 			-Descriptor: Write Characteristic - Value
     * 			-Descriptor: Write Characteristic - Client Characteristic Configuration
     */
    static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
    static const uint16_t char_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
    static const uint16_t client_char_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
    static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
    static const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
    static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
    static const uint8_t client_char_config[2] = {0x00, 0x00};
    static uint8_t char_value[4] = {0x11, 0x22, 0x33, 0x44};

    /*
     * 	- Attribute control
     * 	- Attribute description:
     * 		+ UUID length
     * 		+ UUID value
     * 		+ Attribute permission
     * 		+ Maximum length of the element
     * 		+ Current length of the element
     * 		+ Element value array
     */
    static const esp_gatts_attr_db_t gatts_attr_database[IDX_ATTR_IDX_NUM] = {
        // Service Declaration
        [IDX_SERVICE] = {
            {ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(SERVICE_UUID), (uint8_t *)&SERVICE_UUID}},

        // Characteristic Declaration
        [IDX_CHAR_READ_WRITE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        // Characteristic Value
        [IDX_CHAR_VAL_READ_WRITE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&CHAR_UUID_READ_WRITE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

        // Client Characteristic Configuration Descriptor
        [IDX_CHAR_CFG_READ_WRITE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&client_char_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(client_char_config), (uint8_t *)client_char_config}},

        // Characteristic Declaration
        [IDX_CHAR_READ] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},

        // Characteristic Value
        [IDX_CHAR_VAL_READ] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&CHAR_UUID_READ, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

        // Characteristic Declaration
        [IDX_CHAR_WRITE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&char_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},

        // Characteristic Value
        [IDX_CHAR_VAL_WRITE] = {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&CHAR_UUID_WRITE, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},
    };

#ifdef __cplusplus
}
#endif

#endif // __BLE_GATTS_TABLE_H__