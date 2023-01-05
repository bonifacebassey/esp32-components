#include <stdio.h>
#include "uart.h"

static void uart_onDataReceive(uart_cb_param_t *param)
{
}
void app_main(void)
{
    uart_onDataReceive_register_callback(uart_onDataReceive);
    uart_init();
}
