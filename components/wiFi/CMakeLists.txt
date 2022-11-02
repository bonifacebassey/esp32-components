idf_component_register(SRCS "wiFi_station.c" "wiFi_softAP.c" "wiFi_espnow.c" "wiFi.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "nvs_flash")
