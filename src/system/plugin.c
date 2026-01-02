/**
 * @file plugin.c
 * @brief 插件管理和Shell执行模块实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include "plugin.h"

/* 危险命令黑名单 */
static const char *dangerous_commands[] = {
    "rm -rf /",
    "rm -rf /*",
    "mkfs",
    "dd if=",
    ":(){:|:&};:",
    "chmod -R 777 /",
    "chown -R",
    "> /dev/sda",
    "mv /* ",
    NULL
};

/* 检查命令是否安全 */
static int is_command_safe(const char *cmd) {
    for (int i = 0; dangerous_commands[i] != NULL; i++) {
        if (strstr(cmd, dangerous_commands[i]) != NULL) {
            return 0;
        }
    }
    return 1;
}

/* 确保插件目录存在 */
int ensure_plugin_dir(void) {
    struct stat st = {0};
    if (stat(PLUGIN_DIR, &st) == -1) {
        if (mkdir(PLUGIN_DIR, 0755) == -1) {
            /* 尝试递归创建 */
            extern int run_command(char *output, size_t size, const char *cmd, ...);
            char output[256];
            return run_command(output, sizeof(output), "mkdir", "-p", PLUGIN_DIR, NULL) == 0 ? 0 : -1;
        }
    }
    return 0;
}

/* 执行Shell命令 */
int execute_shell(const char *cmd, char *output, size_t size) {
    if (!cmd || !output || size == 0) {
        return -1;
    }

    /* 安全检查 */
    if (!is_command_safe(cmd)) {
        snprintf(output, size, "Error: Command blocked for security reasons");
        return -1;
    }

    FILE *fp = popen(cmd, "r");
    if (!fp) {
        snprintf(output, size, "Error: Failed to execute command");
        return -1;
    }

    size_t total = 0;
    char buffer[256];
    output[0] = '\0';

    while (fgets(buffer, sizeof(buffer), fp) != NULL && total < size - 1) {
        size_t len = strlen(buffer);
        if (total + len >= size - 1) {
            len = size - 1 - total;
        }
        memcpy(output + total, buffer, len);
        total += len;
    }
    output[total] = '\0';

    int status = pclose(fp);
    return WIFEXITED(status) && WEXITSTATUS(status) == 0 ? 0 : -1;
}


/* JSON字符串转义 */
static void json_escape(const char *src, char *dst, size_t dst_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j < dst_size - 2; i++) {
        char c = src[i];
        switch (c) {
            case '"':  if (j + 2 < dst_size) { dst[j++] = '\\'; dst[j++] = '"'; } break;
            case '\\': if (j + 2 < dst_size) { dst[j++] = '\\'; dst[j++] = '\\'; } break;
            case '\n': if (j + 2 < dst_size) { dst[j++] = '\\'; dst[j++] = 'n'; } break;
            case '\r': if (j + 2 < dst_size) { dst[j++] = '\\'; dst[j++] = 'r'; } break;
            case '\t': if (j + 2 < dst_size) { dst[j++] = '\\'; dst[j++] = 't'; } break;
            default:
                if ((unsigned char)c >= 0x20) {
                    dst[j++] = c;
                }
                break;
        }
    }
    dst[j] = '\0';
}

/* 从插件内容中提取元信息 */
static int extract_plugin_meta(const char *content, char *name, char *version,
                                char *author, char *description, char *icon, char *color) {
    /* 默认值 */
    strcpy(name, "未命名插件");
    strcpy(version, "1.0.0");
    strcpy(author, "未知");
    strcpy(description, "");
    strcpy(icon, "fa-puzzle-piece");
    strcpy(color, "from-blue-500 to-cyan-400");

    /* 首先尝试解析 JSDoc 风格的注释 (@name, @version 等) */
    const char *jsdoc_tags[] = {"@name ", "@version ", "@author ", "@description ", "@icon ", "@color "};
    char *jsdoc_dests[] = {name, version, author, description, icon, color};

    int found_jsdoc = 0;
    for (int i = 0; i < 6; i++) {
        const char *p = strstr(content, jsdoc_tags[i]);
        if (p) {
            found_jsdoc = 1;
            p += strlen(jsdoc_tags[i]);
            /* 跳过空白字符 */
            while (*p == ' ' || *p == '\t') p++;

            char *dst = jsdoc_dests[i];
            int j = 0;
            /* 提取到行尾或 */ 为止 */
            while (*p && *p != '\n' && *p != '\r' && j < 127) {
                /* 如果遇到注释结束符，停止 */
                if (*p == '*' && *(p+1) == '/') break;
                dst[j++] = *p++;
            }
            /* 去除尾部空白 */
            while (j > 0 && (dst[j-1] == ' ' || dst[j-1] == '\t')) j--;
            dst[j] = '\0';
        }
    }

    /* 如果找到了 JSDoc 注释，直接返回 */
    if (found_jsdoc) {
        return 0;
    }

    /* 否则尝试解析 window.PLUGIN 对象内的元数据 */
    const char *plugin_start = strstr(content, "window.PLUGIN");
    if (!plugin_start) {
        plugin_start = strstr(content, "PLUGIN");
    }

    /* 如果找不到PLUGIN定义，使用整个内容 */
    if (!plugin_start) {
        plugin_start = content;
    }

    /* 简单解析 name: 'xxx' 或 name: "xxx" */
    const char *obj_tags[] = {"name:", "version:", "author:", "description:", "icon:", "color:"};
    char *obj_dests[] = {name, version, author, description, icon, color};

    for (int i = 0; i < 6; i++) {
        const char *p = strstr(plugin_start, obj_tags[i]);
        if (p) {
            p += strlen(obj_tags[i]);
            /* 跳过空白字符（包括空格、制表符、换行符） */
            while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
            if (*p == '\'' || *p == '"') {
                char quote = *p++;
                char *dst = obj_dests[i];
                int j = 0;
                /* 提取值，支持转义字符 */
                while (*p && *p != quote && j < 127) {
                    if (*p == '\\' && *(p+1)) {
                        /* 处理转义字符 */
                        p++;
                        if (*p == 'n') dst[j++] = '\n';
                        else if (*p == 't') dst[j++] = '\t';
                        else if (*p == 'r') dst[j++] = '\r';
                        else if (*p == '\\') dst[j++] = '\\';
                        else if (*p == '\'' || *p == '"') dst[j++] = *p;
                        else dst[j++] = *p;
                        p++;
                    } else {
                        dst[j++] = *p++;
                    }
                }
                dst[j] = '\0';
            }
        }
    }
    return 0;
}

