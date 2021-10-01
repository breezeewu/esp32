/****************************************************************************************************************
 * filename     sun_aiot_mqtt_client.h
 * describe     Sunvalley aiot platform mqtt sdk api implement
 * author       Created by dawson on 2021/01/28
 * Copyright    ©2007 - 2029 Sunvally.com.cn All Rights Reserved.
 ***************************************************************************************************************/
#include <stdio.h>
#include <string.h>
//#include "openssl/ssh.h"
#include "lbmacro.h"
#include "lblist.h"
//#include "lbmacros.h"
#include "cJSON.h"
#include "sun_aiot_api.h"
#include "sun_mqtt_api.h"
#include <sys/time.h>
#include <time.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_tls.h"
//#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "mbedtls/base64.h"

#ifndef SSL_VERIFY_PEER
#define SSL_VERIFY_PEER		0x01
#endif

//#define lbstrcp(pdst, psrc) if(psrc) { pdst = (char*)lbmalloc(strlen(psrc) + 1); memcpy(pdst, psrc, strlen(psrc) + 1);}
//#define lbcheck_resulst(res, ret) if(!(res)) { lberror("%s check result failed, ret:%d\n", __func__, (int)ret); return ret;}


#define SDK_VERSION_MAYJOR	0
#define SDK_VERSION_MINOR	0
#define SDK_VERSION_MACRO	2
#define SDK_VERSION_TINY	0
const char cert_pem[] = "-----BEGIN CERTIFICATE-----\r\n\
MIIDYTCCAkmgAwIBAgIJANX7XrvGbIvZMA0GCSqGSIb3DQEBCwUAMEYxDTALBgNV\r\n\
BAMMBENBMDExKDAmBgNVBAoMH1N1bnZhbGxleXRlayBJbnRlcm5hdGlvbmFsIElu\r\n\
Yy4xCzAJBgNVBAYTAlVTMCAXDTIwMDcwODA2NDc1MFoYDzIwNzAwNjI2MDY0NzUw\r\n\
WjBGMQ0wCwYDVQQDDARDQTAxMSgwJgYDVQQKDB9TdW52YWxsZXl0ZWsgSW50ZXJu\r\n\
YXRpb25hbCBJbmMuMQswCQYDVQQGEwJVUzCCASIwDQYJKoZIhvcNAQEBBQADggEP\r\n\
ADCCAQoCggEBAKvbmr3OVAaI6hpgaD1PeYOYeuXlVBj9kgHlCxTlSn/B/P0rKjc0\r\n\
u0bATGFSW8ROFWP2fxipcnRQYca8v13/ASeyWwUP4S16R/zwgaMUIQ04+vsxsM/1\r\n\
V6ol+wt/CyhwaGEaYrU1KhheMtpElr3LLf1wourBPRp9gdXQoMBubRe7gyL42WRs\r\n\
IOBbcW77qVIlbEEDqh69aD0kIHn17QqArtZS14AZSZ6ySZ6M6QyHSDaYGK/3Ymrb\r\n\
jvp7sOsiCghLfVMHWYPERtHAz1Pu6fH28FZS7HlWtiR8FaFMp55nGK3bFh/yIdKX\r\n\
Epw1TwFTjUVXO5o1/qnvMlMkSJBIxV9N58sCAwEAAaNQME4wHQYDVR0OBBYEFBaP\r\n\
nXt2LIOrERuHsS695SjzrxzYMB8GA1UdIwQYMBaAFBaPnXt2LIOrERuHsS695Sjz\r\n\
rxzYMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAGJB/7Z+KLz8TsOY\r\n\
WuDhig/ZjAEZwqH3grHWIfwYUtoeEQI5iP4tIab7eCEqPuBrnlOf1Kll/9yVjK9L\r\n\
RL/Ay0s+If0ZoessmQPlLaWsuUvTWHNXNARoLwjXK5DWKoAnEXav66kQVcFAr5Fz\r\n\
BV0/3oYwRSeyBiECRYBnujTt6ssSFV/vUrp6sh3XEl4r9YILwjIA2OyTGL7E61aP\r\n\
/c0Ek+tZmQlWBgfmtUkMDyhAxxWIG8+vqyqq71hdkggVCtgI1G2jNYXKdWxnVlXp\r\n\
CPFzZH2DNq2p80rL4DcwMt7caMzPz1/mNoy5cOAe4sS4M7S/EyJDcXSiPVndmjGZ\r\n\
2NX7PTs=\r\n-----END CERTIFICATE-----";

