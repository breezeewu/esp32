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
#include <sys/time.h>
#include "cJSON.h"
//#include "bool.h"
#define lbinfo printf
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

#define CLIENT_SN "P040433000201210701100010"
#define PRODUCT_ID "95f225e106c0447787a0882c61345cf4"
#define CLIENT_SECRET "a6bb6211db7c116afa8623e9e97e3a1e"
#define CERTIFY_PATH "mqtt_ca.crt"

#define PUB_TOPIC "/shadow/95f225e106c0447787a0882c61345cf4/P040433000201210701100010/update"
#define SUB_TOPIC "/shadow/95f225e106c0447787a0882c61345cf4/P040433000201210701100010/get"

#define PUB_BODY "{\"method\":6,\"flownum\":\"4567898\",\"timestamp\":\"20211010101020\",\"state\": {\"onLine\": 1}}"
#define STATE_BODY "{\"onLine\": 1}"
#define CMD_POWER	"{\"power\": 1}"
#define CMD_MODE	"{\"mode\": 1}"

//#define ENABLE_OTA
char* req_opt[] =
{
	"get_map",
	"get_path",
	"get_both"
};

char* dir_ctl_opt[] =
{
	"forward",
	"backward",
	"turn_left",
	"turn_right",
	"stop"
};

char* suction_opt[] =
{
	"strong",
	"normal",
	"quiet",
	"gentle"
};

char* collect_mode_opt[] = 
{
	"small",
	"middle",
	"large"
};

char* cistern_opt[] =
{
	"small",
	"middle",
	"large"
};

char* language_opt[] = 
{
	"chinese_simplified",
	"chinese_traditional",
	"english",
	"german",
	"french",
	"russian",
	"spanish",
	"korean",
	"latin",
	"portuguese",
	"japanese",	
	"italian"
};
struct mqtt_resp_info
{
	void* handle;
	char* payload;
	int	  payload_len;
};

struct mqtt_resp_info* mqtt_create_response_info(void* handle, char* payload, int payload_len)
{
	struct mqtt_resp_info* pmri = (struct mqtt_resp_info*)malloc(sizeof(struct mqtt_resp_info));
	pmri->handle = handle;
	pmri->payload = (char*)malloc(payload_len + 1);
	memcpy(pmri->payload, payload, payload_len);
	pmri->payload[payload_len] = 0;
	pmri->payload_len = payload_len;

	return pmri;
}

void mqtt_close_response_info(struct mqtt_resp_info** ppmri)
{
	if(ppmri && *ppmri)
	{
		struct mqtt_resp_info* pmri = *ppmri;
		free(pmri->payload);
		free(pmri);
		*ppmri = pmri = NULL;
	}
}

int mqtt_send_command_response_int(void* handle, const char* pkey, int val)
{
	int ret = 0;
	char* state = (char*)malloc(100);
	//lbinfo("state:%p = (char*)malloc(100)\n", state);
	char* result = (char*)malloc(100);
	//lbinfo("result:%p = (char*)malloc(100)\n", result);
	sprintf(state, "{\"%s\":%d}", pkey, val);
	//lbinfo("sprintf(state:%s)\n", state);
	sprintf(result, "{\"600\":\"%s\", \"601\":%d}", pkey, val);
	//lbinfo("sprintf(result:%s)\n", result);
	ret = sun_aiot_api_mqtt_pub_update(handle, state, result, e_aiot_mqtt_at_most_once);
	//lbinfo("ret:%d = sun_aiot_api_mqtt_pub_update(handle:%p, state:%s, result:%s)\n", ret, handle, state, result);
	free(state);
	free(result);
	return ret;
}

