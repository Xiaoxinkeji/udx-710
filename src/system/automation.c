/**
 * @file automation.c
 * @brief 自动化规则引擎实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "automation.h"
#include "database.h"
#include "sysinfo.h"
#include "http_server.h"
#include "exec_utils.h"

static int evaluate_condition(double current, const char *op, double threshold) {
    if (strcmp(op, ">") == 0) return current > threshold;
    if (strcmp(op, "<") == 0) return current < threshold;
    if (strcmp(op, "==") == 0) return current == threshold;
    if (strcmp(op, ">=") == 0) return current >= threshold;
    if (strcmp(op, "<=") == 0) return current <= threshold;
    return 0;
}

void automation_init(void) {
    printf("[AUTO] 自动化引擎初始化\n");
}

int automation_get_rules(AutomationRule *rules, int max_count) {
    char buf[4096];
    if (db_query_rows("SELECT id, name, trigger, operator, value, action, enabled FROM automation_rules", "|", buf, sizeof(buf)) != 0) {
        return 0;
    }

    int count = 0;
    char *line = strtok(buf, "\n");
    while (line && count < max_count) {
        AutomationRule *r = &rules[count];
        sscanf(line, "%d|%63[^|]|%31[^|]|%3[^|]|%lf|%127[^|]|%d", 
               &r->id, r->name, r->trigger, r->operator, &r->value, r->action, &r->enabled);
        line = strtok(NULL, "\n");
        count++;
    }
    return count;
}

void automation_check_cycle(void) {
    AutomationRule rules[MAX_RULES];
    int count = automation_get_rules(rules, MAX_RULES);
    if (count <= 0) return;

    SystemInfo info;
    if (get_system_info(&info) != 0) return;

    for (int i = 0; i < count; i++) {
        if (!rules[i].enabled) continue;

        double current_val = 0;
        int valid_trigger = 0;

        if (strcmp(rules[i].trigger, "temperature") == 0) {
            current_val = info.temperature;
            valid_trigger = 1;
        } else if (strcmp(rules[i].trigger, "uptime") == 0) {
            current_val = info.uptime / 60.0; /* 分钟 */
            valid_trigger = 1;
        } else if (strcmp(rules[i].trigger, "mem_percent") == 0) {
            current_val = info.ram_percent;
            valid_trigger = 1;
        }
        /* TODO: 增加更多触发器，如流量百分比等 */

        if (valid_trigger && evaluate_condition(current_val, rules[i].operator, rules[i].value)) {
            char log_msg[256];
            snprintf(log_msg, sizeof(log_msg), "[AUTO] 规则命中: %s (当前值: %.2f %s %.2f)", 
                   rules[i].name, current_val, rules[i].operator, rules[i].value);
            geek_logger_broadcast(log_msg);
            printf("%s\n", log_msg);
            
            if (strcmp(rules[i].action, "reboot") == 0) {
                geek_logger_broadcast("[AUTO] 执行动作: 重启设备");
                printf("[AUTO] 执行动作: 重启设备\n");
                system("reboot &");
            } else if (g_str_has_prefix(rules[i].action, "shell:")) {
                const char *cmd = rules[i].action + 6;
                char act_msg[256];
                snprintf(act_msg, sizeof(act_msg), "[AUTO] 执行自定义命令: %s", cmd);
                geek_logger_broadcast(act_msg);
                printf("%s\n", act_msg);
                system(cmd);
            } else if (strcmp(rules[i].action, "drop_caches") == 0) {
                geek_logger_broadcast("[AUTO] 执行动作: 释放系统缓存 (Drop Caches)");
                printf("[AUTO] 执行动作: 释放系统缓存\n");
                system("echo 3 > /proc/sys/vm/drop_caches");
            } else if (strcmp(rules[i].action, "compact_memory") == 0) {
                geek_logger_broadcast("[AUTO] 执行动作: 整理内存碎片 (Compact Memory)");
                printf("[AUTO] 执行动作: 整理内存碎片\n");
                system("echo 1 > /proc/sys/vm/compact_memory");
            }
        }
    }
}

int automation_save_rule(AutomationRule *rule) {
    char sql[512];
    if (rule->id <= 0) {
        snprintf(sql, sizeof(sql),
            "INSERT INTO automation_rules (name, trigger, operator, value, action, enabled) VALUES ('%s', '%s', '%s', %f, '%s', %d)",
            rule->name, rule->trigger, rule->operator, rule->value, rule->action, rule->enabled);
    } else {
        snprintf(sql, sizeof(sql),
            "UPDATE automation_rules SET name='%s', trigger='%s', operator='%s', value=%f, action='%s', enabled=%d WHERE id=%d",
            rule->name, rule->trigger, rule->operator, rule->value, rule->action, rule->enabled, rule->id);
    }
    return db_execute_safe(sql);
}

int automation_delete_rule(int id) {
    char sql[64];
    snprintf(sql, sizeof(sql), "DELETE FROM automation_rules WHERE id=%d", id);
    return db_execute_safe(sql);
}