// subcribe info struct
typedef struct
{
	// message id
	int			msg_id;

	// subcribe topic
	char* 		ptopic;
	char*		payload;
	int			npayload_len;
	int			npayload_size;
	// subcribe message qos
	e_aiot_mqtt_qos	qos;

	// user data
	void*		puser_data;

	// subcribe message callback
	on_mqtt_subscribe_cb pon_sub_msg;
} sub_info;

typedef void (*on_connect_cb)(long, void *, int);
typedef void (*on_disconnect_cb)(long, void *, int);

// sunvalley mqtt handle struct
typedef struct sun_mqtt_handle
{
	esp_mqtt_client_handle_t mqtt_client;
	esp_mqtt_client_config_t cfg;
	//struct mosquitto* pmosq;

	char* pclient_id;
	char* puser_name;
	char* ppassword;
	char* psub_topic;
	char* ppub_topic;
	char* pcert_pem;
	char* purl;

	int keep_live_time;
	int clean_session;
	int subscribe_qos;

	int flow_num;

	// deviece app callback func
	esp_event_handler_t	pevent_cb;
	void*				pcb_obj;

	int32_t				event_id;
	on_connect_cb		pon_connect;
	on_disconnect_cb	pon_disconnect;
	

	struct lblist_ctx*	sub_list;
} mqtt_handle;

void dump_event_data(esp_mqtt_event_handle_t pevent_data)
{
	lbinfo("dump_event_data begin, pevent_data:%p\n", pevent_data);
	if(pevent_data)
	{
		lbinfo("dump event data\n");
		lbinfo("pevent_data->data_len:%d, pevent_data->event_id:%d\n", pevent_data->data_len, pevent_data->event_id);
		if(pevent_data->data)
		{
			lbinfo("pevent_data->data:%s\n", pevent_data->data);
		}
		
		//lbinfo("pevent_data->data_len:%d\n", pevent_data->data_len);
		lbinfo("pevent_data->total_data_len:%d\n", pevent_data->total_data_len);
		lbinfo("pevent_data->current_data_offset:%d\n", pevent_data->current_data_offset);
		if(pevent_data->topic)
		{
			lbinfo("pevent_data->topic:%s\n", pevent_data->topic);
			lbmemory(3, pevent_data->topic, pevent_data->topic_len, "topic:");
			lbinfo("pevent_data->topic_len:%d\n", pevent_data->topic_len);
		}
		
		lbinfo("pevent_data->msg_id:%d\n", pevent_data->msg_id);
		lbinfo("pevent_data->session_present:%d\n", pevent_data->session_present);
		lbinfo("dump event data end, pevent_data->error_handle:%p\n", pevent_data->error_handle);
		if(pevent_data->error_handle)
		{
			lbinfo("dump error\n");
			esp_mqtt_error_codes_t* err = pevent_data->error_handle;
			lbinfo("esp_tls_last_esp_err:%d\n", err->esp_tls_last_esp_err);
			lbinfo("esp_tls_stack_err:%d\n", err->esp_tls_stack_err);
			lbinfo("esp_tls_cert_verify_flags:%d\n", err->esp_tls_cert_verify_flags);
			lbinfo("esp_tls_cert_verify_flags:%d\n", err->esp_tls_cert_verify_flags);
			lbinfo("error_type:%d\n", err->error_type);
			lbinfo("connect_return_code:%d\n", err->connect_return_code);
			lbinfo("esp_transport_sock_errno:%d\n", err->esp_transport_sock_errno);
		}
	}
	else
	{
		lbtrace("pevent_data:%p\n", pevent_data);
	}
}

