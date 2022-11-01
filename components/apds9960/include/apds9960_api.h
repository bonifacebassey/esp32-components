
#ifndef _APDS9960_API_H_
#define _APDS9960_API_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /// APDS9960 callback function events
    typedef enum
    {
        APDS9960_GESTURE_UP_EVT = 0x01,    /*!< When up gesture is detected, the event comes */
        APDS9960_GESTURE_DOWN_EVT = 0x02,  /*!< When down gesture is detected, the event comes */
        APDS9960_GESTURE_LEFT_EVT = 0x03,  /*!< When left gesture is detected, the event comes */
        APDS9960_GESTURE_RIGHT_EVT = 0x04, /*!< When right gesture is detected, the event comes */

    } apds9960_cb_event_t;

    typedef struct
    {
        int id;
    } apds9960_cb_param_t;

    /**
     * @brief Gesture callback function type
     * @param event : Event type
     * @param param : Point to callback parameter
     */
    typedef void (*apds9960_cb_t)(apds9960_cb_event_t event, apds9960_cb_param_t *param);

    /**
     * @brief           This function is called to register a gesture event, such as gesture detection
     *
     * @param[in]       callback: callback function
     */
    void apds9960_register_callback(apds9960_cb_t callback);

    /**
     * @brief           This function is called to unregister a gesture event
     */
    void apds9960_unregister_callback();

    /**
     * @brief           This function is called to initialize the apds9960-gesture and start sensing
     * @note            It's recommended to call apds9960_register_callback(apds9960_cb_t callback) before calling this function
     */
    void apds9960_init();

    /**
     * @brief           This function is called to deinitialize the apds9960-gesture and stop sensing
     */
    void apds9960_deinit();

#ifdef __cplusplus
}
#endif

#endif