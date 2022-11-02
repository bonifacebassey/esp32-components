#include <string.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "wiFi.h"
#include "wiFi_softAP.h"

static const char *TAG = "wiFi-softAP";

esp_event_handler_instance_t event_instance;
static esp_netif_t *ap_handle = NULL;

static void wiFi_softap_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);
    }
}

esp_err_t wifi_init_SOFTAP_mode(const char *ssid, const char *password)
{
    if (wiFi_init(WIFI_MODE_AP) != ESP_OK)
    {
        ESP_LOGE(TAG, "initialize wifi failed");
        return ESP_FAIL;
    }

    ap_handle = esp_netif_create_default_wifi_ap();

    if (esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &wiFi_softap_event_handler,
                                            NULL,
                                            &event_instance) != ESP_OK)
    {
        ESP_LOGE(TAG, "register event handler failed");
        return ESP_FAIL;
    }

    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = strlen(ssid),
            .channel = 1,
            .max_connection = 4,
            .beacon_interval = 100,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
            .ssid_hidden = 0,
        },
    };
    memcpy(ap_config.ap.ssid, ssid, strlen(ssid));
    memcpy(ap_config.ap.password, password, strlen(password));
    if (strlen(password) == 0)
    {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    if (esp_wifi_set_config(WIFI_IF_AP, &ap_config) != ESP_OK)
    {
        ESP_LOGE(TAG, "set wifi ap configs failed");
        return ESP_FAIL;
    }

    if (esp_wifi_start() != ESP_OK)
    {
        ESP_LOGE(TAG, "start wifi failed");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "start wlanAP completed.");

    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(ap_handle, &ip);
    wiFi_print_connction(ssid, password, ip);

    return ESP_OK;
}

esp_err_t wifi_deinit_SOFTAP_mode()
{
    if (esp_event_handler_instance_unregister(WIFI_EVENT,
                                              ESP_EVENT_ANY_ID,
                                              event_instance) != ESP_OK)
    {
        ESP_LOGE(TAG, "unregister event handler failed");
        return ESP_FAIL;
    }

    if (esp_event_loop_delete_default() != ESP_OK)
    {
        ESP_LOGE(TAG, "delete event loop failed");
        return ESP_FAIL;
    }

    if (esp_wifi_stop() != ESP_OK)
    {
        ESP_LOGE(TAG, "stop wifi failed");
        return ESP_FAIL;
    }

    return ESP_OK;
}