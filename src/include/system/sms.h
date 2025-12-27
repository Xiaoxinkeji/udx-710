/**
 * @file sms.h
 * @brief 短信管理模块 - D-Bus监控、发送、存储、转发
 */

#ifndef SMS_H
#define SMS_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 短信数据结构 */
typedef struct {
    int id;
    char sender[64];
    char content[1024];
    time_t timestamp;
    int is_read;
} SmsMessage;

/* Webhook配置结构 */
typedef struct {
    int enabled;
    char platform[32];
    char url[512];
    char body[2048];
    char headers[512];
} WebhookConfig;

/**
 * 初始化短信模块
 * @param db_path 数据库文件路径
 * @return 0成功, -1失败
 */
int sms_init(const char *db_path);

/**
 * 关闭短信模块
 */
void sms_deinit(void);

/**
 * 发送短信
 * @param recipient 收件人号码
 * @param content 短信内容
 * @param result_path 返回的消息路径(可为NULL)
 * @param path_size result_path缓冲区大小
 * @return 0成功, -1失败
 */
int sms_send(const char *recipient, const char *content, char *result_path, size_t path_size);

/**
 * 获取短信列表
 * @param messages 输出数组
 * @param max_count 最大数量
 * @return 实际获取的数量, -1失败
 */
int sms_get_list(SmsMessage *messages, int max_count);

/**
 * 获取短信总数
 * @return 短信数量, -1失败
 */
int sms_get_count(void);

/**
 * 删除短信
 * @param id 短信ID
 * @return 0成功, -1失败
 */
int sms_delete(int id);

/**
 * 清空所有短信
 * @return 0成功, -1失败
 */
int sms_clear_all(void);

/**
 * 获取Webhook配置
 * @param config 输出配置
 * @return 0成功, -1失败
 */
int sms_get_webhook_config(WebhookConfig *config);

/**
 * 保存Webhook配置
 * @param config 配置
 * @return 0成功, -1失败
 */
int sms_save_webhook_config(const WebhookConfig *config);

/**
 * 测试Webhook
 * @return 0成功, -1失败
 */
int sms_test_webhook(void);

/* 发送记录结构 */
typedef struct {
    int id;
    char recipient[64];
    char content[1024];
    time_t timestamp;
    char status[32];
} SentSmsMessage;

/**
 * 获取发送记录列表
 * @param messages 输出数组
 * @param max_count 最大数量
 * @return 实际获取的数量, -1失败
 */
int sms_get_sent_list(SentSmsMessage *messages, int max_count);

/**
 * 获取最大存储数量配置
 * @return 最大数量
 */
int sms_get_max_count(void);

/**
 * 设置最大存储数量
 * @param count 最大数量
 * @return 0成功, -1失败
 */
int sms_set_max_count(int count);

/**
 * 获取发送记录最大存储数量
 * @return 最大数量
 */
int sms_get_max_sent_count(void);

/**
 * 设置发送记录最大存储数量
 * @param count 最大数量(1-20)
 * @return 0成功, -1失败
 */
int sms_set_max_sent_count(int count);

/**
 * 删除发送记录
 * @param id 记录ID
 * @return 0成功, -1失败
 */
int sms_delete_sent(int id);

/**
 * 检查短信模块状态
 * @return 1正常, 0异常
 */
int sms_check_status(void);

/**
 * 定期维护短信模块（检查并恢复D-Bus连接）
 * 应在主循环中定期调用
 */
void sms_maintenance(void);

/**
 * 获取短信接收修复开关状态
 * @return 1开启, 0关闭
 */
int sms_get_fix_enabled(void);

/**
 * 设置短信接收修复开关
 * @param enabled 1开启, 0关闭
 * @return 0成功, -1失败
 */
int sms_set_fix_enabled(int enabled);

/**
 * 获取管理员号码
 * @param buf 缓冲区
 * @param size 缓冲区大小
 * @return 0成功, -1失败
 */
int sms_get_admin_number(char *buf, size_t size);

/**
 * 设置管理员号码
 * @param number 管理员号码
 * @return 0成功, -1失败
 */
int sms_set_admin_number(const char *number);

#ifdef __cplusplus
}
#endif

#endif /* SMS_H */
