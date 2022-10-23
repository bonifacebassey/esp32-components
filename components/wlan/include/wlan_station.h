#ifndef __WLAN_STATION_H__
#define __WLAN_STATION_H__

#include "esp_err.h"

esp_err_t wlan_station_start(const char *ssid, const char *password);

esp_err_t wlan_station_stop();

#endif /* __WLAN_STATION_H__ */