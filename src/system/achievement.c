#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "achievement.h"
#include "database.h"
#include "sysinfo.h"

void achievement_init(void) {
    /* 可以在此处插入默认值或进行自检 */
}

void achievement_update(const char *id, int progress) {
    char key_achieved[64], key_progress[64];
    snprintf(key_achieved, sizeof(key_achieved), "ach_%s_status", id);
    snprintf(key_progress, sizeof(key_progress), "ach_%s_progress", id);

    if (progress > 100) progress = 100;
    
    int current = config_get_int(key_progress, 0);
    if (progress > current) {
        config_set_int(key_progress, progress);
    }
    
    if (progress >= 100) {
        config_set_int(key_achieved, 1);
    }
}

void achievement_check_system(void) {
    SystemInfo info;
    if (get_system_info(&info) != 0) return;

    /* 运行时间成就: 1小时 (3600秒) */
    int uptime_prog = (int)((info.uptime / 3600.0) * 100.0);
    achievement_update("uptime_novice", uptime_prog);

    /* 信号猎手成就: 信号强度好于 -80dBm */
    char *dbm_ptr = strstr(info.signal_strength, "-");
    if (dbm_ptr) {
        int dbm = abs(atoi(dbm_ptr));
        if (dbm > 0 && dbm <= 80) {
            achievement_update("signal_hunter", 100);
        } else if (dbm > 0) {
            int sig_prog = (int)(((120 - dbm) / (120 - 80.0)) * 100.0);
            if (sig_prog < 0) sig_prog = 0;
            achievement_update("signal_hunter", sig_prog);
        }
    }
    
    /* 流量成就模拟: 用内存占用率代替，或者后续对接 traffic.c */
    int mem_prog = (int)(((info.total_ram - info.free_ram) / (double)info.total_ram) * 100.0);
    achievement_update("traffic_master", mem_prog);
}

int achievement_get_list(Achievement *list, int max_count) {
    const char *ids[] = {"uptime_novice", "traffic_master", "signal_hunter"};
    int count = 3;
    if (count > max_count) count = max_count;

    for (int i = 0; i < count; i++) {
        char key_achieved[64], key_progress[64];
        snprintf(key_achieved, sizeof(key_achieved), "ach_%s_status", ids[i]);
        snprintf(key_progress, sizeof(key_progress), "ach_%s_progress", ids[i]);

        strncpy(list[i].id, ids[i], sizeof(list[i].id) - 1);
        list[i].achieved = config_get_int(key_achieved, 0);
        list[i].progress = config_get_int(key_progress, 0);
    }

    return count;
}
