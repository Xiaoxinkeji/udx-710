/**
 * @file sysinfo.h
 * @brief System information (Go: system/sysinfo.go)
 */

#ifndef SYSINFO_H
#define SYSINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/* 系统信息结构 */
typedef struct {
    char hostname[64];
    char sysname[64];
    char release[128];
    char version[256];
    char machine[32];
    unsigned long total_ram;      /* MB */
    unsigned long free_ram;       /* MB */
    unsigned long cached_ram;     /* MB */
    double ram_percent;           /* % */
    double cpu_usage;             /* % */
    double uptime;                /* seconds */
    char bridge_status[32];
    char sim_slot[16];
    char signal_strength[64];
    double thermal_temp;          /* Celsius */
    char power_status[32];
    char battery_health[32];
    unsigned int battery_capacity;
    char ssid[64];
    char passwd[64];
    char select_network_mode[32];
    int is_activated;
    char serial[32];
    char network_mode[32];
    int airplane_mode;
    char imei[20];
    char iccid[24];
    char imsi[20];
    char carrier[32];
    char network_type[16];
    char network_band[16];
    int qci;
    int downlink_rate;
    int uplink_rate;
} SystemInfo;

/**
 * @brief 获取完整系统信息
 * @param info 输出系统信息结构
 * @return 0 成功, -1 失败
 */
int get_system_info(SystemInfo *info);

/**
 * @brief 获取系统运行时间
 * @return 运行时间(秒), -1 失败
 */
double get_uptime(void);

/**
 * @brief 获取设备序列号
 * @param serial 输出缓冲区
 * @param size 缓冲区大小
 * @return 0 成功, -1 失败
 */
int get_serial(char *serial, size_t size);

/**
 * @brief 获取当前 SIM 卡槽
 * @param slot 输出卡槽名 (slot1/slot2)
 * @param ril_path 输出 RIL 路径 (/ril_0 或 /ril_1)
 * @return 0 成功, -1 失败
 */
int get_current_slot(char *slot, char *ril_path);

/**
 * @brief 获取信号强度
 * @param strength 输出缓冲区
 * @param size 缓冲区大小
 * @return 0 成功, -1 失败
 */
int get_signal_strength(char *strength, size_t size);

/**
 * @brief 获取温度
 * @return 温度(摄氏度), -1 失败
 */
double get_thermal_temp(void);

/**
 * @brief 获取 QoS 签约速率
 * @param qci QCI 值
 * @param downlink 下行速率 (kbps)
 * @param uplink 上行速率 (kbps)
 * @return 0 成功, -1 失败
 */
int get_qos_info(int *qci, int *downlink, int *uplink);

/**
 * @brief 获取网络类型和频段
 * @param net_type 网络类型输出
 * @param type_size 缓冲区大小
 * @param band 频段输出
 * @param band_size 缓冲区大小
 * @return 0 成功, -1 失败
 */
int get_network_type_and_band(char *net_type, size_t type_size, char *band, size_t band_size);

/**
 * @brief 获取 CPU 使用率
 * @return CPU 使用率 (%)
 */
double get_cpu_usage(void);

/**
 * @brief 优化系统内存参数 (VM 调优 & OOM 保护)
 */
void system_optimize_memory(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSINFO_H */
