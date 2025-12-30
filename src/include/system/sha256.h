/**
 * @file sha256.h
 * @brief SHA256哈希算法纯C实现
 * @note 基于公共领域实现，无外部依赖
 */

#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SHA256输出长度 */
#define SHA256_BLOCK_SIZE 32  /* 256 bits = 32 bytes */
#define SHA256_HEX_SIZE   65  /* 64 hex chars + null terminator */

/* SHA256上下文结构 */
typedef struct {
    uint8_t  data[64];
    uint32_t datalen;
    uint64_t bitlen;
    uint32_t state[8];
} SHA256_CTX;

/**
 * 初始化SHA256上下文
 * @param ctx SHA256上下文指针
 */
void sha256_init(SHA256_CTX *ctx);

/**
 * 更新SHA256哈希数据
 * @param ctx SHA256上下文指针
 * @param data 输入数据
 * @param len 数据长度
 */
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);

/**
 * 完成SHA256计算并输出哈希值
 * @param ctx SHA256上下文指针
 * @param hash 输出缓冲区（至少32字节）
 */
void sha256_final(SHA256_CTX *ctx, uint8_t *hash);

/**
 * 便捷函数：计算字符串的SHA256哈希并输出hex字符串
 * @param str 输入字符串
 * @param hex_out 输出缓冲区（至少65字节）
 */
void sha256_hash_string(const char *str, char *hex_out);

/**
 * 便捷函数：计算数据的SHA256哈希并输出hex字符串
 * @param data 输入数据
 * @param len 数据长度
 * @param hex_out 输出缓冲区（至少65字节）
 */
void sha256_hash_data(const uint8_t *data, size_t len, char *hex_out);

/**
 * 便捷函数：计算文件的SHA256哈希并输出hex字符串
 * @param path 文件路径
 * @param hex_out 输出缓冲区（至少65字节）
 * @param hex_size 缓冲区大小
 * @return 0 成功，-1 失败
 */
int sha256_file(const char *path, char *hex_out, size_t hex_size);

#ifdef __cplusplus
}
#endif

#endif /* SHA256_H */
