#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------
// Includes
//-------------------------------------------
#include "esp_err.h"

    typedef struct
    {
        const uint8_t* data;
        size_t size;
    } uart_cb_param_t;

    /**
     * @brief uart-onDataReceive callback function type
     * @param event         Event type
     * @param data          Point to callback value
     */
    typedef void (*uart_onDataReceive_cb_t)(uart_cb_param_t* param);

    /**
     * @brief This function is called to register a uart data receive callback
     * @param callback  callback function
     */
    void uart_onDataReceive_register_callback(uart_onDataReceive_cb_t callback);

    /**
     * @brief This function is called to unregister a uart data receive callback
     */
    void uart_onDataReceive_unregister_callback();

    //------------------------------------------
    // Prototypes
    //-------------------------------------------

    /**
     * @brief Initialize the UART driver
     */
    void uart_init();

    /**
     * @brief Sends data to UART.
     * @param data			The data buffer.
     * @param dataSize		The data size.
     * @return The number of bytes sent.
     */
    int uart_send_data(const uint8_t* data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __UART_H__ */