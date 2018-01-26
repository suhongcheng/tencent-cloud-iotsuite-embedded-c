#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);

int run_mqtt(tc_iot_mqtt_client_config* p_client_config);

tc_iot_mqtt_client_config g_client_config = {
    {
        // device info
        TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
        TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
        TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
    },
    TC_IOT_CONFIG_SERVER_HOST,
    TC_IOT_CONFIG_SERVER_PORT,
    TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
    TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
    TC_IOT_CONFIG_CLEAN_SESSION,
    TC_IOT_CONFIG_USE_TLS,
    TC_IOT_CONFIG_AUTO_RECONNECT,
    TC_IOT_CONFIG_ROOT_CA,
    TC_IOT_CONFIG_CLIENT_CRT,
    TC_IOT_CONFIG_CLIENT_KEY,
};

char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_SUB_TOPIC_DEF;
char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN+1] = TC_IOT_PUB_TOPIC_DEF;

int main(int argc, char** argv) {
    int ret = 0;
    bool token_defined;
    tc_iot_mqtt_client_config * p_client_config;

    p_client_config = &(g_client_config);
    parse_command(p_client_config, argc, argv);

    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!token_defined) {
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token(
                TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                &p_client_config->device_info);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    snprintf(sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    tc_iot_hal_printf("sub topic: %s\n", sub_topic);
    tc_iot_hal_printf("pub topic: %s\n", pub_topic);
    run_mqtt(&g_client_config);
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
}

static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop = 1;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop = 1;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
}

int run_mqtt(tc_iot_mqtt_client_config* p_client_config) {
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    tc_iot_mqtt_client client;
    tc_iot_mqtt_client* p_client = &client;
    tc_iot_mqtt_client_construct(p_client, p_client_config);
    int ret = tc_iot_mqtt_client_subscribe(p_client, sub_topic, QOS1,
                                           _on_message_received);
    int timeout = 2000;
    tc_iot_mqtt_client_yield(p_client, timeout);

    while (!stop) {
        char* action_get = "{\"method\":\"get\"}";

        tc_iot_mqtt_message pubmsg;
        memset(&pubmsg, '\0', sizeof(pubmsg));
        pubmsg.payload = action_get;
        pubmsg.payloadlen = strlen(pubmsg.payload);
        pubmsg.qos = QOS1;
        pubmsg.retained = 0;
        pubmsg.dup = 0;
        tc_iot_hal_printf("[c->s] shadow_get\n");
        ret = tc_iot_mqtt_client_publish(p_client, pub_topic, &pubmsg);
        if (TC_IOT_SUCCESS != ret) {
            if (ret != TC_IOT_MQTT_RECONNECT_IN_PROGRESS) {
                tc_iot_hal_printf("publish failed: %d, not reconnect, exiting now\n", ret);
                break;
            } else {
                tc_iot_hal_printf("publish failed client trying to reconnect.\n");
            }
        }

        timeout = 5000;
        tc_iot_mqtt_client_yield(p_client, timeout);
    }

    tc_iot_mqtt_client_disconnect(p_client);
}

