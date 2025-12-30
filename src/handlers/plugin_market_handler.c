/**
 * @file plugin_market_handler.c
 * @brief 插件商城 HTTP 接口实现
 */

#include "mongoose.h"
#include "plugin_market.h"
#include "handlers.h"

/* GET /api/plugins/market - 获取远程插件列表 */
void handle_plugin_market_list(struct mg_connection *c, struct mg_http_message *hm) {
    (void)hm;
    char json_buf[64 * 1024]; /* 64KB 足够 */
    if (plugin_market_fetch_list(json_buf, sizeof(json_buf)) != 0) {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                      "{\"error\":\"无法拉取插件列表\"}");
        return;
    }
    mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json_buf);
}

/* POST /api/plugins/market/install - 安装指定插件 */
void handle_plugin_market_install(struct mg_connection *c, struct mg_http_message *hm) {
    char plugin_name[256] = {0};
    char expected_sha256[65] = {0};
    /* 解析 JSON body */
    mg_http_get_var(&hm->body, "plugin_name", plugin_name, sizeof(plugin_name));
    mg_http_get_var(&hm->body, "sha256", expected_sha256, sizeof(expected_sha256));
    if (plugin_name[0] == '\0') {
        mg_http_reply(c, 400, "Content-Type: application/json\r\n",
                      "{\"error\":\"缺少 plugin_name\"}");
        return;
    }
    int rc = plugin_market_install(plugin_name, expected_sha256[0] ? expected_sha256 : NULL);
    if (rc == 0) {
        mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                      "{\"msg\":\"安装成功，请刷新插件列表\"}");
    } else {
        mg_http_reply(c, 500, "Content-Type: application/json\r\n",
                      "{\"error\":\"安装失败，错误码 %d\"}", rc);
    }
}

/* POST /api/plugins/market/mirror - 设置镜像地址 */
void handle_plugin_market_mirror(struct mg_connection *c, struct mg_http_message *hm) {
    char mirror[512] = {0};
    mg_http_get_var(&hm->body, "mirror", mirror, sizeof(mirror));
    plugin_market_set_mirror(mirror[0] ? mirror : NULL);
    mg_http_reply(c, 200, "Content-Type: application/json\r\n",
                  "{\"msg\":\"镜像地址已更新\"}");
}