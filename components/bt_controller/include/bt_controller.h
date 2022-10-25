#ifndef __BT_CONTROLLER_H__
#define __BT_CONTROLLER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_bt.h"

    typedef enum
    {
        MINIMAL,
        BALANCED,
        FULL,
    } power_level_t;

    esp_err_t bt_controller_start(esp_bt_mode_t bt_mode);

    esp_err_t bt_controller_stop();

    /**
     * @brief	Bluetooth TX power level(index), corresponding to power(dbm).
     * 			ESP_PWR_LVL_N12 = 0,        Corresponding to -12dbm
     *  		ESP_PWR_LVL_N9  = 1,        Corresponding to  -9dbm
     *  		ESP_PWR_LVL_N6  = 2,        Corresponding to  -6dbm
     *  		ESP_PWR_LVL_N3  = 3,        Corresponding to  -3dbm
     *  		ESP_PWR_LVL_N0  = 4,        Corresponding to   0dbm
     *  		ESP_PWR_LVL_P3  = 5,        Corresponding to  +3dbm
     *  		ESP_PWR_LVL_P6  = 6,        Corresponding to  +6dbm
     *  		ESP_PWR_LVL_P9  = 7,        Corresponding to  +9dbm
     * @param  	power_level: Minimal, Balanced, Full. Given in terms of minimum and maximum values
     * @return 	ESP_OK - success, other - failed
     */
    esp_err_t bt_set_power_level(power_level_t bt_power_level);

    power_level_t bt_get_power_level();

    /**
     * @brief  	Set BLE TX power
     *         	Connection Tx power should only be set after connection created.
     * @param  	power_type : The type of which tx power, could set Advertising/Scanning/Default (ESP_PWR_LVL_P3 as default for ADV/SCAN/CONN0-9.)
     * @param  	power_level: Power level(index) corresponding to absolute value(dbm)
     * @return 	ESP_OK - success, other - failed
     */
    esp_err_t ble_set_power_level(esp_ble_power_type_t power_type, power_level_t ble_power_level);

    power_level_t ble_get_power_level();

#ifdef __cplusplus
}
#endif

#endif /* __BT_CONTROLLER_H__ */