int mqtt_send_command_response_string(void* handle, const char* pkey, const char* val)
{
	int ret = 0;
	char* state = (char*)malloc(100);
	char* result = (char*)malloc(100);
	sprintf(state, "{\"%s\":\"%s\"}", pkey, val);
	sprintf(result, "\"600\":\"%s\", \"601\":\"%s\"", pkey, val);
	ret = sun_aiot_api_mqtt_pub_update(handle, state, result, e_aiot_mqtt_at_most_once);
	//lbinfo("ret:%d = sun_aiot_api_mqtt_pub_update(handle:%p, state:%s, result:%s)\n", ret, handle, state, result);
	free(state);
	free(result);
	return ret;
}
//on_mqtt_msg_cb(handle:0x3ffc5bc4, userdata:0x0, ptopic:/shadow/95f225e106c0447787a0882c61345cf4/P040433000201210701100010/get, payload:{"flownum":"301_06e24bb0-0844-4d","method":1,"command":{"edge_brush_reset":1},"timestamp":"1628216053444"}, payload_len:106, qos:1)
int handle_mqtt_sub_msg(void* handle, const char* payload, int payload_len)
{
	cJSON* pit = NULL;
	cJSON* parry = NULL;
	int	ret = -1;
	lbinfo("%s(handle:%p, payload:%s, payload_len:%d)\n", __FUNCTION__, handle, payload, payload_len);
	if(NULL == handle || NULL == payload)
	{
		lberror("%s(handle:%p, payload:%p, payload_len:%d)\n", __FUNCTION__, handle, payload, payload_len);
		return -1;
	}
	cJSON* pjr = cJSON_Parse(payload);
	if(NULL == pjr)
	{
		lberror("pjr:%p = cJSON_Parse(payload:%s)\n", pjr, payload);
		return -1;
	}
    parry = cJSON_GetObjectItem(pjr, "command");
	//lbinfo("parry:%p = cJSON_GetObjectItem(pjr:%p, command)\n", parry, pjr);
	if(NULL == parry)
	{
		lberror("parry:%p = cJSON_GetObjectItem(pjr:%p, state)\n", parry, pjr);
		return -1;
	}
	int size = cJSON_GetArraySize(parry);
	//lbinfo("size = cJSON_GetArraySize(parry)\n", size, parry);
	for(int i = 0; i < size; i++)
	{
		pit = cJSON_GetArrayItem(parry, i);
		assert(pit);
		if(cJSON_IsNumber(pit))
		{
			lbinfo("key:%s, nval:%d\n", pit->string, pit->valueint);
			ret = mqtt_send_command_response_int(handle, pit->string, pit->valueint);
			lbinfo("ret:%d = mqtt_send_command_response_int(handle:%p, pit->string:%s, pit->valuedouble:%lf)\n", ret, handle, pit->string, pit->valuedouble);
		}
		else if(cJSON_IsString(pit))
		{
			lbinfo("key:%s, strval:%s\n", pit->string, pit->valuestring);
			ret = mqtt_send_command_response_string(handle, pit->string, pit->valuestring);
			lbinfo("ret:%d = mqtt_send_command_response_string(handle:%p, pit->string:%s, pit->valuestring:%s)\n", ret, handle, pit->string, pit->valuestring);
		}
	}
	return ret;
}

void handle_mqtt_sub_msg_proc(void* owner)
{
	struct mqtt_resp_info* pmri = (struct mqtt_resp_info*)owner;
	lbinfo("handle_mqtt_sub_msg_proc(pmri:%p)\n", pmri);
	if(pmri)
	{
		handle_mqtt_sub_msg(pmri->handle, pmri->payload, pmri->payload_len);
		mqtt_close_response_info(&pmri);
	}
}
int mqtt_report_str_property(void* handle, const char* pcmd, const char* pval)
{
	char cmd[100];
	sprintf(cmd, "{\"%s\": \"%s\"}", pcmd, pval);
	int ret = sun_aiot_api_mqtt_pub_property(handle, cmd, e_aiot_mqtt_at_most_once);
	if(ret != 0)
	{
		printf("ret:%d = sun_aiot_api_mqtt_pub_property(handle, cmd:%s, e_aiot_mqtt_at_most_once) failed\n", ret, cmd);
	}
	return ret;
}

int mqtt_report_int_property(void* handle, const char* pcmd, int val)
{
	char cmd[100];
	sprintf(cmd, "{\"%s\": %d}", pcmd, val);
	int ret = sun_aiot_api_mqtt_pub_property(handle, cmd, e_aiot_mqtt_at_most_once);
	if(ret != 0)
	{
		printf("ret:%d = sun_aiot_api_mqtt_pub_property(handle, cmd:%s, e_aiot_mqtt_at_most_once) failed\n", ret, cmd);
	}
	return ret;
}

