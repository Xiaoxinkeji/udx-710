/**
 * @file database.c
 * @brief 数据库操作模块实现 - SQLite3 统一接口
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "database.h"
#include "exec_utils.h"

/*============================================================================
 * 全局变量
 *============================================================================*/

static char g_db_path[256] = "9898.db";
static pthread_mutex_t g_db_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_db_initialized = 0;

/*============================================================================
 * 内部函数
 *============================================================================*/

/**
 * 创建数据库表结构
 */
static int db_create_tables(void) {
    const char *sql = 
        "CREATE TABLE IF NOT EXISTS sms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "sender TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "timestamp INTEGER NOT NULL,"
        "is_read INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS sent_sms ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "recipient TEXT NOT NULL,"
        "content TEXT NOT NULL,"
        "timestamp INTEGER NOT NULL,"
        "status TEXT DEFAULT 'sent'"
        ");"
        "CREATE TABLE IF NOT EXISTS webhook_config ("
        "id INTEGER PRIMARY KEY,"
        "enabled INTEGER DEFAULT 0,"
        "platform TEXT,"
        "url TEXT,"
        "body TEXT,"
        "headers TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS sms_config ("
        "id INTEGER PRIMARY KEY,"
        "max_count INTEGER DEFAULT 50,"
        "max_sent_count INTEGER DEFAULT 10,"
        "sms_fix_enabled INTEGER DEFAULT 0"
        ");"
        "CREATE TABLE IF NOT EXISTS config ("
        "key TEXT PRIMARY KEY,"
        "value TEXT"
        ");"
        "CREATE TABLE IF NOT EXISTS auth_tokens ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "token TEXT UNIQUE NOT NULL,"
        "expire_time INTEGER NOT NULL,"
        "created_at INTEGER NOT NULL"
        ");"
        "CREATE TABLE IF NOT EXISTS automation_rules ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "trigger TEXT NOT NULL,"
        "operator TEXT NOT NULL,"
        "value REAL NOT NULL,"
        "action TEXT NOT NULL,"
        "enabled INTEGER DEFAULT 1"
        ");";
    
    return db_execute(sql);
}

/*============================================================================
 * 公共接口实现
 *============================================================================*/

int db_init(const char *path) {
    if (g_db_initialized) {
        return 0;
    }
    
    if (path && strlen(path) > 0) {
        strncpy(g_db_path, path, sizeof(g_db_path) - 1);
        g_db_path[sizeof(g_db_path) - 1] = '\0';
    }
    
    printf("[DB] 初始化数据库: %s\n", g_db_path);
    
    if (db_create_tables() != 0) {
        printf("[DB] 创建表失败\n");
        return -1;
    }
    
    /* 为旧数据库添加新字段（忽略错误，字段可能已存在） */
    db_execute("ALTER TABLE sms_config ADD COLUMN sms_fix_enabled INTEGER DEFAULT 0;");
    
    g_db_initialized = 1;
    printf("[DB] 数据库初始化完成\n");
    return 0;
}

void db_deinit(void) {
    g_db_initialized = 0;
    printf("[DB] 数据库模块已关闭\n");
}

const char *db_get_path(void) {
    return g_db_path;
}

int db_execute(const char *sql) {
    char output[1024];
    int ret = -1;
    
    if (!sql || strlen(sql) == 0) {
        return -1;
    }
    
    /* 对于长SQL或包含特殊字符的SQL，使用临时文件 */
    size_t sql_len = strlen(sql);
    if (sql_len > 1000 || strchr(sql, '"') || strchr(sql, '\n')) {
        const char *tmp_sql = "/tmp/db_sql.tmp";
        FILE *fp = fopen(tmp_sql, "w");
        if (!fp) {
            printf("[DB] SQL执行失败: 无法创建临时文件\n");
            return -1;
        }
        fputs(sql, fp);
        fclose(fp);
        
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "sqlite3 '%s' < %s", g_db_path, tmp_sql);
        ret = run_command(output, sizeof(output), "sh", "-c", cmd, NULL);
        
        unlink(tmp_sql);
    } else {
        char cmd[4096];
        snprintf(cmd, sizeof(cmd), "sqlite3 '%s' \"%s\"", g_db_path, sql);
        ret = run_command(output, sizeof(output), "sh", "-c", cmd, NULL);
    }
    
    if (ret != 0) {
        printf("[DB] SQL执行失败: %.200s...\n", sql);
        return -1;
    }
    return 0;
}

int db_execute_safe(const char *sql) {
    pthread_mutex_lock(&g_db_mutex);
    int ret = db_execute(sql);
    pthread_mutex_unlock(&g_db_mutex);
    return ret;
}

