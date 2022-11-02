#ifndef __WIFI_SETUP_H_
#define __WIFI_SETUP_H_

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------
// Includes
//-------------------------------------------
#include "esp_err.h"

    /**
     * @brief : Initialize the wifi module
     * @param  : None
     * @return : none
     */
    esp_err_t wiFi_init(wifi_mode_t wiFi_mode);

    /**
     * @brief : Deinitialize the wifi module
     * @param  : None
     * @return : none
     */
    esp_err_t wiFi_deinit(void);

    /**
     * @brief : Prints the connection information
     * @param ssid : connection ssid
     * @param password : connection password
     * @param ip : connection ip information
     * @return : none
     */
    void wiFi_print_connction(const char *ssid, const char *password, esp_netif_ip_info_t ip);

    /**
     * @brief : Prints the MAC address of the AP which ESP32 station is associated with
     * @param  : None
     * @return : none
     */
    void sta_print_ap_MAC();

#ifdef __cplusplus
}
#endif

#endif /* __WIFI_SETUP_H_ */