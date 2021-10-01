#include "lbmacro.h"
#include "esp_ota.h"
#include <stdint.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_http_client.h"
#include "lbthread.h"
#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */
#define CONFIG_EXAMPLE_GPIO_DIAGNOSTIC 4
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };

#define OTA_URL_SIZE 256
#define CONFIG_SUN_DEFAULT_RECV_TIMEOUT         10000

typedef struct{
    struct lbthread_context* pota_thread;
    char* purl;
    esp_http_client_config_t    cfg;
    esp_http_client_handle_t*   pclient;
    e_aiot_http_msg_type        msg_type;
    http_download_info          hdp;
    int                         percent;
} esp_ota_task;
static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    lberror("Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void print_sha256 (const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    lbinfo("%s: %s", label, hash_print);
}

static void infinite_loop(void)
{
    int i = 0;
    lbinfo("When a new firmware is available on the server, press the reset button to download it");
    while(1) {
        lbinfo("Waiting for a new firmware ... %d", ++i);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

int esp_init()
{
    // Initialize NVS.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // OTA app partition table has a smaller NVS partition size than the non-OTA
        // partition table. This size mismatch may cause NVS initialization to fail.
        // If this happens, we erase NVS partition and initialize NVS again.
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();

    return 0;
}
static bool diagnostic(void)
{
    gpio_config_t io_conf;
    io_conf.intr_type    = GPIO_PIN_INTR_DISABLE;
    io_conf.mode         = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);
    lbinfo("Diagnostics (5 sec)...\n");
    //lbinfo("Diagnostics (5 sec)...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

    gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
    return diagnostic_is_ok;
}

int esp_ota_prepare()
{
    uint8_t sha_256[HASH_LEN] = { 0 };
    esp_partition_t partition;
    lbinfo("esp_ota_prepare begin\n");
    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    //print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    //print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    //print_sha256(sha_256, "SHA-256 for current firmware: ");

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            // run diagnostic function ...
            bool diagnostic_is_ok = diagnostic();
            if (diagnostic_is_ok) {
                lbinfo("Diagnostics completed successfully! Continuing execution ...\n");
                //lbinfo("Diagnostics completed successfully! Continuing execution ...");
                esp_ota_mark_app_valid_cancel_rollback();
            } else {
                lbinfo("Diagnostics failed! Start rollback to the previous version ...\n");
                //lberror("Diagnostics failed! Start rollback to the previous version ...");
                esp_ota_mark_app_invalid_rollback_and_reboot();
            }
        }
    }
    lbinfo("esp_ota_prepare end\n");
    return 0;
}

int esp_http_ota(const char* purl)
{
    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = NULL;

    lbinfo("Starting OTA example");
    err = esp_ota_prepare();
    lbinfo("err:%d = esp_ota_prepare()\n", err);

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running) {
        lbwarn("Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x\n", configured->address, running->address);
        lbwarn("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)\n");
        //ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x", configured->address, running->address);
        //ESP_LOGW(TAG, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    }
    lbinfo("Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);
    //lbinfo("Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = purl,
        .cert_pem = NULL,
        .timeout_ms = CONFIG_SUN_DEFAULT_RECV_TIMEOUT,
        .keep_alive_enable = true,
    };

/*#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        lberror("Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif*/

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        lberror("Failed to initialise HTTP connection");
        //lberror("Failed to initialise HTTP connection");
        task_fatal_error();
    }
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        lberror("Failed to open HTTP connection: %s", esp_err_to_name(err));
        //lberror("Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        task_fatal_error();
    }
    esp_http_client_fetch_headers(client);

    update_partition = esp_ota_get_next_update_partition(NULL);
    lbinfo("Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);
    //lbinfo("Writing to partition subtype %d at offset 0x%x",
    //         update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    int binary_file_length = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        lbinfo("data_read:%d = esp_http_client_read(client:%p, ota_write_data, BUFFSIZE:%d)\n", data_read, client, BUFFSIZE);
        if (data_read < 0) {
            lberror("Error: SSL data read error");
            http_cleanup(client);
            task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    lbinfo("New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        lbinfo("Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        lbinfo("Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            lbwarn("New version is the same as invalid version.");
                            lbwarn("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            lbwarn("The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            infinite_loop();
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECKptc
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        lbwarn("Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);
                        infinite_loop();
                    }
#endif

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                    if (err != ESP_OK) {
                        lberror("esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        task_fatal_error();
                    }
                    lbinfo("esp_ota_begin succeeded");
                } else {
                    lberror("received package is not fit len");
                    http_cleanup(client);
                    task_fatal_error();
                }
            }
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                task_fatal_error();
            }
            binary_file_length += data_read;
            lbinfo("Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                lberror("Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                lbinfo("Connection closed");
                break;
            }
        }
    }
    lbinfo("Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(client) != true) {
        lberror("Error in receiving complete file");
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            lberror("Image validation failed, image is corrupted");
        }
        lberror("esp_ota_end failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        lberror("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }
    lbinfo("Prepare to restart system!");
    esp_restart();
    return 0;
}

