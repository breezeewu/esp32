#
# Component Makefile
#
COMPONENT_ADD_INCLUDEDIRS := mbedtls/include esp_crt_bundle/include
COMPONENT_ADD_INCLUDEDIRS := sun_sdk_inc
COMPONENT_SRCDIRS := ./
COMPONENT_SUBMODULES := ./
idf_component_register(SRCS "lbbase64.c"
                            "lbkv_list.c"
                            "lbmbedtls.c"
                            "lbhttp.c"
                            "lbnet.c"
                            "lbstring.c"
                            "lbthread.c"
                            "lburl.c"
                            "esp_ota.c"
                            "esp_http.c"
                            "sun_mqtt_client.c"
                            "aiot_util.c"
                            "sun_aiot.c"
COMPONENT_OBJS := lbbase64.o lbkv_list.o lbmbedtls.o lbhttp.o lbnet.o lbstring.o lbthread.o esp_http.o esp_ota.o lburl.o sun_mqtt_client.o aiot_util.o sun_aiot.o