static void mqtt_event_handler(void *owner, esp_event_base_t base, int32_t event_id, void *event_data) {
	int ret = 0;
    lbtrace("Event dispatched from event loop base=%s, event_id=%d, event_data:%p\n", base, event_id, event_data);
	esp_mqtt_event_handle_t event = event_data;
	mqtt_handle* pmh = (mqtt_handle*)owner;
	if(NULL == pmh)
	{
		lberror("Invalid parameter, pmh:%p\n", pmh);
		return ;
	}

	switch(event_id)
	{
		case MQTT_EVENT_CONNECTED:
		{
			lbinfo("mqtt event connected, pmh->sub_list:%p\n", pmh->sub_list);
			BEGIN_ENUM_LIST(pmh->sub_list, ptmp)
				printf("ptmp:%p\n", ptmp);
				printf("ptmp->pitem:%p\n", ptmp->pitem);
				sub_info* psi = (sub_info*)ptmp->pitem;
				lbinfo("before esp_mqtt_client_subscribe(pmh->mqtt_client:%p, psi->ptopic:%s, psi->qos:%d)\n", pmh->mqtt_client, psi->ptopic, psi->qos);
				ret = esp_mqtt_client_subscribe(pmh->mqtt_client, psi->ptopic, psi->qos);
				lbinfo("on mqtt connect, ret:%d = esp_mqtt_client_subscribe(pmh->mqtt_client:%p, psi->ptopic:%s)\n", ret, pmh->mqtt_client, psi->ptopic);
			END_ENUM_LIST
			break;
		}
		case MQTT_EVENT_DISCONNECTED:
		{
			lbtrace("mqtt event disconnect!\n");
			break;
		}
		case MQTT_EVENT_SUBSCRIBED:
		{
			lbtrace("mqtt event subscribe!\n");
			//dump_event_data(event);
			break;
		}
		case MQTT_EVENT_UNSUBSCRIBED:
		{
			lbtrace("mqtt event unsubscribe\n");
			//dump_event_data(event);
			break;
		}
		case MQTT_EVENT_PUBLISHED:
		{
			lbtrace("mqtt event published\n");
			//dump_event_data(event);
			break;
		}
		case MQTT_EVENT_DATA:
		{
			lbtrace("mqtt event data, event->topic_len:%d, event->data_len:%d\n", event->topic_len, event->data_len);
			//dump_event_data(event);
			BEGIN_ENUM_LIST(pmh->sub_list, ptmp)
				sub_info* psi = (sub_info*)ptmp->pitem;
				//lbinfo("strlen(psi->ptopic):%d == event->topic_len:%d, psi->pon_sub_msg:%p\n", strlen(psi->ptopic), event->topic_len, psi->pon_sub_msg);
				if(strlen(psi->ptopic) == event->topic_len && 0 == memcmp(psi->ptopic, event->topic, event->topic_len))
				{
					if(psi->npayload_size <= event->data_len + 1)
					{
						lbfree(psi->payload);
						psi->npayload_size = (event->data_len + 1) * 2;
						psi->payload = (char*)lbmalloc(psi->npayload_size);
					}

					psi->npayload_len = event->data_len;
					memset(psi->payload, 0, psi->npayload_size);
					memcpy(psi->payload, event->data, psi->npayload_len);
					lbtrace("psi->pon_sub_msg:%p, psi->ptopic:%s, psi->payload:%s\n", psi->pon_sub_msg, psi->ptopic, psi->payload);
					if(psi->pon_sub_msg)
					{
						psi->pon_sub_msg(pmh, psi->puser_data, psi->ptopic, psi->payload, psi->npayload_len, psi->qos);
					}
				}
				/*printf("ptmp:%p\n", ptmp);
				printf("ptmp->pitem:%p\n", ptmp->pitem);
				
				lbinfo("before esp_mqtt_client_subscribe(pmh->mqtt_client:%p, psi->ptopic:%s, psi->qos:%d)\n", pmh->mqtt_client, psi->ptopic, psi->qos);
				ret = esp_mqtt_client_subscribe(pmh->mqtt_client, psi->ptopic, psi->qos);
				lbinfo("on mqtt connect, ret:%d = esp_mqtt_client_subscribe(pmh->mqtt_client:%p, psi->ptopic:%s)\n", ret, pmh->mqtt_client, psi->ptopic);*/
			END_ENUM_LIST
			break;
		}
		case MQTT_EVENT_ERROR:
		{
			lbtrace("mqtt event error\n");
			dump_event_data(event);
			break;
		}
		case MQTT_EVENT_BEFORE_CONNECT:
		{
			lbtrace("mqtt event error before connect\n");
			//dump_event_data(event);
			break;
		}
		default:
		{
			lberror("other event id:%d\n", event_id);
			dump_event_data(event);
			break;
		}
	}

	if(pmh->pevent_cb && (MQTT_EVENT_ANY == pmh->event_id ||  pmh->event_id == event_id))
	{
		lbtrace("%s(), pmh->event_id:%d, event_id:%d", __FUNCTION__, pmh->event_id, event_id);
		return pmh->pevent_cb(pmh->pcb_obj, base, event_id, event_data);
	}
    //mqtt_event_handler_cb(event_data);
}