void* ota_thread_proc(void* arg)
{
    esp_err_t err;
    esp_ota_handle_t update_handle = 0 ;
    unsigned long begin_time = lbget_sys_time();
    unsigned long last_update_time = begin_time;
    unsigned long last_update_bytes = 0;
    const esp_partition_t *update_partition = NULL;
    struct lbthread_context* ptc = (struct lbthread_context*)arg;
    lbcheck_pointer(ptc, NULL, "Invalid parameter, ptc:%p\n", ptc);
    esp_ota_task* peot = lbthread_get_owner(ptc);
    lbcheck_pointer(peot, NULL, "Invalid parameter, peot:%p\n", peot);
    //lbinfo("ota thread begin, ptc:%p, peot:%p\n", ptc, peot);

    err = esp_ota_prepare();
    //lbinfo("err:%d = esp_ota_prepare()\n", err);
    //lbcheck_return(err, "err:%d = esp_ota_prepare()\n", err);
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    lbcheck_pointer(configured, NULL, "configured:%p = esp_ota_get_boot_partition()\n", configured);
    //lbinfo("configured:%p = esp_ota_get_boot_partition()\n", configured);
    const esp_partition_t *running = esp_ota_get_running_partition();
    lbcheck_pointer(running, NULL, "running:%p = esp_ota_get_running_partition()\n", running);
    //lbinfo("running:%p = esp_ota_get_running_partition()\n", running);
    if (configured != running) {
        lbwarn("Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x\n", configured->address, running->address);
        lbwarn("(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)\n");
    }
    //lbinfo("Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype, running->address);
    peot->pclient = esp_http_client_init(&peot->cfg);
    lbcheck_pointer(peot->pclient, NULL, "peot->pclient:%p = esp_http_client_init(&peot->cfg)\n", peot->pclient);
    //lbinfo("peot->pclient:%p = esp_http_client_init(&peot->cfg)\n", peot->pclient);
    err = esp_http_client_open(peot->pclient, 0);
    if (err != ESP_OK) {
        lberror("Failed to open HTTP connection: %s", esp_err_to_name(err));
        //lberror("Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(peot->pclient);
        task_fatal_error();
    }
    esp_http_client_fetch_headers(peot->pclient);
    peot->msg_type = e_aiot_http_msg_on_response_body_progress;
    peot->hdp.ltotal_bytes = esp_http_client_get_content_length(peot->pclient);
    lbinfo("peot->hdp.ltotal_bytes:%d = esp_http_client_get_content_length(), spend time:%lu\n", peot->hdp.ltotal_bytes, lbget_sys_time() - begin_time);
    update_partition = esp_ota_get_next_update_partition(NULL);
    //lbinfo("Writing to partition subtype %d at offset 0x%x", update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);
    int binary_file_length = 0;
    // deal with all receive packet
    bool image_header_was_checked = false;
    while(lbthread_is_alive(ptc))
    {
        int data_read = esp_http_client_read(peot->pclient, ota_write_data, BUFFSIZE);
        //lbinfo("data_read:%d = esp_http_client_read(peot->pclient:%p, ota_write_data, BUFFSIZE:%d)\n", data_read, peot->pclient, BUFFSIZE);
        if (data_read < 0) {
            lberror("Error: SSL data read error");
            http_cleanup(peot->pclient);
            task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    lbinfo("New firmware version: %s", new_app_info.version);

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                        lbinfo("Running firmware version: %s", running_app_info.version);
                    }

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        lbinfo("Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
                            lbwarn("New version is the same as invalid version.");
                            lbwarn("Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            lbwarn("The firmware has been rolled back to the previous version.");
                            http_cleanup(peot->pclient);
                            infinite_loop();
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECKptc
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        lbwarn("Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(peot->pclient);
                        infinite_loop();
                    }
#endif

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
                    if (err != ESP_OK) {
                        lberror("esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(peot->pclient);
                        task_fatal_error();
                    }
                    lbinfo("esp_ota_begin succeeded");
                } else {
                    lberror("received package is not fit len");
                    http_cleanup(peot->pclient);
                    task_fatal_error();
                }
            }
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            //lbinfo("err:%d = esp_ota_write\n", err);
            if (err != ESP_OK) {
                http_cleanup(peot->pclient);
                task_fatal_error();
            }

            peot->hdp.ldownload_bytes += data_read;
            peot->hdp.lspend_time_ms    = lbget_sys_time() - begin_time;
            
            if(lbget_sys_time() - last_update_time >= 1000)
            {
                peot->hdp.ldownload_speed = (peot->hdp.ldownload_bytes - last_update_bytes) * 8 * 1000 / (lbget_sys_time() - last_update_time);
                last_update_bytes = peot->hdp.ldownload_bytes;
                last_update_time = lbget_sys_time();
            }

            if(peot->hdp.ltotal_bytes)
            {
                peot->percent = peot->hdp.ldownload_bytes * 100 / peot->hdp.ltotal_bytes;
            }
            //lbinfo("ldownload_speed:%d, ldownload_bytes:%d, ltotal_bytes:%d, lspend_time_ms:%d, percent:%d\n", peot->hdp.ldownload_speed, peot->hdp.ldownload_bytes, peot->hdp.ltotal_bytes, peot->hdp.lspend_time_ms, peot->percent);
            binary_file_length += data_read;
            //lbinfo("Written image length %d", binary_file_length);
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                lberror("Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(peot->pclient) == true) {
                lbinfo("Connection closed");
                break;
            }
        }
    }
    lbinfo("Total Write binary data length: %d", binary_file_length);
    if (esp_http_client_is_complete_data_received(peot->pclient) != true) {
        lberror("Error in receiving complete file");
        http_cleanup(peot->pclient);
        task_fatal_error();
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            lberror("Image validation failed, image is corrupted");
        }
        lberror("esp_ota_end failed (%s)!", esp_err_to_name(err));
        http_cleanup(peot->pclient);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        lberror("esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(peot->pclient);
        task_fatal_error();
    }
    lbinfo("Prepare to restart system!");
    esp_restart();
    return NULL;
}
/*e_aiot_http_msg_on_connect_complete             = 0,    // http connect complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_header_complete      = 1,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_request_complete             = 2,    // request send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_header_complete     = 3,    // response send complete, wparam:0, lparam:0
    e_aiot_http_msg_on_response_body_progress       = 4,    // http resonse body receive progress, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_response_complete            = 5,    // http download complete, wparam:http_download_info ptr, lparam:http download percent
    e_aiot_http_msg_on_connect_error                = 20,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_request_error                = 21,   // http download error, wparam: error message, can be null, lparam: error code
    e_aiot_http_msg_on_response_error               = 22    // http download error, wparam: error message, can be null, lparam: error code
*/

