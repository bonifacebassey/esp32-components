#include <stdio.h>
#include "apds9960.h"
#include "apds9960_api.h"

#define APDS9960_VL_IO (gpio_num_t)19
#define APDS9960_I2C_MASTER_SCL_IO (gpio_num_t)21 /*!< gpio number for I2C master clock */
#define APDS9960_I2C_MASTER_SDA_IO (gpio_num_t)22 /*!< gpio number for I2C master data  */
#define APDS9960_I2C_MASTER_NUM I2C_NUM_1         /*!< I2C port number for master dev */
#define APDS9960_I2C_MASTER_TX_BUF_DISABLE 0      /*!< I2C master do not need buffer */
#define APDS9960_I2C_MASTER_RX_BUF_DISABLE 0      /*!< I2C master do not need buffer */
#define APDS9960_I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */

i2c_bus_handle_t i2c_bus = NULL;
apds9960_handle_t apds9960 = NULL;

apds9960_cb_t apds9960_event_cb = NULL;
apds9960_cb_param_t *params;

static void apds9960_gpio_vl_init(void)
{
    gpio_config_t cfg;
    cfg.pin_bit_mask = BIT(APDS9960_VL_IO);
    cfg.intr_type = 0;
    cfg.mode = GPIO_MODE_OUTPUT;
    cfg.pull_down_en = 0;
    cfg.pull_up_en = 0;
    gpio_config(&cfg);
    gpio_set_level(APDS9960_VL_IO, 0);
}

/**
 * @brief i2c master initialization
 */
static void apds9960_api_init()
{
    int i2c_master_port = APDS9960_I2C_MASTER_NUM;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = APDS9960_I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = APDS9960_I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = APDS9960_I2C_MASTER_FREQ_HZ,
    };
    i2c_bus = i2c_bus_create(i2c_master_port, &conf);
    apds9960 = apds9960_create(i2c_bus, APDS9960_I2C_ADDRESS);
}

static void apds9960_gesture_task(void *arg)
{
    for (;;)
    {
        if (apds9960_event_cb == NULL)
        {
            vTaskDelay(1000 / portTICK_RATE_MS);
            continue;
        }

        uint8_t gesture = apds9960_read_gesture(apds9960);
        switch (gesture)
        {
        case APDS9960_UP:
            apds9960_event_cb(APDS9960_GESTURE_UP_EVT, params);
            break;
        case APDS9960_DOWN:
            apds9960_event_cb(APDS9960_GESTURE_DOWN_EVT, params);
            break;
        case APDS9960_LEFT:
            apds9960_event_cb(APDS9960_GESTURE_LEFT_EVT, params);
            break;
        case APDS9960_RIGHT:
            apds9960_event_cb(APDS9960_GESTURE_RIGHT_EVT, params);
            break;
        default:
            break;
        }
    }
}

void apds9960_register_callback(apds9960_cb_t callback)
{
    apds9960_event_cb = callback;
}

void apds9960_unregister_callback()
{
    apds9960_event_cb = NULL;
}

void apds9960_init()
{
    apds9960_gpio_vl_init();
    apds9960_api_init();
    apds9960_gesture_init(apds9960);
    vTaskDelay(1000 / portTICK_RATE_MS);
    xTaskCreate(apds9960_gesture_task, "gesture_task", 2048, NULL, 10, NULL);
}

void apds9960_deinit()
{
    apds9960_delete(&apds9960);
    i2c_bus_delete(&i2c_bus);
}