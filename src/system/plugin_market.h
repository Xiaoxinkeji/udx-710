#ifndef PLUGIN_MARKET_H
#define PLUGIN_MARKET_H

#include <stddef.h>

/* 设置/获取镜像地址 */
void plugin_market_set_mirror(const char *mirror);

/* 拉取远程插件列表，成功返回 0，json_buffer 需由调用者保证足够大 */
int plugin_market_fetch_list(char *json_buffer, size_t size);

/* 下载并安装插件，expected_sha256 可选（NULL 则跳过校验） */
int plugin_market_install(const char *plugin_name, const char *expected_sha256);

#endif /* PLUGIN_MARKET_H */