#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "wlan.h"

static const char *TAG = "wlan";
connection_info_t connection_Info;

void wlan_dump_info(connection_info_t *info)
{
    ESP_LOGI(TAG, "Connection Information:");
    ESP_LOGI(TAG, "*****************************");
    ESP_LOGI(TAG, "*  SSID:      %s", info->ssid);
    ESP_LOGI(TAG, "*  Password:  %s", info->password);
    ESP_LOGI(TAG, "*  IPAddress: " IPSTR, IP2STR(&info->ipInfo.ip));
    ESP_LOGI(TAG, "*  Gateway:   " IPSTR, IP2STR(&info->ipInfo.gw));
    ESP_LOGI(TAG, "*  Netmask:   " IPSTR, IP2STR(&info->ipInfo.netmask));
    ESP_LOGI(TAG, "*****************************\n");
}

esp_err_t wlan_initialize(void)
{
    esp_err_t status = ESP_OK;
    do
    {
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        status = esp_netif_init();
        if (status != ESP_OK)
        {
            ESP_LOGI(TAG, "init netif failed");
            break;
        }

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        status = esp_wifi_init(&cfg);
        if (status != ESP_OK)
        {
            ESP_LOGI(TAG, "initialize default config failed");
            break;
        }

        status = esp_event_loop_create_default();
        if (status != ESP_OK)
        {
            ESP_LOGI(TAG, "create event loop failed");
            break;
        }

    } while (false);

    return status;
}

esp_err_t wlan_deinitialize(void)
{
    esp_err_t status = ESP_OK;

    do
    {
        status = esp_wifi_deinit();
        if (status != ESP_OK)
            break;

        // Note: Deinitialization is not supported yet
        // status = esp_netif_deinit();
        // if (status != ESP_OK)
        //     break;

    } while (false);

    return status;
}