int add_subscribe(mqtt_handle* pmh, int msg_id, const char* ptopic, e_aiot_mqtt_qos qos, on_mqtt_subscribe_cb on_topic_msg, void* userdata)
{
	assert(pmh);
	if(NULL == pmh->sub_list)
	{
		pmh->sub_list = lblist_context_create(100);
	}
	//lbinfo("%s(pmh:%p)\n", __FUNCTION__, pmh);
	sub_info* psi = (sub_info*)lbmalloc(sizeof(sub_info));
	memset(psi, 0, sizeof(sub_info));
	psi->msg_id = msg_id;
	psi->pon_sub_msg = on_topic_msg;
	psi->qos = qos;
	psi->puser_data = userdata;
	//lbinfo("%s lbstrcp\n", __FUNCTION__);
	lbstrcp(psi->ptopic, ptopic);
	lblist_push(pmh->sub_list, psi);
	//lbinfo("%s end\n", __FUNCTION__);
	return 0;
}

int remove_subscribe(mqtt_handle* pmh, const char* ptopic)
{
	assert(pmh);
	if(pmh->sub_list)
	{
		BEGIN_ENUM_LIST(pmh->sub_list, ptmp)
			sub_info* psi = (sub_info*)ptmp->pitem;
			if(psi && memcmp(psi->ptopic, ptopic, strlen(ptopic)) == 0)
			{
				lblist_remove(pmh->sub_list, ptmp);
				lbfree(psi->ptopic);
				lbfree(psi);
				return 0;
			}
		END_ENUM_LIST
	}

	return -1;
}

void* sun_mqtt_init(const char* pdevice_sn, const char* pproduct_key, const char* pclient_secret, const char* pcrt_path)
{
	lbinfo("%s(pdevice_sn:%s, pproduct_key:%s, pclient_secret:%s, pcrt_path:%s)\n", __FUNCTION__, pdevice_sn, pproduct_key, pclient_secret, pcrt_path);
	mqtt_handle* pmh = NULL;
	if(NULL == pdevice_sn || NULL == pproduct_key || NULL == pclient_secret || NULL == pcrt_path)
	{
		lberror("Invalid parameter, pdevice_sn:%s, pproduct_key:%s, pclient_secret:%s, pcrt_path:%s\n", pdevice_sn, pproduct_key, pclient_secret, pcrt_path);
		return NULL;
	}

	pmh = (mqtt_handle*)lbmalloc(sizeof(mqtt_handle));
	memset(pmh, 0, sizeof(mqtt_handle));

	lbstrcp(pmh->pclient_id, pdevice_sn);
	lbstrcp(pmh->puser_name, pproduct_key);
	lbstrcp(pmh->ppassword, pclient_secret);

	pmh->cfg.client_id = pmh->pclient_id;
	pmh->cfg.username = pmh->puser_name;
	pmh->cfg.password = pmh->ppassword;
	pmh->cfg.keepalive = 10;
	//printf("cert_pem begin\n");
	pmh->cfg.cert_len = 0;
	if(pcrt_path)
	{
		FILE* pfile = fopen(pcrt_path, "rb");
		if(pfile)
		{
			fseek(pfile, 0, SEEK_END);
			long file_size = ftell(pfile);
			pmh->pcert_pem = (char*)lbmalloc(file_size + 1);
			memset(pmh->pcert_pem, 0, file_size + 1);
			fread(pmh->pcert_pem, 1, file_size, pfile);
			fclose(pfile);
			pmh->cfg.cert_pem = pmh->pcert_pem;
		}
	}

	if(NULL == pmh->cfg.cert_pem)
	{
		pmh->cfg.cert_pem = cert_pem;
	}
	//(char*)lbmalloc(pmh->cfg.cert_len + 1);
	//memcpy(pmh->cfg.cert_pem, cert_pem, pmh->cfg.cert_len + 1);
	//printf("cert_pem end, pmh->cfg.cert_len:%d\n", pmh->cfg.cert_len);

	if(pproduct_key && pdevice_sn)
	{
		char topic[256];
		sprintf(topic, "/shadow/%s/%s/update", pproduct_key, pdevice_sn);
		lbstrcp(pmh->ppub_topic, topic);
		sprintf(topic, "/shadow/%s/%s/get", pproduct_key, pdevice_sn);
		lbstrcp(pmh->psub_topic, topic);
	}

	//printf("nvs_flash_init begin\n");
	ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    esp_event_loop_create_default();

	//ESP_ERROR_CHECK(example_connect());
	//pmh->mqtt_client = esp_mqtt_client_init(&pmh->cfg);
	//lbcheck_pointer(pmh->mqtt_client, -1, "mqtt_client:%p = esp_mqtt_client_init(&pmh->cfg) failed", pmh->mqtt_client);
	//printf("sun_mqtt_init end, pmh:%p\n", pmh);
	return pmh;
}

