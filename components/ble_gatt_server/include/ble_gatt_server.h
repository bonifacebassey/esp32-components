#ifndef __BLE_GATT_SERVER_H__
#define __BLE_GATT_SERVER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>

#include "esp_gatt_defs.h"
#include "esp_err.h"
#include "bt_controller.h"

    typedef uint16_t connection_t;

    typedef enum
    {
        FAST,
        FASTER,
        SLOW,
        SLOWER
    } connection_interval_t;

    esp_err_t ble_start(const char *name, bool with_pin, power_level_t power_level, connection_interval_t c_interval);

    esp_err_t ble_stop();

    esp_err_t ble_start_pairing(const char *name, bool with_pin);

    esp_err_t ble_stop_pairing();

    esp_err_t ble_clear_paired_devices();

    esp_err_t ble_send(uint8_t *data, size_t size);

    esp_err_t ble_receive(esp_gatt_if_t gatts_if, uint8_t handle, uint16_t conn_id, uint32_t trans_id);

    esp_err_t update_connection_parameters(connection_interval_t conn_interval);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_GATT_SERVER_H__ */