int db_query_int(const char *sql, int default_val) {
    char cmd[1024];
    char output[256] = {0};
    
    if (!sql || strlen(sql) == 0) {
        return default_val;
    }
    
    snprintf(cmd, sizeof(cmd), "sqlite3 '%s' \"%s\"", g_db_path, sql);
    
    pthread_mutex_lock(&g_db_mutex);
    int ret = run_command(output, sizeof(output), "sh", "-c", cmd, NULL);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (ret != 0 || strlen(output) == 0) {
        return default_val;
    }
    
    /* 去除换行符 */
    size_t len = strlen(output);
    if (len > 0 && output[len-1] == '\n') {
        output[len-1] = '\0';
    }
    
    return atoi(output);
}

int db_query_string(const char *sql, char *buf, size_t size) {
    char cmd[1024];
    
    if (!sql || !buf || size == 0) {
        return -1;
    }
    
    buf[0] = '\0';
    snprintf(cmd, sizeof(cmd), "sqlite3 '%s' \"%s\"", g_db_path, sql);
    
    pthread_mutex_lock(&g_db_mutex);
    int ret = run_command(buf, size, "sh", "-c", cmd, NULL);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (ret != 0) {
        buf[0] = '\0';
        return -1;
    }
    
    /* 去除换行符 */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
    }
    
    return 0;
}

int db_query_rows(const char *sql, const char *separator, char *buf, size_t size) {
    char cmd[2048];
    
    if (!sql || !buf || size == 0) {
        return -1;
    }
    
    buf[0] = '\0';
    
    if (separator && strlen(separator) > 0) {
        snprintf(cmd, sizeof(cmd), "sqlite3 -separator '%s' '%s' \"%s\"", 
                 separator, g_db_path, sql);
    } else {
        snprintf(cmd, sizeof(cmd), "sqlite3 '%s' \"%s\"", g_db_path, sql);
    }
    
    pthread_mutex_lock(&g_db_mutex);
    int ret = run_command(buf, size, "sh", "-c", cmd, NULL);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (ret != 0) {
        buf[0] = '\0';
        return -1;
    }
    
    /* 去除末尾换行符 */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
    }
    
    return 0;
}


/*============================================================================
 * 字符串处理
 *============================================================================*/

void db_escape_string(const char *src, char *dst, size_t size) {
    if (!src || !dst || size == 0) {
        if (dst && size > 0) dst[0] = '\0';
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; src[i] && j < size - 4; i++) {
        switch (src[i]) {
            case '\'':
                dst[j++] = '\'';
                dst[j++] = '\'';
                break;
            case '\n':
                dst[j++] = '\\';
                dst[j++] = 'n';
                break;
            case '\r':
                dst[j++] = '\\';
                dst[j++] = 'r';
                break;
            case '\\':
                dst[j++] = '\\';
                dst[j++] = '\\';
                break;
            default:
                dst[j++] = src[i];
                break;
        }
    }
    dst[j] = '\0';
}

void db_unescape_string(char *str) {
    if (!str) return;
    
    char *src = str;
    char *dst = str;
    
    while (*src) {
        if (*src == '\\' && *(src + 1)) {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 'r': *dst++ = '\r'; break;
                case '\\': *dst++ = '\\'; break;
                default: *dst++ = *src; break;
            }
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

/*============================================================================
 * 配置管理
 *============================================================================*/

int config_get(const char *key, char *value, size_t value_size) {
    char cmd[512];
    
    if (!key || !value || value_size == 0) {
        return -1;
    }
    
    value[0] = '\0';
    snprintf(cmd, sizeof(cmd), 
        "sqlite3 '%s' \"SELECT value FROM config WHERE key='%s';\"", 
        g_db_path, key);
    
    pthread_mutex_lock(&g_db_mutex);
    int ret = run_command(value, value_size, "sh", "-c", cmd, NULL);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (ret != 0 || strlen(value) == 0) {
        value[0] = '\0';
        return -1;
    }
    
    /* 去除换行符 */
    size_t len = strlen(value);
    if (len > 0 && value[len-1] == '\n') {
        value[len-1] = '\0';
    }
    
    return 0;
}

int config_set(const char *key, const char *value) {
    char sql[1024];
    
    if (!key || !value) {
        return -1;
    }
    
    snprintf(sql, sizeof(sql),
        "INSERT OR REPLACE INTO config (key, value) VALUES ('%s', '%s');",
        key, value);
    
    return db_execute_safe(sql);
}

int config_get_int(const char *key, int default_val) {
    char value[64];
    if (config_get(key, value, sizeof(value)) == 0) {
        return atoi(value);
    }
    return default_val;
}

int config_set_int(const char *key, int value) {
    char str[32];
    snprintf(str, sizeof(str), "%d", value);
    return config_set(key, str);
}

long long config_get_ll(const char *key, long long default_val) {
    char value[64];
    if (config_get(key, value, sizeof(value)) == 0) {
        return atoll(value);
    }
    return default_val;
}

int config_set_ll(const char *key, long long value) {
    char str[32];
    snprintf(str, sizeof(str), "%lld", value);
    return config_set(key, str);
}