int sun_mqtt_register_event_handle(void* handle, mqtt_event_callback_t mqtt_event_cb, void* powner)
{
	int ret = 0;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, handle:%p\n", handle);
	pmh->pevent_cb = mqtt_event_cb;
	pmh->pcb_obj = powner;
	/*lbcheck_value(NULL == pmh, -1, "Invalid parameter, pmh:%p, mqtt_event_cb:%s", pmh);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt handle not init, mqtt_client:%p", pmh->mqtt_client);
	ret = esp_mqtt_client_register_event(pmh->mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, pmh);
	lbtrace("ret:%d = esp_mqtt_client_register_event(pmh->mqtt_client:%p, ESP_EVENT_ANY_ID:%d, mqtt_event_handler:%p, powner:%p)\n", ret, pmh->mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, powner);
	*/
	return ret;
}

int sun_mqtt_connect(void* handle, const char* pmqtt_host, int mqtt_port, int keep_live_time)
{
	int ret = -1;
	char url[256];
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_value(NULL == pmh || NULL == pmqtt_host || mqtt_port <= 0, -1, "Invalid parameter, pmh:%p, pmqtt_host:%s, mqtt_port:%d", pmh, pmqtt_host, mqtt_port);

	//lbstrcp(pmh->cfg.host, pmqtt_host);
	//pmh->cfg.port = mqtt_port;
	//pmh->cfg.transport = MQTT_TRANSPORT_OVER_SSL;
	sprintf(url, "mqtts://%s:%d", pmqtt_host, mqtt_port);
	lbstrcp(pmh->purl, url);
	pmh->cfg.uri = pmh->purl;
	printf("sun_mqtt_connect\n");
	pmh->mqtt_client = esp_mqtt_client_init(&pmh->cfg);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt_client:%p = esp_mqtt_client_init(&pmh->cfg) failed", pmh->mqtt_client);
	printf("pmh->mqtt_client:%p = esp_mqtt_client_init(&pmh->cfg)\n", pmh->mqtt_client);
	ret = esp_mqtt_client_register_event(pmh->mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, pmh);
    lbcheck_return(ret, "ret:%d = esp_mqtt_client_register_event\n", ret);
	ret = esp_mqtt_client_start(pmh->mqtt_client);
	lbcheck_return(ret, "ret:%d = esp_mqtt_client_start(pmh->mqtt_client:%p)\n", ret, pmh->mqtt_client);
	lbinfo("sun_mqtt_connect end, ret:%d\n", ret);
	return ret;
}

int sun_mqtt_pub(void* handle, const char* ptopic, const char* payload, int payload_len, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_value(NULL == pmh || NULL == ptopic || NULL == payload || payload_len <= 0, -1, "Invalid parameter, pmh:%p, ptopic:%s, payload:%s, payload_len:%d", pmh, ptopic, payload, payload_len);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt handle not init, mqtt_client:%p", pmh->mqtt_client);
	ret = esp_mqtt_client_publish(pmh->mqtt_client, ptopic, payload, payload_len, qos, 0);
	lbcheck_return(ret, "ret:%d = esp_mqtt_client_publish(pmh->mqtt_client:%p, ptopic:%s, payload:%s, payload_len:%d, qos:%d, 0)\n", ret, pmh->mqtt_client, ptopic, payload, payload_len, qos);

	return ret;
}

