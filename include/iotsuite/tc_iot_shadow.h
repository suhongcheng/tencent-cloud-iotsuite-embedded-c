#ifndef TC_IOT_SHADOW_H
#define TC_IOT_SHADOW_H


/**
 * @brief 影子设备配置
 */
typedef struct _tc_iot_shadow_config {
    tc_iot_mqtt_client_config mqtt_client_config;  /**< MQTT 相关配置*/
    char sub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN]; /**< 影子设备订阅 Topic*/
    char pub_topic[TC_IOT_MAX_MQTT_TOPIC_LEN];  /**< 影子设备消息 Publish Topic*/
    message_handler on_receive_msg; /**< 影子设备消息回调*/
} tc_iot_shadow_config;

typedef enum _tc_iot_command_ack_status_e {
    TC_IOT_ACK_SUCCESS,
    TC_IOT_ACK_TIMEOUT,
} tc_iot_command_ack_status_e;

typedef void (*message_ack_handler)(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);


#define TC_IOT_SESSION_ID_LEN     8
#define TC_IOT_MAX_SESSION_COUNT  3

typedef struct _tc_iot_shadow_session{
    char sid[TC_IOT_SESSION_ID_LEN+1];
    tc_iot_timer        timer;
    message_ack_handler handler;    
    void * session_context;
}tc_iot_shadow_session;


/**
 * @brief 影子设备客户端
 */
typedef struct _tc_iot_shadow_client {
    tc_iot_shadow_config* p_shadow_config; /**< 影子设备配置*/
    tc_iot_mqtt_client mqtt_client; /**< MQTT 客户端*/
    tc_iot_shadow_session sessions[TC_IOT_MAX_SESSION_COUNT];
} tc_iot_shadow_client;


/**
 * @brief tc_iot_shadow_doc_pack_for_get 构造 get 请求包，用于读取影子设备服务端数据。
 *
 * @param buffer 输出结果缓存区 
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_get(char * buffer, int buffer_len, tc_iot_shadow_client *c);


/**
 * @brief tc_iot_shadow_doc_pack_for_update 生成影子设备文档或属性上报请求包
 *
 * @param buffer 输出结果缓存区 
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 * @param reported Reported 文档上报数据
 * @param desired  Desired 文档上报数据
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_update(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);

/**
 * @brief tc_iot_shadow_doc_pack_for_delete 生成影子设备删除文档或属性请求包
 *
 * @param buffer 输出结果缓存区 
 * @param buffer_len 输出结果缓存区大小
 * @param c 影子设备客户端
 * @param reported Reported 属性数据删除指令
 * @param desired  Desired 属性数据删除指令
 *
 * @return 输出结果长度
 */
int tc_iot_shadow_doc_pack_for_delete(char * buffer, int buffer_len, tc_iot_shadow_client *c, const char * reported, const char * desired);


/*--- begin 影子设备请求响应包 method 字段取值----*/
/* 请求类 */
/**< 读取服务端影子设备数据*/
#define TC_IOT_MQTT_METHOD_GET       "get"
/**< 更新服务端影子设备数据*/
#define TC_IOT_MQTT_METHOD_UPDATE    "update"
/**< 删除服务端影子设备数据*/
#define TC_IOT_MQTT_METHOD_DELETE    "delete"

/* 响应类 */
/**< 读取请求响应*/
#define TC_IOT_MQTT_METHOD_REPLY     "reply"
/**< 更新请求响应*/
#define TC_IOT_MQTT_METHOD_CONTROL   "control"
/*--- end 影子设备请求响应包 method 字段取值----*/

int tc_iot_shadow_doc_pack_for_get_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    tc_iot_shadow_client *c);
int tc_iot_shadow_doc_pack_for_update_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) ;
int tc_iot_shadow_doc_pack_for_delete_with_sid(char *buffer, int buffer_len,
                                    char * session_id, int session_id_len,
                                    const char * reported, const char * desired,
                                    tc_iot_shadow_client *c) ;

int tc_iot_shadow_doc_pack_start(char *buffer, int buffer_len,
                                 char * session_id, int session_id_len,
                                 const char * method,
                                 tc_iot_shadow_client *c);

int tc_iot_shadow_doc_pack_format(char *buffer, int buffer_len, 
        const char * reported, 
        const char * desired);
int tc_iot_shadow_doc_pack_end(char *buffer, int buffer_len, tc_iot_shadow_client *c);

#endif /* end of include guard */

