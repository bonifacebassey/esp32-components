#include <string.h>

#include "esp_wifi.h"
#include "esp_log.h"
#include "wlan.h"
#include "wlan_soft_ap.h"

static const char *TAG = "wlan softAP";

esp_event_handler_instance_t event_instance;
static esp_netif_t *ap_handle = NULL;

static void wlan_softap_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
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

esp_err_t wlan_softap_start(const char *ssid, const char *password)
{
    esp_err_t status = ESP_OK;
    do
    {
        status = wlan_initialize();
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "initialize wifi failed");
            break;
        }

        ap_handle = esp_netif_create_default_wifi_ap();

        status = esp_event_handler_instance_register(WIFI_EVENT,
                                                     ESP_EVENT_ANY_ID,
                                                     &wlan_softap_event_handler,
                                                     NULL,
                                                     &event_instance);
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "register event handler failed");
            break;
        }

        status = esp_wifi_set_mode(WIFI_MODE_AP);
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "set wifi AP mode failed");
            break;
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
            ap_config.ap.authmode = WIFI_AUTH_OPEN;

        status = esp_wifi_set_config(WIFI_IF_AP, &ap_config);
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "set wifi ap configs failed");
            break;
        }

        status = esp_wifi_start();
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "start wifi failed");
            break;
        }

        ESP_LOGI(TAG, "start wlanAP completed.");

        esp_netif_ip_info_t ip_info;
        esp_netif_get_ip_info(ap_handle, &ip_info);

        connection_info_t conn;
        memcpy(conn.ssid, ssid, strlen(ssid));
        memcpy(conn.password, password, strlen(password));
        conn.ipInfo = ip_info;
        conn.mode = SELECT_AP_MODE;
        wlan_dump_info(&conn);

    } while (false);

    return status;
}

esp_err_t wlan_softap_stop()
{
    esp_err_t status = ESP_OK;
    do
    {
        status = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, event_instance);
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "unregister event handler failed");
            break;
        }
        status = esp_event_loop_delete_default();
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "delete event loop failed");
            break;
        }

        status = esp_wifi_stop();
        if (status != ESP_OK)
        {
            ESP_LOGE(TAG, "stop wifi failed");
            break;
        }

    } while (false);

    return status;
}