int gen_flow_num(mqtt_handle* pmh, char* pbuf, int len)
{
	time_t sectime;
    time(&sectime);
    struct tm* nowTime;
    nowTime = localtime(&sectime);
    char current[1024];
    sprintf(current, "%02d%02d%02d-%04d%02d%02d-%d", nowTime->tm_hour, nowTime->tm_min, nowTime->tm_sec, nowTime->tm_year + 1900, nowTime->tm_mon+1, nowTime->tm_mday, pmh->flow_num++);
	if(strlen(current) + 1 <= len)
	{
		memcpy(pbuf, current, strlen(current) + 1);
		return strlen(current) + 1;
	}
	else
	{
		return -1;
	}
}

int sun_mqtt_quick_pub(void* handle, e_aiot_mqtt_msg_type msg_type, const char* event, const char* command, const char* data, const char* state, const char* result, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	cJSON* pjr = NULL;
	char* pstr = NULL;
	char* pbuf = (char*)malloc(256);
	const char* topic = NULL;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	//lbinfo("sun_mqtt_quick_pub begin, pmh:%p\n", pmh);
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt handle not init, mqtt_client:%p", pmh->mqtt_client);
	//lbinfo("%s(handle:%p, msg_type:%d, event:%p, command:%p, data:%p, state:%p, result:%p, qos:%d), pmh->ppub_topic:%s\n", __FUNCTION__, handle, msg_type, event, command, data, state, result, qos, pmh->ppub_topic);

	do{
		pjr = cJSON_CreateObject();
		//lbtrace("sun_mqtt_quick_pub(handle:%p, msg_type:%d, event:%s, command:%s, data:%s, state:%s, result:%s, qos:%d\n)", handle, msg_type, event, command, data, state, result, qos);
		cJSON_AddNumberToObject(pjr, "method", msg_type);
		ret = gen_flow_num(pmh, pbuf, 256);
		cJSON_AddStringToObject(pjr, "flownum", pbuf);
		sprintf(pbuf, "%lu", lbget_sys_time());
		cJSON_AddStringToObject(pjr, "timestamp", pbuf);
		if(e_aiot_mqtt_msg_type_event_report == msg_type && event)
		{
			cJSON_AddStringToObject(pjr, "event", event);
		}

		if(command)
		{
			cJSON *pcom_obj = cJSON_Parse(command);
			//lbinfo("pcom_obj:%p = cJSON_Parse(command:%s)\n", pcom_obj, command);
			lbcheck_pointer_break(pcom_obj, "pcom_obj:%p = cJSON_Parse(command:%s)\n", pcom_obj, command);
			cJSON_AddItemToObject(pjr, "command", pcom_obj);
		}

		if(data)
		{
			cJSON *pdata_obj = cJSON_Parse(data);
			//lbinfo("pdata_obj:%p = cJSON_Parse(data:%s)\n", pdata_obj, data);
			lbcheck_pointer_break(pdata_obj, "pdata_obj:%p = cJSON_Parse(data:%s)\n", pdata_obj, data);
			cJSON_AddItemToObject(pjr, "data", pdata_obj);
		}

		if(state)
		{
			cJSON *pstate_obj = cJSON_Parse(state);
			//lbinfo("pstate_obj:%p = cJSON_Parse(state:%s)\n", pstate_obj, state);
			lbcheck_pointer_break(pstate_obj, "pstate_obj:%p = cJSON_Parse(state:%s) failed\n", pstate_obj, state);
			cJSON_AddItemToObject(pjr, "state", pstate_obj);
		}

		if(result)
		{
			cJSON *presult_obj = cJSON_Parse(result);
			//lbinfo("presult_obj:%p = cJSON_Parse(result:%s)\n", presult_obj, result);
			lbcheck_pointer_break(presult_obj, "presult_obj:%p = cJSON_Parse(result:%s) failed\n", presult_obj, result);
			cJSON_AddItemToObject(pjr, "result", presult_obj);
		}
		if(NULL == topic)
		{
			topic = pmh->ppub_topic;
		}

		pstr = cJSON_PrintUnformatted(pjr);
		//lbinfo("pstr:%s = cJSON_PrintUnformatted(pjr:%p)\n", pstr, pjr);
		lbmem_add_ref(pstr, strlen(pstr) + 1);
		lbcheck_pointer_break(pstr, "pstr:%s = cJSON_PrintUnformatted(pJsonRoot:%p)\n", pstr, pjr);
		//lbinfo("before esp_mqtt_client_publish(pmh->mqtt_client:%p, topic:%s, pstr:%s, strlen(pstr):%ld, qos:%d, 0)\n", pmh->mqtt_client, topic, pstr, strlen(pstr), qos);
		//sleep(1);
		ret = esp_mqtt_client_publish(pmh->mqtt_client, topic, pstr, strlen(pstr), qos, 0);
		lbcheck_break(ret, "ret:%d = esp_mqtt_client_publish(pmh->mqtt_client:%p, topic:%s,\n pstr:%s, strlen(pstr):%d, qos:%d, 0)\n", ret, pmh->mqtt_client, topic, pstr, strlen(pstr), qos);
		lbinfo("ret:%d = esp_mqtt_client_publish(pmh->mqtt_client:%p, topic:%s, pstr:%s, strlen(pstr):%d, qos:%d, 0)\n", ret, pmh->mqtt_client, topic, pstr, strlen(pstr), qos);
	}while(0);
	//lbinfo("%s after while\n", __FUNCTION__);
	lbfree(pbuf);
	lbfree(pstr);
	//lbinfo("%s lbfree(pstr)\n", __FUNCTION__);
	cJSON_Delete(pjr);
	//lbinfo("%s end\n", __FUNCTION__);
	return ret;
}

