#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "nvs_flash.h"
#include "esp_random.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_crc.h"
#include "espnow.h"

esp_err_t espnow_wifi_init()
{
    esp_err_t ret = ESP_OK;
    do
    {
        ret = esp_netif_init();
        if (ret != ESP_OK)
            break;

        ret = esp_event_loop_create_default();
        if (ret != ESP_OK)
            break;

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ret = esp_wifi_init(&cfg);
        if (ret != ESP_OK)
            break;

        ret = esp_wifi_set_storage(WIFI_STORAGE_RAM);
        if (ret != ESP_OK)
            break;

        ret = esp_wifi_set_mode(ESPNOW_WIFI_MODE);
        if (ret != ESP_OK)
            break;

        ret = esp_wifi_start();
        if (ret != ESP_OK)
            break;

        ret = esp_wifi_set_channel(CONFIG_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);
        if (ret != ESP_OK)
            break;

#if CONFIG_ESPNOW_ENABLE_LONG_RANGE
        ESP_ERROR_CHECK(esp_wifi_set_protocol(ESPNOW_WIFI_IF, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N | WIFI_PROTOCOL_LR));
#endif

    } while (false);

    return ret;
}

void espnow_deinit(espnow_send_param_t *send_param, QueueHandle_t queue_handle)
{
    free(send_param->buffer);
    free(send_param);
    vSemaphoreDelete(queue_handle);
    esp_now_deinit();
}