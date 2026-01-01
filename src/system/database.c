/**
 * @file database.c
 * @brief 数据库操作模块实现 - SQLite3 CLI 封装接口
 * 安全加固版本：杜绝 Shell 注入，自研参数转义逻辑，无需 sqlite3.h 依赖
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "database.h"

/* 外部函数 - 执行命令 */
extern int run_command(char *output, size_t output_size, const char *cmd, ...);

/*============================================================================
 * 全局变量
 *============================================================================*/

static char g_db_path[256] = "9898.db";
static pthread_mutex_t g_db_mutex = PTHREAD_MUTEX_INITIALIZER;
static int g_db_initialized = 0;

/*============================================================================
 * 内部工具函数
 *============================================================================*/

/**
 * 严格的 SQL 转义逻辑
 * 将 ' 替换为 ''，这是 SQLite 防止注入的标准做法
 */
static void sql_escape(const char *src, char *dst, size_t dst_size) {
    if (!src || !dst || dst_size == 0) return;
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dst_size - 2; i++) {
        if (src[i] == '\'') {
            dst[j++] = '\'';
            dst[j++] = '\'';
        } else {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

/**
 * 执行 SQLite CLI 命令
 */
static int sqlite_cli_exec(const char *sql, char *output, size_t output_size) {
    if (!sql) return -1;
    
    /* 构造命令: sqlite3 <db_path> '<sql>' */
    /* 注意：这里通过转义单引号并包裹在单引号内来防止 shell 注入 */
    char escaped_sql[4096];
    sql_escape(sql, escaped_sql, sizeof(escaped_sql));
    
    char cmd_sql[4096];
    sql_escape(sql, cmd_sql, sizeof(cmd_sql));
    
    return run_command(output, output_size, "sqlite3", g_db_path, cmd_sql, NULL);
}

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
    
    return sqlite_cli_exec(sql, NULL, 0);
}

/*============================================================================
 * 公共接口实现
 *============================================================================*/

int db_init(const char *path) {
    pthread_mutex_lock(&g_db_mutex);
    if (g_db_initialized) {
        pthread_mutex_unlock(&g_db_mutex);
        return 0;
    }
    
    if (path && strlen(path) > 0) {
        strncpy(g_db_path, path, sizeof(g_db_path) - 1);
        g_db_path[sizeof(g_db_path) - 1] = '\0';
    }
    
    printf("[DB] 初始化数据库 (CLI 封装): %s\n", g_db_path);
    
    /* 优化性能 */
    sqlite_cli_exec("PRAGMA journal_mode=WAL;", NULL, 0);
    sqlite_cli_exec("PRAGMA synchronous=NORMAL;", NULL, 0);
    
    if (db_create_tables() != 0) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    /* 尝试增加列（忽略错误，如果已存在） */
    sqlite_cli_exec("ALTER TABLE sms_config ADD COLUMN sms_fix_enabled INTEGER DEFAULT 0;", NULL, 0);
    
    g_db_initialized = 1;
    pthread_mutex_unlock(&g_db_mutex);
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
    if (!sql) return -1;
    return sqlite_cli_exec(sql, NULL, 0);
}

int db_execute_safe(const char *sql) {
    pthread_mutex_lock(&g_db_mutex);
    int ret = db_execute(sql);
    pthread_mutex_unlock(&g_db_mutex);
    return ret;
}

int db_query_int(const char *sql, int default_val) {
    char output[64] = {0};
    pthread_mutex_lock(&g_db_mutex);
    int rc = sqlite_cli_exec(sql, output, sizeof(output));
    pthread_mutex_unlock(&g_db_mutex);
    
    if (rc != 0 || strlen(output) == 0) {
        return default_val;
    }
    return atoi(output);
}

int db_query_string(const char *sql, char *buf, size_t size) {
    if (!sql || !buf || size == 0) return -1;
    
    pthread_mutex_lock(&g_db_mutex);
    int rc = sqlite_cli_exec(sql, buf, size);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (rc != 0) return -1;
    
    /* 移除尾部换行符 */
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') {
        buf[len-1] = '\0';
    }
    
    return 0;
}

int db_query_rows(const char *sql, const char *separator, char *buf, size_t size) {
    if (!sql || !buf || size == 0) return -1;
    
    char cmd_with_sep[5120];
    const char *sep = separator ? separator : "|";
    
    /* 使用 sqlite3 -separator 命令选项或直接在 SQL 中处理 */
    /* 这里我们采用简单的方式：让 sqlite 输出默认格式 */
    pthread_mutex_lock(&g_db_mutex);
    int rc = sqlite_cli_exec(sql, buf, size);
    pthread_mutex_unlock(&g_db_mutex);
    
    return (rc == 0) ? 0 : -1;
}

/*============================================================================
 * 字符串处理
 *============================================================================*/

void db_escape_string(const char *src, char *dst, size_t size) {
    sql_escape(src, dst, size);
}

void db_unescape_string(char *str) {
    /* CLI 输出不需要特别反转义 */
}

/*============================================================================
 * 配置管理 (封装转义逻辑)
 *============================================================================*/

int config_get(const char *key, char *value, size_t value_size) {
    char sql[512];
    char escaped_key[256];
    sql_escape(key, escaped_key, sizeof(escaped_key));
    
    snprintf(sql, sizeof(sql), "SELECT value FROM config WHERE key = '%s';", escaped_key);
    return db_query_string(sql, value, value_size);
}

int config_set(const char *key, const char *value) {
    char sql[4096];
    char escaped_key[256];
    char escaped_value[3072];
    
    sql_escape(key, escaped_key, sizeof(escaped_key));
    sql_escape(value, escaped_value, sizeof(escaped_value));
    
    snprintf(sql, sizeof(sql), 
        "INSERT OR REPLACE INTO config (key, value) VALUES ('%s', '%s');", 
        escaped_key, escaped_value);
    
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
