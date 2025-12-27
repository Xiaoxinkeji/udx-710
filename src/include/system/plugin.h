/**
 * @file plugin.h
 * @brief 插件管理和Shell执行模块
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 插件存储目录 */
#define PLUGIN_DIR "/home/root/9898/Plugins/plugins"

/* 插件文件最大大小 (100KB) */
#define PLUGIN_MAX_SIZE (100 * 1024)

/* 最大插件数量 */
#define PLUGIN_MAX_COUNT 20

/**
 * @brief 执行Shell命令
 * @param cmd 要执行的命令
 * @param output 输出缓冲区
 * @param size 缓冲区大小
 * @return 0 成功, -1 失败
 */
int execute_shell(const char *cmd, char *output, size_t size);

/**
 * @brief 获取插件列表
 * @param json_output JSON输出缓冲区
 * @param size 缓冲区大小
 * @return 插件数量, -1 失败
 */
int get_plugin_list(char *json_output, size_t size);

/**
 * @brief 保存插件
 * @param name 插件名称
 * @param content 插件内容
 * @return 0 成功, -1 失败
 */
int save_plugin(const char *name, const char *content);

/**
 * @brief 删除插件
 * @param name 插件名称
 * @return 0 成功, -1 失败
 */
int delete_plugin(const char *name);

/**
 * @brief 删除所有插件
 * @return 0 成功, -1 失败
 */
int delete_all_plugins(void);

/**
 * @brief 确保插件目录存在
 * @return 0 成功, -1 失败
 */
int ensure_plugin_dir(void);

#ifdef __cplusplus
}
#endif

#endif /* PLUGIN_H */
