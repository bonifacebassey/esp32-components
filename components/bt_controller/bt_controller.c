#include "sdkconfig.h"

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#include "esp_bt.h"
#include "esp_log.h"
#include "esp_err.h"

#include "bt_controller.h"

static const char *const TAG = "btcontroller";

static bool bt_controller_idle()
{
    return (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE);
}

static bool bt_controller_inited()
{
    return (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_INITED);
}

bool bt_controller_enabled()
{
    return (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_ENABLED);
}

esp_err_t bt_controller_start(esp_bt_mode_t bt_mode)
{
    if (bt_controller_enabled())
    {
        return ESP_OK;
    }

    if (bt_controller_idle())
    {
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if (esp_bt_controller_init(&bt_cfg) != ESP_OK)
        {
            ESP_LOGE(TAG, "init controller failed");
            return ESP_FAIL;
        }
    }

    if (bt_controller_inited())
    {
        if (esp_bt_controller_enable(bt_mode) != ESP_OK)
        {
            ESP_LOGE(TAG, "enable controller failed");
            return ESP_FAIL;
        }
    }

    if (bt_controller_enabled())
        return ESP_OK;

    ESP_LOGE(TAG, "start controller failed");
    return ESP_FAIL;
}

esp_err_t bt_controller_stop()
{
    if (bt_controller_idle())
    {
        return ESP_OK;
    }

    if (bt_controller_enabled())
    {
        if (esp_bt_controller_disable() != ESP_OK)
        {
            ESP_LOGE(TAG, "disable controller failed");
            return ESP_FAIL;
        }
        while (bt_controller_enabled())
            ;
    }

    if (bt_controller_inited())
    {
        if (esp_bt_controller_deinit() != ESP_OK)
        {
            ESP_LOGE(TAG, "deinit controller failed");
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "deinit controller success");
        return ESP_OK;
    }

    ESP_LOGE(TAG, "stop controller failed");
    return ESP_FAIL;
}

esp_err_t bt_set_power_level(power_level_t bt_power_level)
{
    esp_power_level_t power_level_min = ESP_PWR_LVL_N0;
    esp_power_level_t power_level_max = ESP_PWR_LVL_P3;

    switch (bt_power_level)
    {
    case MINIMAL:
        power_level_min = ESP_PWR_LVL_N12;
        power_level_max = ESP_PWR_LVL_N3;
        break;

    case BALANCED:
        power_level_min = ESP_PWR_LVL_N3;
        power_level_max = ESP_PWR_LVL_P3;
        break;

    case FULL:
        power_level_min = ESP_PWR_LVL_P3;
        power_level_max = ESP_PWR_LVL_P9;
        break;
    }

    return esp_bredr_tx_power_set(power_level_min, power_level_max);
}

esp_err_t ble_set_power_level(esp_ble_power_type_t power_type, power_level_t ble_power_level)
{
    esp_power_level_t power_level = ESP_PWR_LVL_N0;

    switch (ble_power_level)
    {
    case MINIMAL:
        power_level = ESP_PWR_LVL_N12;
        break;

    case BALANCED:
        power_level = ESP_PWR_LVL_N0;
        break;

    case FULL:
        power_level = ESP_PWR_LVL_P9;
        break;
    }

    return esp_ble_tx_power_set(power_type, power_level);
}
#else
bool bt_controller_enabled()
{
    return false;
}

bool bt_controller_start(esp_bt_mode_t mode)
{
    return false;
}

bool bt_controller_stop()
{
    return false;
}
#endif