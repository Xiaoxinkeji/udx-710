/**
 * @file auth.c
 * @brief 后台认证模块实现 - 支持多Token
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "auth.h"
#include "sha256.h"
#include "database.h"
#include <sqlite3.h>
#include <pthread.h>

/* 外部函数 - 执行命令 */
extern int run_command(char *output, size_t output_size, const char *cmd, ...);

/* 配置键名 */
#define KEY_PASSWORD_HASH   "auth_password_hash"

/**
 * 生成随机Token
 */
static int generate_token(char *token, size_t size)
{
    if (size < AUTH_TOKEN_SIZE) return -1;
    
    uint8_t random_bytes[32];
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        /* 备用方案：使用时间和进程ID */
        srand((unsigned int)(time(NULL) ^ getpid()));
        for (int i = 0; i < 32; i++) {
            random_bytes[i] = (uint8_t)(rand() & 0xFF);
        }
    } else {
        ssize_t n = read(fd, random_bytes, 32);
        close(fd);
        if (n != 32) {
            return -1;
        }
    }
    
    /* 转换为hex字符串 */
    for (int i = 0; i < 32; i++) {
        sprintf(token + (i * 2), "%02x", random_bytes[i]);
    }
    token[64] = '\0';
    
    return 0;
}

/**
 * 验证密码
 */
static int verify_password(const char *password)
{
    char stored_hash[SHA256_HEX_SIZE] = {0};
    char input_hash[SHA256_HEX_SIZE] = {0};
    
    /* 获取存储的密码哈希 */
    if (config_get(KEY_PASSWORD_HASH, stored_hash, sizeof(stored_hash)) != 0) {
        return -1;  /* 未设置密码 */
    }
    
    /* 计算输入密码的哈希 */
    sha256_hash_string(password, input_hash);
    
    /* 比较哈希值 */
    return (strcmp(stored_hash, input_hash) == 0) ? 0 : -1;
}

/**
 * 清理过期Token
 */
static int cleanup_expired_tokens(void)
{
    char sql[256];
    long long now = (long long)time(NULL);
    
    snprintf(sql, sizeof(sql),
        "DELETE FROM auth_tokens WHERE expire_time <= %lld;", now);
    
    return db_execute_safe(sql);
}

/**
 * 获取当前Token数量
 */
static int get_token_count(void)
{
    return db_query_int("SELECT COUNT(*) FROM auth_tokens;", 0);
}

/**
 * 删除最早的Token
 */
static int delete_oldest_token(void)
{
    return db_execute_safe(
        "DELETE FROM auth_tokens WHERE id = "
        "(SELECT id FROM auth_tokens ORDER BY created_at ASC LIMIT 1);");
}


int auth_init(void)
{
    char hash[SHA256_HEX_SIZE] = {0};
    
    printf("[AUTH] 初始化认证模块\n");
    
    /* 检查是否已设置密码 */
    if (config_get(KEY_PASSWORD_HASH, hash, sizeof(hash)) != 0 || strlen(hash) == 0) {
        /* 设置默认密码 */
        printf("[AUTH] 设置默认密码: %s\n", AUTH_DEFAULT_PASSWORD);
        sha256_hash_string(AUTH_DEFAULT_PASSWORD, hash);
        if (config_set(KEY_PASSWORD_HASH, hash) != 0) {
            printf("[AUTH] 设置默认密码失败\n");
            return -1;
        }
    }
    
    /* 启动时清理过期Token */
    cleanup_expired_tokens();
    
    printf("[AUTH] 认证模块初始化完成\n");
    return 0;
}

int auth_login(const char *password, char *token, size_t token_size)
{
    char sql[512];
    long long now, expire_time;
    int count;
    
    if (!password || !token || token_size < AUTH_TOKEN_SIZE) {
        return -2;
    }
    
    printf("[AUTH] 尝试登录\n");
    
    /* 验证密码 */
    if (verify_password(password) != 0) {
        printf("[AUTH] 密码错误\n");
        return -1;
    }
    
    /* 先清理过期Token */
    cleanup_expired_tokens();
    
    /* 检查Token数量，超过限制则删除最早的 */
    count = get_token_count();
    while (count >= AUTH_MAX_TOKENS && count > 0) {
        printf("[AUTH] Token数量已达上限(%d)，删除最早的Token\n", AUTH_MAX_TOKENS);
        delete_oldest_token();
        count--;
    }
    
    /* 生成新Token */
    if (generate_token(token, token_size) != 0) {
        printf("[AUTH] 生成Token失败\n");
        return -2;
    }
    
    /* 计算过期时间 */
    now = (long long)time(NULL);
    expire_time = now + AUTH_TOKEN_EXPIRE_SECONDS;
    
    /* 插入新Token */
    snprintf(sql, sizeof(sql),
        "INSERT INTO auth_tokens (token, expire_time, created_at) VALUES ('%s', %lld, %lld);",
        token, expire_time, now);
    
    if (db_execute_safe(sql) != 0) {
        printf("[AUTH] 保存Token失败\n");
        return -2;
    }
    
    printf("[AUTH] 登录成功，Token有效期: %d秒，当前Token数: %d\n", 
           AUTH_TOKEN_EXPIRE_SECONDS, get_token_count());
    return 0;
}


