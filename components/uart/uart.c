#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"

#include "uart.h"

/**
 * - Port: UART1
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: on
 * - Pin assignment: TxD (default), RxD (default)
 */
#define UART_NUM (UART_NUM_1)
#define UART_TXD (GPIO_NUM_17)
#define UART_RXD (GPIO_NUM_16)
#define UART_RTS (UART_PIN_NO_CHANGE)
#define UART_CTS (UART_PIN_NO_CHANGE)

#define PATTERN_CHR_NUM (3) /*!< Set the number of consecutive and identical characters received by receiver which defines a UART pattern*/
#define BUF_SIZE (1024)
static QueueHandle_t uart1_queue;

uart_onDataReceive_cb_t onDataReceive_callback_handler = NULL;
uart_cb_param_t *params;

static const char *TAG = "uart";

static void uart_tx_task(void *arg)
{
    static int num = 0;
    char *data = (char *)malloc(100);
    while (1)
    {
        sprintf(data, "Hello world index = %d\r\n", num++);
        uart_send_data((uint8_t *)data, strlen(data));
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void uart_rx_task(void *arg)
{
    uart_event_t event;
    size_t buffered_size;
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE + 1);

    for (;;)
    {
        // Waiting for UART event.
        if (xQueueReceive(uart1_queue, (void *)&event, (portTickType)portMAX_DELAY))
        {
            bzero(data, BUF_SIZE);
            switch (event.type)
            {
            case UART_DATA:
                uart_read_bytes(UART_NUM, data, event.size, portMAX_DELAY);
                if (event.size)
                {
                    ESP_LOGI(TAG, "Read %d bytes: '%s'", event.size, data);
                    params->data = data;
                    params->size = event.size;
                    onDataReceive_callback_handler(params);
                }
                break;

            case UART_FIFO_OVF:
                ESP_LOGI(TAG, "hw fifo overflow");
                uart_flush_input(UART_NUM);
                xQueueReset(uart1_queue);
                // notify onUartError(event.type);
                break;

            case UART_BUFFER_FULL:
                ESP_LOGI(TAG, "ring buffer full");
                uart_flush_input(UART_NUM);
                xQueueReset(uart1_queue);
                break;

            case UART_PATTERN_DET:
            {
                uart_get_buffered_data_len(UART_NUM, &buffered_size);
                int pos = uart_pattern_pop_pos(UART_NUM);
                ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
                if (pos == -1)
                {
                    uart_flush_input(UART_NUM);
                }
                else
                {
                    uart_read_bytes(UART_NUM, data, pos, 100 / portTICK_PERIOD_MS);
                    uint8_t pat[PATTERN_CHR_NUM + 1];
                    memset(pat, 0, sizeof(pat));
                    uart_read_bytes(UART_NUM, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);
                    ESP_LOGI(TAG, "read data: %s", data);
                    ESP_LOGI(TAG, "read pat : %s", pat);
                }
            }
            break;
            default:
                break;
            }
        }
    }
    free(data);
    data = NULL;
    vTaskDelete(NULL);
}

void uart_init()
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    int intr_alloc_flags = 0;
#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    // Install UART driver, and get the queue.
    uart_driver_install(UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &uart1_queue, intr_alloc_flags);
    // Configure UART parameters
    uart_param_config(UART_NUM, &uart_config);
    // Set UART pins (using UART1 default pins ie no changes.)
    uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS);

    xTaskCreate(uart_rx_task, "uart_rx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(uart_tx_task, "uart_tx_task", 1024 * 2, NULL, configMAX_PRIORITIES - 2, NULL);
}

void uart_onDataReceive_register_callback(uart_onDataReceive_cb_t callback)
{
    onDataReceive_callback_handler = callback;
}

void uart_onDataReceive_unregister_callback()
{
    onDataReceive_callback_handler = NULL;
}

int uart_send_data(const uint8_t *data, size_t size)
{
    const int txBytes = uart_write_bytes(UART_NUM, data, size);
    ESP_LOGI(TAG, "(%d bytes) %s", txBytes, data);
    return txBytes;
}