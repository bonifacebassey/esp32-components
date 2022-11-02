//---------------------------------------------------------------
// included header files:
//----------------------
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_now.h"

#include "lwip/err.h"
#include "lwip/sys.h"

//---------------------------------------------------------------
// global variables:
//-------------------------
static const char *TAG = "wiFi";

//---------------------------------------------------------------
// Static functoins prototypes:
//----------------------------

//---------------------------------------------------------------
// class and function definitions:
//------------------------------
esp_err_t wiFi_init(wifi_mode_t wiFi_mode)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    if (esp_netif_init() != ESP_OK)
        return ESP_FAIL;

    if (esp_event_loop_create_default() != ESP_OK)
        return ESP_FAIL;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK)
        return ESP_FAIL;

    if (esp_wifi_set_storage(WIFI_STORAGE_RAM) != ESP_OK)
        return ESP_FAIL;

    if (esp_wifi_set_mode(wiFi_mode) != ESP_OK)
        return ESP_FAIL;

    ESP_LOGI(TAG, "wiFi init completed");

    return ESP_OK;
}

esp_err_t wiFi_deinit(void)
{
    if (esp_wifi_stop() != ESP_OK)
        return ESP_FAIL;

    vTaskDelay(500 / portTICK_PERIOD_MS);

    if (esp_wifi_deinit() != ESP_OK)
        return ESP_FAIL;

    // Note: Deinitialization is not supported yet
    // if (esp_netif_deinit() != ESP_OK)
    //    return ESP_FAIL;

    ESP_LOGI(TAG, "wiFi deinit completed");

    return ESP_OK;
}

void wiFi_print_connction(const char *ssid, const char *password, esp_netif_ip_info_t ip)
{
    ESP_LOGI(TAG, "Connection Information:");
    ESP_LOGI(TAG, "*****************************");
    ESP_LOGI(TAG, "*  SSID:      %s", ssid);
    ESP_LOGI(TAG, "*  Password:  %s", password);
    ESP_LOGI(TAG, "*  IPAddress: " IPSTR, IP2STR(&ip.ip));
    ESP_LOGI(TAG, "*  Gateway:   " IPSTR, IP2STR(&ip.gw));
    ESP_LOGI(TAG, "*  Netmask:   " IPSTR, IP2STR(&ip.netmask));
    ESP_LOGI(TAG, "*****************************\n");
}

void sta_print_ap_MAC()
{
    wifi_ap_record_t mac;
    if (esp_wifi_sta_get_ap_info(&mac) == ESP_OK)
    {
        ESP_LOGI(TAG, "MAC details \n\r bssid: %X %X %X %X %X %X",
                 mac.bssid[0],
                 mac.bssid[1],
                 mac.bssid[2],
                 mac.bssid[3],
                 mac.bssid[4],
                 mac.bssid[5]);
    }
    else
    {
        ESP_LOGI(TAG, "Failed to get MAC details");
    }
}