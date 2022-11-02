#ifndef __WIFI_STATION_H__
#define __WIFI_STATION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

    //------------------------------------------
    // Prototypes
    //-------------------------------------------

    /**
     * @brief : Initialize the STATION wifi mode
     *
     * @param  ssid: STA ssid
     * @param  password: STA password
     *
     * @return
     *    - ESP_OK: succeed
     *    - others: failed
     */
    esp_err_t wifi_init_STATION_mode(const char *ssid, const char *password);

    /**
     * @brief : Deinitialize the STATION wifi mode
     *
     * @param  : None
     *
     * @return
     *    - ESP_OK: succeed
     *    - others: failed
     */
    esp_err_t wifi_deinit_STATION_mode();

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_SOFTAP_H__ */