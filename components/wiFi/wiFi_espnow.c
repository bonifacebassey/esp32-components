#include "esp_wifi.h"
#include "esp_log.h"

#include "wiFi.h"
#include "wiFi_espnow.h"

static const char *TAG = "wiFi-espnow";

esp_err_t wifi_init_ESPNOW_mode()
{
    if (wiFi_init(ESPNOW_WIFI_MODE) != ESP_OK)
        return ESP_FAIL;

    if (esp_wifi_start() != ESP_OK)
        return ESP_FAIL;

    if (esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE) != ESP_OK)
        return ESP_FAIL;

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
    ESP_ERROR_CHECK(esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif

    ESP_LOGI(TAG, "wiFi ESPNOW mode init completed");

    return ESP_OK;
}