int sun_mqtt_pub_ctrl(void* handle, const char* command, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	ret = sun_mqtt_quick_pub(handle, e_aiot_mqtt_msg_type_control, NULL, command, NULL, NULL, NULL, qos);
	lbcheck_return(ret, "ret:%d = sun_mqtt_quick_pub(handle:%p, e_aiot_mqtt_msg_type_control:%d, NULL, command:%s, NULL, NULL, NULL, qos:%d)\n", ret, handle, e_aiot_mqtt_msg_type_control, command, qos);
	return ret;
}

int sun_mqtt_pub_update(void* handle, const char* state, const char* result, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	lbtrace("%s(handle:%p, state:%s, result:%s, qos:%d), pmh->ppub_topic:%s\n", __FUNCTION__, handle, state, result, qos, pmh->ppub_topic);

	ret = sun_mqtt_quick_pub(handle, e_aiot_mqtt_msg_type_update, NULL, NULL, NULL, state, result, qos);
	lbcheck_return(ret, "ret:%d = sun_mqtt_quick_pub(handle:%p, e_aiot_mqtt_msg_type_update, NULL, NULL, NULL, state:%s, result:%s, qos:%d)\n", ret, handle, state, result, qos);
	return ret;
}


int sun_mqtt_pub_event(void* handle, const char* event, const char* data, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	lbtrace("%s(handle:%p, event:%s, data:%s, qos:%d)\n", __FUNCTION__, handle, event, data, qos);

	ret = sun_mqtt_quick_pub(handle, e_aiot_mqtt_msg_type_event_report, event, NULL, data, NULL, NULL, qos);
	lbcheck_return(ret, "ret:%d = = sun_mqtt_quick_pub(handle:%p, e_aiot_mqtt_msg_type_event_report, event:%s, NULL, data,:%s NULL, NULL, qos:%d)\n", handle, event, data, qos);
	return ret;
}

int sun_mqtt_pub_property(void* handle, const char* state, e_aiot_mqtt_qos qos)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	//lbtrace("%s(handle:%p, state:%s, qos:%d)\n", __FUNCTION__, handle, state, qos);

	ret = sun_mqtt_quick_pub(handle, e_aiot_mqtt_msg_type_property_report, NULL, NULL, NULL, state, NULL, qos);
	lbcheck_return(ret, "ret:%d = sun_mqtt_quick_pub(handle:%p, e_aiot_mqtt_msg_type_property_report:%d, NULL, NULL, NULL, state:%s, NULL, qos:%d)\n", ret, handle, e_aiot_mqtt_msg_type_property_report, state, qos);
	return ret;
}