long get_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	long timestamp = tv.tv_sec;
	timestamp = timestamp * 1000 + tv.tv_usec/1000;
    
	return timestamp;
}

int mqtt_send_str_property(void* handle, const char* pword, char** pstr_list, int list_size)
{
	for(int i = 0; i < list_size; i++)
	{
		mqtt_report_str_property(handle, pword, pstr_list[i]);
		//mqtt_report_int_property(handle, "0", i);
	}
	sleep(10);
	return 0;
}

int mqtt_send_int_property(void* handle, const char* pword, int min, int max)
{
	for(int i = min; i <= max; i++)
	{
		mqtt_report_int_property(handle, pword, i);
		//mqtt_report_int_property(handle, "0", i);
		if(max - min > 5)
		{
			break;
		}
	}
	sleep(10);
	return 0;
}


void mqtt_report_test(void* handle)
{
	mqtt_send_int_property(handle, "power", 0, 1);
	mqtt_send_int_property(handle, "mode", 0, 1);
	mqtt_send_str_property(handle, "request", req_opt, sizeof(req_opt)/sizeof(char*));
	mqtt_send_str_property(handle, "direction", dir_ctl_opt, sizeof(dir_ctl_opt)/sizeof(char*));
	mqtt_send_int_property(handle, "suction", suction_opt, sizeof(suction_opt)/sizeof(char*));
	mqtt_send_int_property(handle, "working_on", 0, 1);
	mqtt_send_int_property(handle, "volume", 0, 100);
	mqtt_send_int_property(handle, "edge_brush_reset", 0, 1);
	mqtt_send_int_property(handle, "roll_brush_reset", 0, 1);
	mqtt_send_int_property(handle, "filter_reset", 0, 1);
	mqtt_send_int_property(handle, "seek", 0, 1);
	mqtt_send_int_property(handle, "voice_on", 0, 1);
	mqtt_send_int_property(handle, "pause", 0, 1);
	mqtt_send_int_property(handle, "collection_mode", collect_mode_opt, sizeof(collect_mode_opt)/sizeof(char*));
	mqtt_send_int_property(handle, "recharge_power", 0, 1);
	mqtt_send_int_property(handle, "cistern", cistern_opt, sizeof(cistern_opt)/sizeof(char*));
	mqtt_send_int_property(handle, "dust_collection", 0, 1);
	mqtt_send_int_property(handle, "duster_reset", 0, 1);
	mqtt_send_int_property(handle, "wake_up", 0, 1);
	mqtt_send_int_property(handle, "map_reset", 0, 1);
	mqtt_send_int_property(handle, "not_disturb", 0, 1);
	mqtt_send_int_property(handle, "customize_mode", 0, 1);
	mqtt_send_int_property(handle, "language", language_opt, sizeof(language_opt)/sizeof(char*));
	mqtt_send_int_property(handle, "contiue_clean", 0, 1);
	mqtt_send_int_property(handle, "dust_collection_frequence", 0, 4);
	mqtt_send_int_property(handle, "dust_collections", 0, 1);
	mqtt_send_int_property(handle, "charging", 0, 1);
	mqtt_send_int_property(handle, "work_time", 0, 10000);

	mqtt_send_int_property(handle, "battery", 0, 100);
	mqtt_send_int_property(handle, "carpet_pressurize", 0, 1);
	/*int i = 0;
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "power", i);
		//mqtt_report_int_property(handle, "0", i);
	}
	lbsleep(1);
	for(i = 0; i < 5; i++)
	{
		mqtt_report_int_property(handle, "mode", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < sizeof(req_opt)/sizeof(char*); i++)
	{
		mqtt_report_str_property(handle, "request", req_opt[i]);
		//mqtt_report_int_property(handle, "3", i);
	}
	lbsleep(1);
	for(i = 0; i < sizeof(dir_ctl_opt)/sizeof(char*); i++)
	{
		mqtt_report_str_property(handle, "direction", dir_ctl_opt[i]);
		//mqtt_report_int_property(handle, "3", i);
	}
	lbsleep(1);
	for(i = 0; i < sizeof(suction_opt)/sizeof(char*); i++)
	{
		mqtt_report_str_property(handle, "suction", suction_opt[i]);
		//mqtt_report_int_property(handle, "3", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "working_on", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	mqtt_report_int_property(handle, "volume", get_timestamp()/100);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "edge_brush_reset", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < sizeof(cistern_opt)/sizeof(char*); i++)
	{
		mqtt_report_str_property(handle, "roll_brush_reset", cistern_opt[i]);
		//mqtt_report_int_property(handle, "3", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "filter_reset", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "seek", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "voice_on", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "pause", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < sizeof(collect_mode_opt)/sizeof(char*); i++)
	{
		mqtt_report_str_property(handle, "collection_mode", collect_mode_opt[i]);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);


	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "recharge_power", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);

	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "cleaning_time", i);
		//mqtt_report_int_property(handle, "1", i);
	}
	lbsleep(1);
	for(i = 0; i < 2; i++)
	{
		mqtt_report_int_property(handle, "electricity_remaining", i*100);
		//mqtt_report_int_property(handle, "1", i);
	}*/
}

