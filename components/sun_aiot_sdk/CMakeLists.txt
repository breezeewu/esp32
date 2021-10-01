idf_component_register(SRCS "lbbase64.c"
                            "lbkv_list.c"
                            "lbmbedtls.c"
                            "lbhttp.c"
                            "lbnet.c"
                            "lbstring.c"
                            "lbthread.c"
                            "lburl.c"
                            "esp_http.c"
                            "esp_ota.c"
                            "sun_mqtt_client.c"
                            "aiot_util.c"
                            "sun_aiot.c"
                    INCLUDE_DIRS .
                    PRIV_INCLUDE_DIRS sun_sdk_inc
                    REQUIRES lwip nghttp nvs_flash json mbedtls tcp_transport mqtt esp_http_client app_update)