int sun_mqtt_sub(void* handle, const char* ptopic, e_aiot_mqtt_qos qos, on_mqtt_subscribe_cb on_topic_msg, void* userdata)
{
	int ret = -1;
	int msg_id = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	//lbinfo("%s()\n", __FUNCTION__);
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt handle not init, mqtt_client:%p", pmh->mqtt_client);
	if(NULL == ptopic)
	{
		ptopic = pmh->psub_topic;
	}
	//lbinfo("%s(handle:%p, ptopic:%s, qos:%d, on_topic_msg:%p, userdata:%p)\n", __FUNCTION__, handle, ptopic, qos, on_topic_msg, userdata);
	
	msg_id = esp_mqtt_client_subscribe(pmh->mqtt_client, ptopic, qos);
	ret = msg_id == -1 ? msg_id : 0;
	lbcheck_return(ret, "ret:%d = esp_mqtt_client_subscribe(pmh->mqtt_client:%p, ptopic:%p, qos:%d)", ret, pmh->mqtt_client, ptopic, qos);
	lbinfo("msg_id:%d = esp_mqtt_client_subscribe(pmh->mqtt_client:%p, ptopic:%s, qos:%d)\n", msg_id, pmh->mqtt_client, ptopic, qos);
	
	ret = add_subscribe(pmh, msg_id, ptopic, qos, on_topic_msg, userdata);
	lbcheck_return(ret, "ret:%d = add_subscribe(pmh:%p, msg_id:%d, ptopic:%s, qos:%d, on_topic_msg:%p, userdata:%p)", ret, pmh, msg_id, ptopic, qos, on_topic_msg, userdata);
	lbinfo("ret:%d = add_subscribe(pmh, ptopic, qos, on_topic_msg, userdata)\n", ret);
	return ret;
}

int sun_mqtt_unsub(void* handle, const char* ptopic)
{
	int ret = -1;
	mqtt_handle* pmh = (mqtt_handle*)handle;
	lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);
	lbcheck_pointer(pmh->mqtt_client, -1, "mqtt handle not init, mqtt_client:%p", pmh->mqtt_client);
	
	ret = esp_mqtt_client_unsubscribe(pmh->mqtt_client, ptopic);
	lbcheck_return(ret, "ret:%d = esp_mqtt_client_unsubscribe(pmh->mqtt_client:%p, ptopic:%s)", ret, pmh->mqtt_client, ptopic);

	return ret;
}

void sun_mqtt_disconnect(void* handle)
{
	mqtt_handle* pmh = (mqtt_handle*)handle;
	//lbcheck_pointer(pmh, -1, "Invalid parameter, pmh:%p", pmh);

	if(pmh && pmh->mqtt_client)
	{
		esp_mqtt_client_disconnect(pmh->mqtt_client);
		esp_mqtt_client_stop(pmh->mqtt_client);
	}
}

void sun_mqtt_deinit(void** pphandle)
{
	if(pphandle && *pphandle)
	{
		mqtt_handle* pmh = *(mqtt_handle**)pphandle;

		// mqtt config
		/*lbfree(pmh->cfg.host);
		lbfree(pmh->cfg.uri);
		lbfree(pmh->cfg.client_id);
		lbfree(pmh->cfg.username);
		lbfree(pmh->cfg.password);
		lbfree(pmh->cfg.lwt_topic);
		lbfree(pmh->cfg.lwt_msg);
		lbfree(pmh->cfg.cert_pem);
		lbfree(pmh->cfg.client_cert_pem);
		lbfree(pmh->cfg.client_key_pem);
		lbfree(pmh->cfg.clientkey_password);*/

		// mqtt handle
		lbfree(pmh->pclient_id);
		lbfree(pmh->puser_name);
		lbfree(pmh->ppassword);
		lbfree(pmh->psub_topic);
		lbfree(pmh->ppub_topic);
		lbfree(pmh->pcert_pem);
		lbfree(pmh->purl);

		if(pmh->sub_list)
		{
			while(lblist_size(pmh->sub_list) > 0)
			{
				sub_info* psi = (sub_info*)lblist_pop(pmh->sub_list);
				if(psi)
				{
					lbfree(psi->ptopic);
					lbfree(psi->payload);
					lbfree(psi);
				}
			}
			lblist_context_close(pmh->sub_list);
		}

		if(pmh->mqtt_client)
		{
			esp_mqtt_client_disconnect(pmh->mqtt_client);
			esp_mqtt_client_stop(pmh->mqtt_client);
			esp_mqtt_client_destroy(pmh->mqtt_client);
		}

		lbfree(pmh);
		*pphandle = NULL;
	}
}
