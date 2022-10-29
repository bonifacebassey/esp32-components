#include <stdio.h>
#include "ble_gatt_server.h"

void app_main(void)
{
    ble_start("BLE-doRit", true, BALANCED, FASTEST);
}