int auth_verify_token(const char *token)
{
    char sql[512];
    long long now;
    int count;
    
    if (!token || strlen(token) == 0) {
        return -1;
    }
    
    now = (long long)time(NULL);
    
    /* 使用参数化查询防止 SQL 注入 */
    sqlite3_stmt *stmt;
    const char *sql_query = "SELECT COUNT(*) FROM auth_tokens WHERE token = ? AND expire_time > ?;";
    
    extern sqlite3 *g_db; /* 从 database.c 引用 */
    extern pthread_mutex_t g_db_mutex;

    pthread_mutex_lock(&g_db_mutex);
    int rc = sqlite3_prepare_v2(g_db, sql_query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_bind_int64(stmt, 2, now);
    
    int valid = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (sqlite3_column_int(stmt, 0) > 0) {
            valid = 1;
        }
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    
    if (valid) {
        return 0;  /* Token有效 */
    }
    
    /* Token无效或已过期，尝试清理 */
    cleanup_expired_tokens();
    return -1;
}

int auth_change_password(const char *old_password, const char *new_password)
{
    char new_hash[SHA256_HEX_SIZE] = {0};
    
    if (!old_password || !new_password) {
        return -2;
    }
    
    if (strlen(new_password) < 1) {
        printf("[AUTH] 新密码不能为空\n");
        return -2;
    }
    
    printf("[AUTH] 尝试修改密码\n");
    
    /* 验证旧密码 */
    if (verify_password(old_password) != 0) {
        printf("[AUTH] 旧密码错误\n");
        return -1;
    }
    
    /* 计算新密码哈希 */
    sha256_hash_string(new_password, new_hash);
    
    /* 保存新密码 */
    if (config_set(KEY_PASSWORD_HASH, new_hash) != 0) {
        printf("[AUTH] 保存新密码失败\n");
        return -2;
    }
    
    /* 清除所有Token，强制所有设备重新登录 */
    db_execute_safe("DELETE FROM auth_tokens;");
    
    printf("[AUTH] 密码修改成功，所有设备需重新登录\n");
    return 0;
}

int auth_logout(const char *token)
{
    char sql[256];
    
    if (!token || strlen(token) == 0) {
        return -1;
    }
    
    /* 使用参数化查询防止 SQL 注入 */
    sqlite3_stmt *stmt;
    const char *sql_query = "DELETE FROM auth_tokens WHERE token = ?;";
    
    extern sqlite3 *g_db;
    extern pthread_mutex_t g_db_mutex;

    pthread_mutex_lock(&g_db_mutex);
    int rc = sqlite3_prepare_v2(g_db, sql_query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        pthread_mutex_unlock(&g_db_mutex);
        return -1;
    }
    
    sqlite3_bind_text(stmt, 1, token, -1, SQLITE_STATIC);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_db_mutex);
    
    printf("[AUTH] 登出成功\n");
    return 0;
}

int auth_get_status(int *logged_in)
{
    int count;
    
    if (!logged_in) {
        return -1;
    }
    
    *logged_in = 0;
    
    /* 先清理过期Token */
    cleanup_expired_tokens();
    
    /* 检查是否有有效Token */
    count = get_token_count();
    if (count > 0) {
        *logged_in = 1;
    }
    
    return 0;
}

int auth_is_required(void)
{
    char hash[SHA256_HEX_SIZE] = {0};
    
    /* 如果设置了密码哈希，则需要认证 */
    if (config_get(KEY_PASSWORD_HASH, hash, sizeof(hash)) == 0 && strlen(hash) > 0) {
        return 1;
    }
    
    return 0;
}
