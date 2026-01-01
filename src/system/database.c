/**
 * @file database.c
 * @brief 数据库操作模块实现 - SQLite3 C API 统一接口
 * 安全加固版本：杜绝 Shell 注入，支持并发访问
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sqlite3.h>
#include "database.h"

/*============================================================================
 * 全局变量
 *============================================================================*/

static char g_db_path[256] = "9898.db";
static sqlite3 *g_db = NULL;
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
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("[DB] 创建表失败: %s\n", err_msg);
        sqlite3_free(err_msg);
        return -1;
    }
    return 0;
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
    
    printf("[DB] 初始化数据库 (C API): %s\n", g_db_path);
    
    int rc = sqlite3_open(g_db_path, &g_db);
    if (rc != SQLITE_OK) {
        printf("[DB] 无法打开数据库: %s\n", sqlite3_errmsg(g_db));
        sqlite3_close(g_db);
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    /* 优化性能 */
    sqlite3_exec(g_db, "PRAGMA journal_mode=WAL;", NULL, 0, NULL);
    sqlite3_exec(g_db, "PRAGMA synchronous=NORMAL;", NULL, 0, NULL);
    
    if (db_create_tables() != 0) {
        sqlite3_close(g_db);
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    /* 升级逻辑 */
    sqlite3_exec(g_db, "ALTER TABLE sms_config ADD COLUMN sms_fix_enabled INTEGER DEFAULT 0;", NULL, 0, NULL);
    
    g_db_initialized = 1;
    pthread_mutex_unlock(&g_db_mutex);
    printf("[DB] 数据库初始化完成\n");
    return 0;
}

void db_deinit(void) {
    pthread_mutex_lock(&g_db_mutex);
    if (g_db) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
    g_db_initialized = 0;
    pthread_mutex_unlock(&g_db_mutex);
    printf("[DB] 数据库模块已关闭\n");
}

const char *db_get_path(void) {
    return g_db_path;
}

int db_execute(const char *sql) {
    if (!g_db || !sql) return -1;
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_db, sql, NULL, 0, &err_msg);
    if (rc != SQLITE_OK) {
        printf("[DB] SQL执行失败: %s (SQL: %.100s)\n", err_msg, sql);
        sqlite3_free(err_msg);
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
    if (!g_db || !sql) return default_val;
    
    pthread_mutex_lock(&g_db_mutex);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return default_val;
    }
    
    int result = default_val;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    return result;
}

int db_query_string(const char *sql, char *buf, size_t size) {
    if (!g_db || !sql || !buf || size == 0) return -1;
    
    pthread_mutex_lock(&g_db_mutex);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    int ret = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *text = sqlite3_column_text(stmt, 0);
        if (text) {
            strncpy(buf, (const char *)text, size - 1);
            buf[size - 1] = '\0';
            ret = 0;
        }
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    return ret;
}

int db_query_rows(const char *sql, const char *separator, char *buf, size_t size) {
    if (!g_db || !sql || !buf || size == 0) return -1;
    
    pthread_mutex_lock(&g_db_mutex);
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    size_t offset = 0;
    buf[0] = '\0';
    const char *sep = separator ? separator : "|";
    
    while (sqlite3_step(stmt) == SQLITE_ROW && offset < size - 1) {
        int cols = sqlite3_column_count(stmt);
        for (int i = 0; i < cols; i++) {
            const char *val = (const char *)sqlite3_column_text(stmt, i);
            if (!val) val = "";
            
            size_t val_len = strlen(val);
            if (offset + val_len + 2 >= size) break;
            
            strcpy(buf + offset, val);
            offset += val_len;
            
            if (i < cols - 1) {
                strcpy(buf + offset, sep);
                offset += strlen(sep);
            }
        }
        if (offset < size - 1) {
            buf[offset++] = '\n';
        }
    }
    
    if (offset > 0 && buf[offset - 1] == '\n') {
        buf[offset - 1] = '\0';
    } else {
        buf[offset] = '\0';
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    return 0;
}

/*============================================================================
 * 字符串处理
 *============================================================================*/

void db_escape_string(const char *src, char *dst, size_t size) {
    if (!src || !dst || size == 0) return;
    char *escaped = sqlite3_mprintf("%q", src);
    if (escaped) {
        strncpy(dst, escaped, size - 1);
        dst[size - 1] = '\0';
        sqlite3_free(escaped);
    } else {
        dst[0] = '\0';
    }
}

void db_unescape_string(char *str) {
    /* SQLite C API 返回的字符串不需要反转义，除非是自定义编码 */
    // Placeholder - current system doesn't need it if we use C API properly
}

/*============================================================================
 * 配置管理 (使用参数化查询)
 *============================================================================*/

int config_get(const char *key, char *value, size_t value_size) {
    if (!g_db || !key || !value) return -1;
    
    pthread_mutex_lock(&g_db_mutex);
    sqlite3_stmt *stmt;
    const char *sql = "SELECT value FROM config WHERE key = ?;";
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    
    int ret = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *val = sqlite3_column_text(stmt, 0);
        if (val) {
            strncpy(value, (const char *)val, value_size - 1);
            value[value_size - 1] = '\0';
            ret = 0;
        }
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    return ret;
}

int config_set(const char *key, const char *value) {
    if (!g_db || !key || !value) return -1;
    
    pthread_mutex_lock(&g_db_mutex);
    sqlite3_stmt *stmt;
    const char *sql = "INSERT OR REPLACE INTO config (key, value) VALUES (?, ?);";
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, key, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, value, -1, SQLITE_STATIC);
    
    int ret = (sqlite3_step(stmt) == SQLITE_DONE) ? 0 : -1;
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    return ret;
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
