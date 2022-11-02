#ifndef __WIFI_SOFTAP_H__
#define __WIFI_SOFTAP_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_err.h"

    //------------------------------------------
    // Prototypes
    //-------------------------------------------

    /**
     * @brief : Initialize the SOFTAP wifi mode
     *
     * @param  ssid: AP ssid
     * @param  password: AP password
     *
     * @return
     *    - ESP_OK: succeed
     *    - others: failed
     */
    esp_err_t wifi_init_SOFTAP_mode(const char *ssid, const char *password);

    /**
     * @brief : Deinitialize the SOFTAP wifi mode
     *
     * @param  : None
     *
     * @return
     *    - ESP_OK: succeed
     *    - others: failed
     */
    esp_err_t wifi_deinit_SOFTAP_mode();

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_SOFTAP_H__ */