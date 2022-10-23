#ifndef __WLAN_SOFT_AP_H__
#define __WLAN_SOFT_AP_H__

#include "esp_err.h"

esp_err_t wlan_softap_start(const char *ssid, const char *password);

esp_err_t wlan_softap_stop();

#endif /* __WLAN_SOFT_AP_H__ */