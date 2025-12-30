#ifndef PLUGIN_MARKET_HANDLER_H
#define PLUGIN_MARKET_HANDLER_H

#include "mongoose.h"

/* 插件商城接口 */
void handle_plugin_market_list(struct mg_connection *c, struct mg_http_message *hm);
void handle_plugin_market_install(struct mg_connection *c, struct mg_http_message *hm);
void handle_plugin_market_mirror(struct mg_connection *c, struct mg_http_message *hm);

#endif