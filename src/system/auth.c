/**
 * @file auth.c
 * @brief 后台认证模块实现 - 支持多Token (CLI 适配版)
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
        srand((unsigned int)(time(NULL) ^ getpid()));
        for (int i = 0; i < 32; i++) {
            random_bytes[i] = (uint8_t)(rand() & 0xFF);
        }
    } else {
        ssize_t n = read(fd, random_bytes, 32);
        close(fd);
        if (n != 32) return -1;
    }
    
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
    
    if (config_get(KEY_PASSWORD_HASH, stored_hash, sizeof(stored_hash)) != 0) {
        return -1;
    }
    
    sha256_hash_string(password, input_hash);
    return (strcmp(stored_hash, input_hash) == 0) ? 0 : -1;
}

/**
 * 清理过期Token
 */
static int cleanup_expired_tokens(void)
{
    char sql[256];
    long long now = (long long)time(NULL);
    snprintf(sql, sizeof(sql), "DELETE FROM auth_tokens WHERE expire_time <= %lld;", now);
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
    
    if (config_get(KEY_PASSWORD_HASH, hash, sizeof(hash)) != 0 || strlen(hash) == 0) {
        sha256_hash_string(AUTH_DEFAULT_PASSWORD, hash);
        config_set(KEY_PASSWORD_HASH, hash);
    }
    
    cleanup_expired_tokens();
    printf("[AUTH] 认证模块初始化完成\n");
    return 0;
}

int auth_login(const char *password, char *token, size_t token_size)
{
    char sql[512];
    long long now, expire_time;
    int count;
    
    if (!password || !token || token_size < AUTH_TOKEN_SIZE) return -2;
    
    if (verify_password(password) != 0) return -1;
    
    cleanup_expired_tokens();
    
    count = get_token_count();
    while (count >= AUTH_MAX_TOKENS && count > 0) {
        delete_oldest_token();
        count--;
    }
    
    if (generate_token(token, token_size) != 0) return -2;
    
    now = (long long)time(NULL);
    expire_time = now + AUTH_TOKEN_EXPIRE_SECONDS;
    
    /* 注意：token 是 hex 格式，不含特殊字符，直接拼接到 SQL 是安全的 */
    snprintf(sql, sizeof(sql),
        "INSERT INTO auth_tokens (token, expire_time, created_at) VALUES ('%s', %lld, %lld);",
        token, expire_time, now);
    
    if (db_execute_safe(sql) != 0) return -2;
    
    return 0;
}

int auth_verify_token(const char *token)
{
    char sql[512];
    long long now;
    char escaped_token[128];
    
    if (!token || strlen(token) == 0) return -1;
    
    now = (long long)time(NULL);
    db_escape_string(token, escaped_token, sizeof(escaped_token));
    
    snprintf(sql, sizeof(sql), 
        "SELECT COUNT(*) FROM auth_tokens WHERE token = '%s' AND expire_time > %lld;", 
        escaped_token, now);
    
    if (db_query_int(sql, 0) > 0) {
        return 0;
    }
    
    cleanup_expired_tokens();
    return -1;
}

int auth_change_password(const char *old_password, const char *new_password)
{
    char new_hash[SHA256_HEX_SIZE] = {0};
    if (!old_password || !new_password) return -2;
    if (strlen(new_password) < 1) return -2;
    
    if (verify_password(old_password) != 0) return -1;
    
    sha256_hash_string(new_password, new_hash);
    if (config_set(KEY_PASSWORD_HASH, new_hash) != 0) return -2;
    
    db_execute_safe("DELETE FROM auth_tokens;");
    return 0;
}

int auth_logout(const char *token)
{
    char sql[256];
    char escaped_token[128];
    
    if (!token || strlen(token) == 0) return -1;
    
    db_escape_string(token, escaped_token, sizeof(escaped_token));
    snprintf(sql, sizeof(sql), "DELETE FROM auth_tokens WHERE token = '%s';", escaped_token);
    db_execute_safe(sql);
    
    return 0;
}

int auth_get_status(int *logged_in)
{
    if (!logged_in) return -1;
    *logged_in = 0;
    cleanup_expired_tokens();
    if (get_token_count() > 0) *logged_in = 1;
    return 0;
}

int auth_is_required(void)
{
    char hash[SHA256_HEX_SIZE] = {0};
    if (config_get(KEY_PASSWORD_HASH, hash, sizeof(hash)) == 0 && strlen(hash) > 0) {
        return 1;
    }
    return 0;
}
