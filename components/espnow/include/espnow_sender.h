#ifndef __ESPNOW_SENDER__H_
#define __ESPNOW_SENDER__H_

#ifdef __cplusplus
extern "C"
{
#endif

//------------------------------------------
// Includes
//-------------------------------------------
#include "esp_err.h"

    //------------------------------------------
    // Prototypes
    //-------------------------------------------

    /**
     * @brief : Initialize espnow in sender Mode
     * @param  : None
     * @return : none
     */
    esp_err_t espnow_sender_init();

#ifdef __cplusplus
}
#endif

#endif /* __ESPNOW_SENDER__H_ */