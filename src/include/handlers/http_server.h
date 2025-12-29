/**
 * @file http_server.h
 * @brief HTTP 服务器框架 (对应 Go: main.go)
 */

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 启动 HTTP 服务器
 * @param port 监听端口 (如 "80" 或 "8080")
 * @return 0 成功, -1 失败
 */
int http_server_start(const char *port);

/**
 * @brief 停止 HTTP 服务器
 */
void http_server_stop(void);

/**
 * @brief 运行服务器事件循环 (阻塞)
 */
void http_server_run(void);



#ifdef __cplusplus
}
#endif

#endif /* HTTP_SERVER_H */