/* 获取插件列表 */
int get_plugin_list(char *json_output, size_t size) {
    ensure_plugin_dir();

    DIR *dir = opendir(PLUGIN_DIR);
    if (!dir) {
        snprintf(json_output, size, "[]");
        return 0;
    }

    int offset = 0;
    int count = 0;
    offset += snprintf(json_output + offset, size - offset, "[");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && count < PLUGIN_MAX_COUNT) {
        /* 只处理.js文件 */
        const char *ext = strrchr(entry->d_name, '.');
        if (!ext || strcmp(ext, ".js") != 0) continue;

        /* 读取文件内容 */
        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", PLUGIN_DIR, entry->d_name);

        FILE *fp = fopen(filepath, "r");
        if (!fp) continue;

        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        if (fsize > PLUGIN_MAX_SIZE) {
            fclose(fp);
            continue;
        }

        char *content = malloc(fsize + 1);
        if (!content) {
            fclose(fp);
            continue;
        }

        fread(content, 1, fsize, fp);
        content[fsize] = '\0';
        fclose(fp);

        /* 提取元信息 */
        char name[128], version[32], author[64], description[256], icon[64], color[128];
        extract_plugin_meta(content, name, version, author, description, icon, color);

        /* 转义内容 */
        char *escaped_content = malloc(fsize * 2 + 1);
        if (escaped_content) {
            json_escape(content, escaped_content, fsize * 2 + 1);

            /* 添加到JSON数组 */
            offset += snprintf(json_output + offset, size - offset,
                "%s{\"filename\":\"%s\",\"name\":\"%s\",\"version\":\"%s\","
                "\"author\":\"%s\",\"description\":\"%s\",\"icon\":\"%s\","
                "\"color\":\"%s\",\"content\":\"%s\"}",
                count > 0 ? "," : "",
                entry->d_name, name, version, author, description, icon, color, escaped_content);

            free(escaped_content);
            count++;
        }
        free(content);
    }

    closedir(dir);
    offset += snprintf(json_output + offset, size - offset, "]");

    return count;
}


/* 保存插件 */
int save_plugin(const char *name, const char *content) {
    if (!name || !content) return -1;

    ensure_plugin_dir();

    /* 验证文件名 */
    if (strstr(name, "..") || strstr(name, "/")) {
        return -1;
    }

    /* 确保以.js结尾 */
    char filename[256];
    if (strstr(name, ".js")) {
        snprintf(filename, sizeof(filename), "%s/%s", PLUGIN_DIR, name);
    } else {
        snprintf(filename, sizeof(filename), "%s/%s.js", PLUGIN_DIR, name);
    }

    /* 检查大小 */
    if (strlen(content) > PLUGIN_MAX_SIZE) {
        return -1;
    }

    FILE *fp = fopen(filename, "w");
    if (!fp) return -1;

    fputs(content, fp);
    fclose(fp);

    return 0;
}

/* 删除插件 */
int delete_plugin(const char *name) {
    if (!name) return -1;

    /* 验证文件名 */
    if (strstr(name, "..") || strstr(name, "/")) {
        return -1;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", PLUGIN_DIR, name);

    /* 确保文件存在且是.js文件 */
    const char *ext = strrchr(name, '.');
    if (!ext || strcmp(ext, ".js") != 0) {
        return -1;
    }

    return unlink(filepath) == 0 ? 0 : -1;
}

/* 删除所有插件 */
int delete_all_plugins(void) {
    DIR *dir = opendir(PLUGIN_DIR);
    if (!dir) return 0;

    struct dirent *entry;
    int deleted = 0;

    while ((entry = readdir(dir)) != NULL) {
        /* 只删除.js文件，保留默认插件 */
        const char *ext = strrchr(entry->d_name, '.');
        if (!ext || strcmp(ext, ".js") != 0) continue;

        /* 跳过默认插件 */
        if (strncmp(entry->d_name, "_default", 8) == 0) continue;

        char filepath[512];
        snprintf(filepath, sizeof(filepath), "%s/%s", PLUGIN_DIR, entry->d_name);

        if (unlink(filepath) == 0) {
            deleted++;
        }
    }

    closedir(dir);
    return 0;
}