/*int ota_event_notify(esp_ota_task* peot, int msg_id, long wapram, long lparam)
{
    lbcheck_pointer(peot, -1, "Invalid parameter peot:%p\n", peot);
    switch(msg_id)
    {
        case e_aiot_http_msg_on_connect_complete:
    }
    peot->
}*/

esp_err_t esp_ota_event_handle(esp_http_client_event_t *evt)
{
    esp_ota_task* peot = NULL;
    lbcheck_pointer(evt, -1, "Invalid parameter evt:%p\n", evt);
    lbcheck_pointer(evt->user_data, -1, "Invalid owner evt->user_data:%p\n", evt->user_data);
    peot = (esp_ota_task*)evt->user_data;
    //lbinfo("%s evt->event_id:%d\n", __FUNCTION__, evt->event_id);
    switch(evt->event_id)
    {
        case HTTP_EVENT_ON_CONNECTED:
        {
            lbinfo("on http connect!\n");
            peot->msg_type = e_aiot_http_msg_on_connect_complete;
            break;
        }
        case HTTP_EVENT_HEADERS_SENT:
        {
            lbinfo("HTTP_EVENT_HEADERS_SENT!\n");
            peot->msg_type = e_aiot_http_msg_on_request_header_complete;
            break;
        }
        case HTTP_EVENT_ON_HEADER:
        {
            lbinfo("HTTP_EVENT_ON_HEADER!\n");
            break;
        }
        case HTTP_EVENT_ON_DATA:
        {
            //lbinfo("HTTP_EVENT_ON_DATA!\n");
            peot->msg_type = e_aiot_http_msg_on_response_body_progress;
            break;
        }
        case HTTP_EVENT_ON_FINISH:
        {
            lbinfo("HTTP_EVENT_ON_FINISH!\n");
            peot->msg_type = e_aiot_http_msg_on_response_complete;
            break;
        }
        case HTTP_EVENT_DISCONNECTED:
        {
            lbinfo("HTTP_EVENT_DISCONNECTED!\n");
            /*char resp[1024];
            int resp_len = 1024;
            memset(resp, 0, 1024);
            
            resp_len = esp_http_client_read_response(evt->client, evt->user_data, 1024);
            lbinfo("resp_len:%s = esp_http_client_read_response(evt->client, resp:%s, 1024)\n", resp_len, evt->user_data);*/
            break;
        }
        default:
        {
            lberror("Invalid event id:%d\n");
            peot->msg_type = e_aiot_http_msg_on_connect_error;
            break;
        }
    }
    return 0;
}

