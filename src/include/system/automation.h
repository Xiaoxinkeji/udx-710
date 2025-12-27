/**
 * @file automation.h
 * @brief 轻量级规则引擎 (Automation Rule Engine)
 */

#ifndef AUTOMATION_H
#define AUTOMATION_H

#define MAX_RULES 20

typedef struct {
    int id;
    char name[64];
    char trigger[32];   /* "temperature", "traffic_usage", "uptime", "battery" */
    char operator[4];  /* ">", "<", "==" */
    double value;
    char action[128];  /* "reboot", "reset_network", "shell:xxx" */
    int enabled;
} AutomationRule;

/**
 * 初始化自动化引擎
 */
void automation_init(void);

/**
 * 运行引擎巡检循环 (由主循环或定时器调用)
 */
void automation_check_cycle(void);

/**
 * 获取所有规则
 */
int automation_get_rules(AutomationRule *rules, int max_count);

/**
 * 添加或更新规则
 */
int automation_save_rule(AutomationRule *rule);

/**
 * 删除规则
 */
int automation_delete_rule(int id);

#endif /* AUTOMATION_H */
