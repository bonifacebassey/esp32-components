#ifndef __BLE_GATT_SERVER_H__
#define __BLE_GATT_SERVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "esp_gatt_defs.h"
#include "esp_gatts_api.h"
#include "esp_err.h"
#include "bt_controller.h"

    typedef uint16_t connection_t;

    typedef enum
    {
        FASTEST,
        FAST,
        SLOW,
        SLOWER
    } connection_interval_t;

    typedef struct
    {
        esp_gatt_if_t gatts_if;
        uint16_t conn_id;  /*!< Connection id */
        uint32_t trans_id; /*!< Transfer id */
        esp_bd_addr_t bda; /*!< The bluetooth device address which been read */
        uint16_t handle;   /*!< The attribute handle */
        uint16_t offset;   /*!< Offset of the value, if the value is too long */
        bool is_long;      /*!< The value is too long or not */
        bool need_rsp;     /*!< The read operation need to do response */
    } read_evt_param_t;

    esp_err_t ble_start(const char *name, bool with_pin, power_level_t power_level, connection_interval_t c_interval);

    esp_err_t ble_stop();

    esp_err_t ble_start_pairing(const char *name, bool with_pin);

    esp_err_t ble_stop_pairing();

    esp_err_t ble_clear_paired_devices();

    esp_err_t ble_send_data(uint8_t *data, size_t size, bool need_confirm);

    esp_err_t ble_process_read_request(read_evt_param_t *read);

    esp_err_t update_connection_parameters(connection_interval_t conn_interval);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_GATT_SERVER_H__ */