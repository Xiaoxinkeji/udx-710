#ifndef ACHIEVEMENT_H
#define ACHIEVEMENT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char id[32];
    int achieved;
    int progress;
} Achievement;

/**
 * 初始化成就系统
 */
void achievement_init(void);

/**
 * 获取成就列表
 * @param list 输出数组
 * @param max_count 数组大小
 * @return 成就数量
 */
int achievement_get_list(Achievement *list, int max_count);

/**
 * 更新成就进度
 * @param id 成就ID
 * @param progress 进度(0-100)
 */
void achievement_update(const char *id, int progress);

/**
 * 检查并更新基于系统状态的成就
 */
void achievement_check_system(void);

#ifdef __cplusplus
}
#endif

#endif /* ACHIEVEMENT_H */
