#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_log.h"
#include "esp_now.h"

#include "espnow.h"
#include "espnow_receiver.h"

static const char *TAG = "espnow-receiver";

static QueueHandle_t s_recv_queue;

static void espnow_rcv_cb(const uint8_t *mac_addr, const uint8_t *data, int len)
{
    espnow_event_t evt;
    espnow_event_recv_cb_t *recv_cb = &evt.info.recv_cb;

    if (mac_addr == NULL || data == NULL || len <= 0)
    {
        ESP_LOGE(TAG, "Receive cb arg error");
        return;
    }

    evt.id = ESPNOW_RECV_CB;
    memcpy(recv_cb->mac_addr, mac_addr, ESP_NOW_ETH_ALEN);
    recv_cb->data = malloc(len);
    if (recv_cb->data == NULL)
    {
        ESP_LOGE(TAG, "Malloc receive data fail");
        return;
    }
    memcpy(recv_cb->data, data, len);
    recv_cb->data_len = len;
    if (xQueueSend(s_recv_queue, &evt, ESPNOW_MAXDELAY) != pdTRUE)
    {
        ESP_LOGW(TAG, "Send receive queue fail");
        free(recv_cb->data);
    }
}

static void handle_rcv_data(espnow_event_recv_cb_t *recv_cb)
{
    ESP_LOGI(TAG, "Data from " MACSTR ": valueA - %u, valueB - %s, dataLen - %d",
             MAC2STR(recv_cb->mac_addr),
             recv_cb->data->valueA,
             recv_cb->data->valueB ? "on" : "off",
             recv_cb->data_len);
}

static void espnow_rcv_task(void *pvParameter)
{
    espnow_event_t evt;
    for (;;)
    {
        if (xQueueReceive(s_recv_queue, &evt, portMAX_DELAY) == pdTRUE)
        {
            handle_rcv_data(&evt.info.recv_cb);
        }
    }
}

esp_err_t espnow_receiver_init()
{
    esp_err_t ret = ESP_OK;
    do
    {
        s_recv_queue = xQueueCreate(ESPNOW_QUEUE_SIZE, sizeof(espnow_event_t));
        if (s_recv_queue == NULL)
        {
            ESP_LOGE(TAG, "Create mutex fail");
            return ESP_FAIL;
        }
        BaseType_t err = xTaskCreate(espnow_rcv_task, "espnow_rcv_task", 2048, NULL, 4, NULL);
        assert(err == pdPASS);

        ret = espnow_wifi_init();
        if (ret != ESP_OK)
            break;

        ret = esp_now_init();
        if (ret != ESP_OK)
            break;

        ret = esp_now_register_recv_cb(espnow_rcv_cb);
        if (ret != ESP_OK)
            break;

        ret = esp_now_set_pmk((uint8_t *)CONFIG_ESPNOW_PMK);
        if (ret != ESP_OK)
            break;

#if CONFIG_ESP_WIFI_STA_DISCONNECTED_PM_ENABLE
        ESP_ERROR_CHECK(esp_now_set_wake_window(65535));
#endif

    } while (false);

    return ret;
}