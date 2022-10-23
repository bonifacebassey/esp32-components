idf_component_register(SRCS "ble_ibeacon.c" "ble_ibeacon_api.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "nvs_flash" "bt")