void* esp_http_create_ota_task(const char* purl)
{
    esp_ota_task* peot = (esp_ota_task*)lbmalloc(sizeof(esp_ota_task));
    lbinfo("%s(purl:%s)\n", __FUNCTION__, purl);
    memset(peot, 0, sizeof(esp_ota_task));
    lbstrcp(peot->purl, purl);
    peot->cfg.url = peot->purl;
    peot->cfg.timeout_ms = CONFIG_SUN_DEFAULT_RECV_TIMEOUT;
    peot->cfg.keep_alive_enable = true;
    peot->cfg.event_handler = esp_ota_event_handle,
    peot->cfg.user_data = peot,
    esp_init();
    lbinfo("%s end, peot:%p\n", __FUNCTION__, peot);
    return peot;
}

int esp_http_ota_start_task(void* task_id)
{
    int ret = -1;
    esp_ota_task* peot = (esp_ota_task*)task_id;
    lbinfo("%s(task_id:%p)\n", __FUNCTION__, task_id);
    lbcheck_pointer(peot, -1, "Invalid parameter, peot:%p\n", peot);
    //peot->pclient = esp_http_client_init(&peot->cfg);
    //lbcheck_pointer(peot->pclient, -1, "peot->pclient:%p = esp_http_client_init(&peot->cfg)\n", peot->pclient);
    //lbinfo("peot->pclient:%p = esp_http_client_init(&peot->cfg)\n", peot->pclient);
    peot->pota_thread = lbthread_open("ota thread", task_id, ota_thread_proc);
    lbcheck_pointer(peot->pota_thread, -1, "peot->pota_thread:%p = lbthread_open(ota thread, task_id, ota_thread_proc)\n", peot->pota_thread);
    ret = lbthread_start(peot->pota_thread);
    lbinfo("%s ret:%d = lbthread_start(peot->pota_thread:%p)\n", __FUNCTION__, ret, peot->pota_thread);
    return ret;
}
int esp_http_ota_get_progress(void* task_id, e_aiot_http_msg_type* msg_type, http_download_info* phdp, int* percent)
{
    esp_ota_task* peot = (esp_ota_task*)task_id;
    lbinfo("%s(task_id:%p)\n", __FUNCTION__, task_id);
    lbcheck_pointer(peot, -1, "Invalid parameter, peot:%p\n", peot);

    if(msg_type)
    {
        *msg_type = peot->msg_type;
    }

    if(phdp)
    {
        *phdp = peot->hdp;
    }
    if(percent)
    {
        *percent = peot->percent;
    }

    return 0;
}
int esp_http_ota_stop_task(void* task_id)
{
    esp_ota_task* peot = (esp_ota_task*)task_id;
    lbinfo("%s(task_id:%p)\n", __FUNCTION__, task_id);
    lbcheck_pointer(peot, -1, "Invalid parameter, peot:%p\n", peot);
    lbthread_stop(peot->pota_thread);
    lbinfo("lbthread_stop(peot->pota_thread)\n");
    lbthread_close(&peot->pota_thread);
    lbinfo("lbthread_close(&peot->pota_thread)\n");
    http_cleanup(peot->pclient);
    lbinfo("http_cleanup(peot->pclient)\n");
    lbfree(peot->purl);
    lbinfo("lbfree(peot->purl)\n");
    lbfree(peot);
    lbinfo("esp_http_ota_stop_task end\n");
    return 0;
}