/* Copyright c 2020, sunvalley.com.cn All rights reserved
 */


#include <signal.h>
#include <stdio.h>
//#include "cjson.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "sun_sdk_inc/sun_aiot_api.h"
#include "mywifi.h"
//#include "bool.h"
#define lbtrace printf
#define lberror printf

int g_running = 0;
/*#define CLIENT_ID "8591877132584364ac85bfbf4a2e7270"
//#define PRODUCT_ID "1354726310844309506"
#define CLIENT_SN "P000222000301210128100001"
#define CLIENT_SECRET "c49c06e7ecff4df392953693c54d39e3"
#define CERTIFY_PATH "ca.crt"
#define PUB_TOPIC "/shadow/1354726310844309506/P000222000301210128100001/update"
#define SUB_TOPIC "/shadow/1354726310844309506/P000222000301210128100001/get"

//#define PUB_MSG_BODY "{\r\n\"method\": 6,\r\n\"flownum\": \"123\",\r\n\"timestamp\": \"15916474144400\",\r\n\"state\": {\r\n\"0\": 1\r\n}\r\n}"
#define PUB_MSG_BODY "{\r\n\"method\": 6,\r\n\"flownum\": \"172049-20210204-10000\",\r\n\"timestamp\": \"15916474144400\",\r\n\"state\": {\r\n\"1\": 1\r\n}\r\n}"*/
#define DATA_BODY "{\r\n\"0\":1\r\n}\r\n}"
#define STATE_BODY "{\r\n\"2\":1\r\n}"
#define RESULT_BODY "{\r\n\"600\":234\r\n}"
#define COMMAND_BODY "{\"1\":{\"tiggertype\":104,\"pushflow\":1,\"devicesn\":\"P020101000201201012400001\",\"timeout\":180,\"timestamp\":\"1611899676331\"}}"
#define KEEPALIVE_TIME      20

#define CLIENT_SN "B040401000201201029200021"
#define PRODUCT_ID "e390c3dd143642f986bcb81cafbffa99"
#define CLIENT_SECRET "e00c20288f6c42bd8db7b4241d0b8396"
#define CERTIFY_PATH "mqtt_ca.crt"

#define PUB_TOPIC "/shadow/e390c3dd143642f986bcb81cafbffa99/B040401000201201029200021/update"
#define SUB_TOPIC "/shadow/e390c3dd143642f986bcb81cafbffa99/B040401000201201029200021/get"

#define PUB_BODY "{\"method\":6,\"flownum\":\"4567898\",\"timestamp\":\"20211010101020\",\"state\": {\"onLine\": 1}}"
#define STATE_BODY "{\"onLine\": 1}"

void sig_stop(int signo)
{
	printf("sig_stop: signo = %d\n", signo);
	g_running = 0;
	return;
}

void on_http_download(void* task_id, void* userdata, long speed, long download_bytes, long toatal_bytes, long spend_time_ms)
{
	long percent = 0;
	if(toatal_bytes > 0)
	{
		percent = download_bytes * 100 / toatal_bytes;
	}

	printf("on_http_download(task_id:%p, userdata:%p, speed:%ld, download_bytes:%ld, toatal_bytes:%ld, spend_time_ms:%ld), percent:%ld\n", task_id, userdata, speed, download_bytes, toatal_bytes, spend_time_ms, percent);
}

void on_http_cb(void* task_id, void* userdata, long msg_id, long wparam, long lparam)
{
	switch(msg_id)
	{
		case e_aiot_http_msg_on_connect_complete:
		{
			printf("http connect success!\n");
			break;
		}
		case e_aiot_http_msg_on_request_header_complete:
		{
			printf("http request header complete!\n");
			break;
		}
		case e_aiot_http_msg_on_request_complete:
		{
			printf("http send request success!\n");
			break;
		}
		case e_aiot_http_msg_on_response_header_complete:
		{
			printf("http recv response success!\n");
			break;
		}
		case e_aiot_http_msg_on_response_body_progress:
		{
			http_download_info* phdp = (http_download_info*)wparam;
			printf("http download progress, speed:%ld, download_bytes:%ld, total_bytes:%ld, spend_time:%ld, percent:%ld\n", phdp->ldownload_speed, phdp->ldownload_bytes, phdp->ltotal_bytes, phdp->lspend_time_ms, lparam);
			break;
		}
		case e_aiot_http_msg_on_response_complete:
		{
			printf("http send response success!\n");
			break;
		}
		case e_aiot_http_msg_on_connect_error:
		case e_aiot_http_msg_on_request_error:
		case e_aiot_http_msg_on_response_error:
		{
			printf("http request error, msg_id:%ld, msg:%s, errno:%ld\n", msg_id, (const char*)wparam, lparam);
			break;
		}
		default:
		{
			printf("unknown msg id:%ld\n", msg_id);
			break;
		}
	}
}

void on_mqtt_msg_cb(void* handle, void * userdata, const char* ptopic, void* payload, int payload_len, e_aiot_mqtt_qos qos)
{
	printf("on_mqtt_msg_cb(handle:%p, userdata:%p, ptopic:%s, payload:%s, payload_len:%d, qos:%d)\n", handle, userdata, ptopic, (char*)payload, payload_len, qos);
}

void mqtt_event_handle(void *handler_args, const char* base, int32_t event_id, void *event_data)
{
	printf("%s(handler_args:%p, base:%s, event_id:%d, event_data:%p)\n", __FUNCTION__, handler_args, base, event_id, event_data);
}

