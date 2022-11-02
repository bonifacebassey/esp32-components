#ifndef __WIFI_ESPNOW__H_
#define __WIFI_ESPNOW__H_

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------
// Includes
//-------------------------------------------
#include "esp_err.h"

/* ESPNOW can work in both station and softap mode. It is configured in menuconfig. */
#if CONFIG_ESPNOW_WIFI_MODE_STATION
#define ESPNOW_WIFI_MODE WIFI_MODE_STA
#define ESPNOW_WIFI_IF ESP_IF_WIFI_STA
#else
#define ESPNOW_WIFI_MODE WIFI_MODE_AP
#define ESPNOW_WIFI_IF ESP_IF_WIFI_AP
#endif

    //------------------------------------------
    // Prototypes
    //-------------------------------------------

    /**
     * @brief : Initialize wifi module with espnow Mode
     * @param  : None
     * @return : none
     */
    esp_err_t wifi_init_ESPNOW_mode();

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_ESPNOW__H_ */