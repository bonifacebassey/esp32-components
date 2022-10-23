#ifndef __WLAN_H__
#define __WLAN_H__

#include "esp_wifi.h"
#include "esp_err.h"

#define SSID_SIZE (32)
#define PASSWORD_SIZE (64)

typedef enum
{
    SELECT_AP_MODE = 0,
    SELECT_STA_MODE
} wifi_select_mode_t;

typedef struct
{
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];
    esp_netif_ip_info_t ipInfo;
    wifi_select_mode_t mode;
} connection_info_t;

void wlan_dump_info(connection_info_t *info);

esp_err_t wlan_initialize(void);

esp_err_t wlan_deinitialize(void);

#endif /* __WLAN_H__ */