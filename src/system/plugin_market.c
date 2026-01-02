/**
 * @file plugin_market.c
 * @brief 插件商城后端：获取远程插件列表、下载、校验、解压
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "plugin_market.h"
#include "exec_utils.h"
#include "sha256.h"
#include "plugin.h"          /* 提供 PLUGIN_DIR */

#define MARKET_LIST_URL_DEFAULT  "https://raw.githubusercontent.com/Xiaoxinkeji/udx-710-plugins/main/index.json"
#define MARKET_TMP_JSON          "/tmp/plugin_market_index.json"
#define MARKET_TMP_ZIP           "/tmp/plugin_market_download.zip"
#define MARKET_TMP_DIR           "/tmp/plugin_market_extract"

static char g_market_mirror[512] = {0};

/* 设置镜像地址，空则使用默认 */
void plugin_market_set_mirror(const char *mirror) {
    if (mirror && strlen(mirror) < sizeof(g_market_mirror)) {
        strncpy(g_market_mirror, mirror, sizeof(g_market_mirror) - 1);
    } else {
        g_market_mirror[0] = '\0';
    }
}

/* 获取实际使用的镜像地址 */
static const char *get_mirror(void) {
    return g_market_mirror[0] ? g_market_mirror : MARKET_LIST_URL_DEFAULT;
}

/* 下载文件到指定路径，优先 curl，失败用 wget */
static int download_file(const char *url, const char *dest_path) {
    extern int run_command(char *output, size_t size, const char *cmd, ...);
    char output[256];

    /* 尝试 curl */
    if (run_command(output, sizeof(output), "curl", "-k", "-s", "-L", "-o", dest_path, url, NULL) == 0) {
        return 0;
    }
    /* 尝试 wget */
    return run_command(output, sizeof(output), "wget", "--no-check-certificate", "-q", "-O", dest_path, url, NULL);
}

/* 拉取远程插件列表，输出到 json_buffer */
int plugin_market_fetch_list(char *json_buffer, size_t size) {
    if (!json_buffer || size == 0) return -1;
    /* 清理旧缓存 */
    unlink(MARKET_TMP_JSON);
    if (download_file(get_mirror(), MARKET_TMP_JSON) != 0) {
        snprintf(json_buffer, size, "{\"error\":\"下载失败，请检查网络或镜像配置\"}");
        return -1;
    }
    /* 读取内容 */
    FILE *fp = fopen(MARKET_TMP_JSON, "rb");
    if (!fp) {
        snprintf(json_buffer, size, "{\"error\":\"无法读取缓存\"}");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (len <= 0 || len >= (long)size) {
        fclose(fp);
        snprintf(json_buffer, size, "{\"error\":\"列表过大\"}");
        return -1;
    }
    fread(json_buffer, 1, len, fp);
    json_buffer[len] = '\0';
    fclose(fp);
    unlink(MARKET_TMP_JSON);
    return 0;
}

/* 下载并安装插件 */
int plugin_market_install(const char *plugin_name, const char *expected_sha256) {
    if (!plugin_name) return -1;
    /* 拼接下载地址：镜像前缀 + plugin_name + ".zip" */
    char url[768];
    const char *base = get_mirror();
    /* 去掉末尾的 index.json 替换为 plugins/xxx.zip */
    char base_dir[512];
    strncpy(base_dir, base, sizeof(base_dir)-1);
    base_dir[sizeof(base_dir)-1]='\0';
    char *slash = strrchr(base_dir, '/');
    if (slash) *slash = '\0';
    snprintf(url, sizeof(url), "%s/plugins/%s.zip", base_dir, plugin_name);

    /* 下载 */
    unlink(MARKET_TMP_ZIP);
    if (download_file(url, MARKET_TMP_ZIP) != 0) return -2;

    /* 校验 SHA256 */
    if (expected_sha256 && strlen(expected_sha256) == 64) {
        char real_sha[65];
        if (sha256_file(MARKET_TMP_ZIP, real_sha, sizeof(real_sha)) != 0 ||
            strcasecmp(real_sha, expected_sha256) != 0) {
            unlink(MARKET_TMP_ZIP);
            return -3; /* 校验失败 */
        }
    }

    /* 解压到临时目录 */
    extern int run_command(char *output, size_t size, const char *cmd, ...);
    char output[256];
    run_command(output, sizeof(output), "rm", "-rf", MARKET_TMP_DIR, NULL);
    mkdir(MARKET_TMP_DIR, 0755);

    if (run_command(output, sizeof(output), "unzip", "-q", "-d", MARKET_TMP_DIR, MARKET_TMP_ZIP, NULL) != 0) {
        unlink(MARKET_TMP_ZIP);
        return -4;
    }

    /* 移动 *.js 到插件目录 */
    char shell_cmd[512];
    snprintf(shell_cmd, sizeof(shell_cmd), "mv %s/*.js \"%s\"", MARKET_TMP_DIR, PLUGIN_DIR);
    int ret = run_command(output, sizeof(output), "sh", "-c", shell_cmd, NULL) == 0 ? 0 : -5;

    /* 清理 */
    run_command(output, sizeof(output), "rm", "-rf", MARKET_TMP_DIR, NULL);
    unlink(MARKET_TMP_ZIP);
    return ret;
}