void mqtt_report_status(void* handle)
{
	char* pbuf = malloc(1024);
	memset(pbuf, 0, 1024);
	snprintf(pbuf, 1024, "{\"%s\": %d", "power", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "mode", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "request", req_opt[2]);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "direction", dir_ctl_opt[2]);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "suction", suction_opt[2]);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "working_on", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "volume", 80);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "edge_brush_reset", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "roll_brush_reset", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "filter_reset", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "seek", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "voice_on", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "pause", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "collection_mode", "small");
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "recharge_power", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "cistern", "small");
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "dust_collection", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "wake_up", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "map_reset", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "not_disturb", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "customize_mode", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": \"%s\"", "language", "chinese_simplified");
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "contiue_clean", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "dust_collection_frequence", 4);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "dust_collections", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "charging", 1);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "work_time", 3600);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "battery", 89);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d", "duster_reset", 0);
	snprintf(pbuf + strlen(pbuf), 1024 - strlen(pbuf), ",\"%s\": %d}", "carpet_pressurize", 0);
	int ret = sun_aiot_api_mqtt_pub_property(handle, pbuf, e_aiot_mqtt_at_most_once);
	free(pbuf);
	if(ret != 0)
	{
		printf("ret:%d = sun_aiot_api_mqtt_pub_property(handle, pbuf:%s, e_aiot_mqtt_at_most_once) failed\n", ret, pbuf);
	}
}
#define CMD_DIR_CTL	"{\"direction_control\": \"forward\"}"
#define CMD_SUCTION "{\"suction\": \"strong\"}"

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
	//int ret = handle_mqtt_sub_msg(handle, (const char*)payload, payload_len);
	//lbinfo("ret:%d = handle_mqtt_sub_msg(handle:%p, payload:%p, payload_len:%d)\n", ret, handle, payload, payload_len);
	//struct mqtt_resp_info* pmri = mqtt_create_response_info(userdata, (char*)payload, payload_len);
	//xTaskCreate(&handle_mqtt_sub_msg_proc, "handle_mqtt_sub", 8192, pmri, tskIDLE_PRIORITY, NULL);
	handle_mqtt_sub_msg(userdata, payload, payload_len);
}

void mqtt_event_handle(void *handler, const char* base, int32_t event_id, void *event_data)
{
	printf("%s %s(handler:%p, base:%s, event_id:%d, event_data:%p)\n", __FILE__, __FUNCTION__, handler, base? "null":base, event_id, event_data);
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
	ret = sun_aiot_api_mqtt_pub_property(handle, STATE_BODY, e_aiot_mqtt_at_most_once);
	//lbtrace("ret:%d = sun_aiot_api_mqtt_pub_property(handle:%p, topic:%p, STATE_BODY:%s, e_mqtt_at_most_once)\n", ret, handle, topic, STATE_BODY);
}
#define WIFI_SSID       "PRD_dev_2.4G"
#define WIFI_PWD        "prd@dev2019#"

int ota_msg_cb(void* owner, long msg_id, long wparam, long lparam)
{
	printf("%s %s(owner:%p, msg_id:%ld, wparam:%ld, lparam:%ld)\n", __FILE__, __FUNCTION__, owner, msg_id, wparam, lparam);
	return 0;
}