void simple_publish_test(void* handle, const char* topic)
{
	int ret = 0; //sun_aiot_api_mqtt_pub(handle, PUB_TOPIC, PUB_BODY, strlen(PUB_BODY), e_mqtt_at_most_once);
	//printf("ret:%d = sun_aiot_api_mqtt_pub(handle, PUB_TOPIC, PUB_BODY, strlen(PUB_BODY), e_mqtt_at_most_once)\n", ret);
	//ret = sun_aiot_api_mqtt_pub_property(handle, STATE_BODY, e_mqtt_at_most_once);
	//printf("ret:%d = sun_aiot_api_mqtt_pub_property(handle, STATE_BODY, e_mqtt_at_most_once)\n", ret);
	/*ret = sun_aiot_api_mqtt_pub_ctrl(handle, COMMAND_BODY, e_mqtt_at_most_once);
	//lbcheck_return(0 != ret, , "ret:%d = sun_mqtt_pub_ctrl(id:%ld, COMMAND_BODY:%s, e_mqtt_at_most_once)\n", ret, id, COMMAND_BODY);
	lbtrace("ret:%d = sun_aiot_api_mqtt_pub_ctrl(handle:%p, topic:%p, COMMAND_BODY:%s, e_mqtt_at_most_once)\n", ret, handle, topic, COMMAND_BODY);
	ret = sun_aiot_api_mqtt_pub_update(handle, STATE_BODY, RESULT_BODY, e_mqtt_at_most_once);
	lbtrace("ret:%d = sun_aiot_api_mqtt_pub_update(handle:%p, topic:%p, STATE_BODY:%s, RESULT_BODY:%s, e_mqtt_at_most_once)\n", ret, handle, topic, STATE_BODY, RESULT_BODY);
	ret = sun_aiot_api_mqtt_pub_event(handle, "alarm", DATA_BODY, e_mqtt_at_most_once);
	lbtrace("ret:%d = sun_aiot_api_mqtt_pub_event(handle:%p, topic:%p, alarm, DATA_BODY:%s, e_mqtt_at_most_once)\n", ret, handle, topic, DATA_BODY);
	*/
	ret = sun_aiot_api_mqtt_pub_property(handle, STATE_BODY, e_aiot_mqtt_atmost_once);
	//lbtrace("ret:%d = sun_aiot_api_mqtt_pub_property(handle:%p, topic:%p, STATE_BODY:%s, e_mqtt_at_most_once)\n", ret, handle, topic, STATE_BODY);
}
#define WIFI_SSID       "Tenda"
#define WIFI_PWD        "1234456789"
int app_main(void)
{
	int ret = -1;
	const char* http_download = "https://live-play-sit.sunvalleycloud.com/ota/BS-20210318_T1-4.0.19T1.bin";
	g_running = 1;
	long speed = 0, download_bytes = 0, toatal_bytes = 0, spend_time_ms = 0, pecent = 0;
	lbtrace("simple_publish_test end");
	//signal(SIGINT,  sig_stop);
	//signal(SIGQUIT, sig_stop);
	//signal(SIGTERM, sig_stop);

	lbconnect_wifi(WIFI_SSID, WIFI_PWD);
	sun_aiot_api_init_log(2, 1, "log");
	printf("beforepecent sun_aiot_api_init\n");
	void* handle = sun_aiot_api_init(PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
	printf("handle:%p = sun_aiot_api_init(CLIENT_ID:%s, CLIENT_SECRET:%s, CLIENT_SN:%s, CERTIFY_PATH:%s, e_aiot_env_default:%d)\n", handle, PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
	if(NULL == handle)
	{
		lberror("handle:%p = sun_aiot_api_init(CLIENT_ID:%s, CLIENT_SECRET:%s, CLIENT_SN:%s, CERTIFY_PATH:%s, e_aiot_env_default:%d)\n", handle, PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
		return -1;
	}
	do
	{
		ret = sun_aiot_api_connect(handle);
		printf("ret:%d = sun_aiot_api_connect(handle)\n", ret);
		if(ret != 0)
		{
			sleep(3);
		}
	}while(ret != 0);
	//ret = sun_aiot_api_connect(handle);
	//printf("ret:%d = sun_aiot_api_connect(handle)\n", ret);
	if(ret != 0)
	{
		lberror("ret:%d = sun_aiot_api_connect(handle:%p) failed\n", ret, handle);
		return ret;
	}
	ret = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL);
	printf("ret:%d = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL)\n", ret);
	if(0 != ret)
	{
		lberror("ret:%d = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL) failed\n", ret);
		return ret;
	}
	sleep(3);
	printf("before sun_aiot_api_mqtt_sub\n");
	ret = sun_aiot_api_mqtt_sub(handle, NULL, e_aiot_mqtt_atleast_once, on_mqtt_msg_cb, NULL);
	printf("ret:%d = sun_aiot_api_mqtt_sub(handle:%p, NULL, e_mqtt_at_least_once, on_mqtt_msg_cb:%p, NULL)\n", ret, handle, on_mqtt_msg_cb);
	//lbcheck_return(0 != ret, ret, "ret:%d = sun_mqtt_sub(handle:%p, SUB_TOPIC:%s, e_mqtt_atleast_once, on_mqtt_msg_cb, NULL)\n", ret, handle, SUB_TOPIC);

	while(g_running)
	{
		simple_publish_test(handle, NULL);
		sleep(10);
	};

	sun_aiot_api_disconnect(handle);

	sun_aiot_api_deinit(&handle);

	return 0;
}