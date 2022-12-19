idf_component_register(SRCS "espnow_sender.c" "espnow_receiver.c" "espnow.c"
                    INCLUDE_DIRS "include"
                    REQUIRES "nvs_flash")