/*static void mqtt_event_handler(void *owner, const char* base, int32_t event_id, void *event_data)
{
	printf("%s %s(owner:%p, base:%s, event_id:%d, event_data:%p)\n", __FILE__, __FUNCTION__, owner, base, event_id, event_data);
}*/

int app_main(void)
{
	int ret = -1;
	const char* http_download = "https://live-play-sit.sunvalleycloud.com/ota/BS-20210318_T1-4.0.19T1.bin";
	g_running = 1;
#ifdef ENABLE_OTA
	aiot_ota_info aoi;
	aoi.state_of_charge = 80;
	aoi.region = 20;
	strcpy(aoi.version, "00.00.00");
#endif
	int msg_id = 0, percent = 0;
	http_download_info hdi;
	long speed = 0, download_bytes = 0, toatal_bytes = 0, spend_time_ms = 0, pecent = 0;
	lbtrace("simple_publish_test end");
	//signal(SIGINT,  sig_stop);
	//signal(SIGQUIT, sig_stop);
	//signal(SIGTERM, sig_stop);

	lbconnect_wifi(WIFI_SSID, WIFI_PWD);
	sun_aiot_api_init_log(2, 1, "log");
	printf("before sun_aiot_api_init\n");
	void* handle = sun_aiot_api_init(PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
	printf("handle:%p = sun_aiot_api_init(CLIENT_ID:%s, CLIENT_SECRET:%s, CLIENT_SN:%s, CERTIFY_PATH:%s, e_aiot_env_default:%d)\n", handle, PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
	if(NULL == handle)
	{
		lberror("handle:%p = sun_aiot_api_init(CLIENT_ID:%s, CLIENT_SECRET:%s, CLIENT_SN:%s, CERTIFY_PATH:%s, e_aiot_env_default:%d)\n", handle, PRODUCT_ID, CLIENT_SECRET, CLIENT_SN, CERTIFY_PATH, e_aiot_env_default);
		return -1;
	}

	ret = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL);
	printf("ret:%d = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL)\n", ret);
	if(0 != ret)
	{
		lberror("ret:%d = sun_aiot_api_mqtt_register_event_handle(handle, mqtt_event_handle, NULL) failed\n", ret);
		return ret;
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
	
	sleep(3);
	printf("before sun_aiot_api_mqtt_sub\n");
	ret = sun_aiot_api_mqtt_sub(handle, NULL, e_aiot_mqtt_at_least_once, on_mqtt_msg_cb, handle);
	printf("ret:%d = sun_aiot_api_mqtt_sub(handle:%p, NULL, e_mqtt_at_least_once, on_mqtt_msg_cb:%p, NULL)\n", ret, handle, on_mqtt_msg_cb);
	//lbcheck_return(0 != ret, ret, "ret:%d = sun_mqtt_sub(handle:%p, SUB_TOPIC:%s, e_mqtt_atleast_once, on_mqtt_msg_cb, NULL)\n", ret, handle, SUB_TOPIC);
#ifdef ENABLE_OTA
	mqtt_report_status(handle);
	void* task_id = sun_aiot_api_create_oat_download_task(handle, &aoi);
	printf("task_id:%p = sun_aiot_api_create_oat_download_task\n", task_id);
	if(task_id)
	{
		sun_aiot_api_set_download_callback(task_id, ota_msg_cb, handle);
		sun_aiot_api_http_download_start(task_id);
	}
#else
	mqtt_report_status(handle);
#endif
	sleep(10);
	while(g_running)
	{
		//simple_publish_test(handle, NULL);
		
#ifdef ENABLE_OTA
		sun_aiot_api_http_download_get_progress(task_id, &msg_id, &hdi, &pecent);
		printf("%s download proc, msg_id:%d, percent:%ld, download bytes:%ld, ldownload_speed:%ld, lspend_time_ms:%ld, ltotal_bytes:%ld\n", 
		__FILE__, msg_id, pecent, hdi.ldownload_bytes, hdi.ldownload_speed, hdi.lspend_time_ms, hdi.ltotal_bytes);

		if(pecent >= 100)
		{
			printf("download complete!\n");
			//break;
		}
#else
		//mqtt_report_test(handle);
		//simple_publish_test(handle, NULL);
#endif
	sleep(1);
	};

	sun_aiot_api_disconnect(handle);

	sun_aiot_api_deinit(&handle);

	return 0;
}
