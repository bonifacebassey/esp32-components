#ifndef __ESPNOW_RECEIVER__H_
#define __ESPNOW_RECEIVER__H_

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
     * @brief : Initialize espnow in receiver Mode
     * @param  : None
     * @return : none
     */
    esp_err_t espnow_receiver_init();

#ifdef __cplusplus
}
#endif

#endif /* __ESPNOW_RECEIVER__H_ */