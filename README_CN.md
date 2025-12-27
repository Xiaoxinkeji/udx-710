# 5G MiFi 管理面板(UDX710)

基于Web的5G MiFi设备管理界面，运行于嵌入式Linux系统（aarch64）。

> ⭐ **如果觉得这个项目有用，请点个Star支持一下！** 辛苦肝了一周的后台，您的支持是我最大的动力！

## 📦 版本说明

本项目提供两个版本，满足不同设备需求：

| 版本类型 | 适用设备 | Git分支 | 功能支持 | 说明 |
|:---:|:---:|:---:|:---:|:---|
| **UDX710 通用版** | 展锐UDX710平台通用 | `main` | ⭐ 基础功能集 | 适用于大多数UDX710设备 |
| **SZ50 专用版** | SZ50随身WiFi | `SZ50` | 🌟 全功能支持 | 额外支持：LED灯控制、按键监听、WiFi控制、恢复出厂设置、设备接入管理 |

> 💡 **切换版本**: `git checkout SZ50` 切换到SZ50专用版，`git checkout main` 切换到通用版

### 📥 软件下载

| 版本 | 下载链接 |
|:---:|:---:|
| **UDX710 通用版** | [📥 点击下载](https://github.com/LeoChen-CoreMind/UDX710-UOOLS/releases/latest) |
| **SZ50 专用版** | [📥 点击下载](https://github.com/LeoChen-CoreMind/UDX710-UOOLS/releases/latest) |

### SZ50专用版额外功能
- 🔆 **LED灯控制** - 自定义LED指示灯状态
- 🔘 **按键监听** - 物理按键事件响应
- 📶 **WiFi控制** - 完整的WiFi AP管理
- 🔄 **恢复出厂设置** - 一键恢复默认配置
- 👥 **设备接入管理** - 管理连接的客户端设备

## ✨ 性能亮点

| 指标 | 本项目 | 传统方案 (8080) |
|------|--------|----------------|
| **打包体积** | ~200 KB | ~6 MB |
| **内存占用** (运行7小时) | ~1 MB | 高得多 |

轻量、高效，完美适配资源受限的嵌入式设备！

## 📸 界面预览

| 系统监控 | 网络管理 | 高级网络 |
|:---:|:---:|:---:|
| <img src="docs/screenshot1.png" width="250" /> | <img src="docs/screenshot2.png" width="250" /> | <img src="docs/screenshot3.png" width="250" /> |

| 短信管理 | 流量统计 | 充电控制 |
|:---:|:---:|:---:|
| <img src="docs/screenshot5.png" width="250" /> | <img src="docs/screenshot6.png" width="250" /> | <img src="docs/screenshot7.png" width="250" /> |

| 系统更新 | AT调试 | Web终端 |
|:---:|:---:|:---:|
| <img src="docs/screenshot8.png" width="250" /> | <img src="docs/screenshot9.png" width="250" /> | <img src="docs/screenshot10.png" width="250" /> |

| USB模式 | 系统设置 |
|:---:|:---:|
| <img src="docs/screenshot11.png" width="250" /> | <img src="docs/screenshot12.png" width="250" /> |

| APN设置 | 插件商城 |
|:---:|:---:|
| <img src="docs/screenshot13.png" width="250" /> | <img src="docs/screenshot14.png" width="250" /> |

## 功能特性

### 网络管理
- **Modem控制**：查看IMEI、ICCID、运营商信息、信号强度
- **频段信息**：实时显示网络类型、频段、ARFCN、PCI、RSRP、RSRQ、SINR
- **小区管理**：查看和管理蜂窝网络连接
- **流量统计**：通过vnstat集成监控数据使用量
- **流量控制**：设置流量限制和自动断网

### WiFi管理
- **AP模式**：配置WiFi热点（SSID、密码、信道）
- **客户端管理**：查看已连接设备、踢出客户端
- **DHCP设置**：配置IP范围和租约时间

### 系统功能
- **系统监控**：CPU、内存、温度监控
- **短信管理**：收发短信
- **LED控制**：管理设备LED指示灯
- **飞行模式**：切换飞行模式
- **电源管理**：电池状态、充电控制
- **USB模式切换**：在CDC-ECM、CDC-NCM、RNDIS三种USB网络模式间切换
  - 临时模式：重启后生效，再次重启恢复默认
  - 永久模式：永久保存，所有重启后都生效
- **APN设置**：自定义APN接入点配置
  - 预设运营商配置（中国移动/联通/电信）
  - 自定义APN、用户名、密码
  - 支持多种认证协议（PAP/CHAP）
- **插件商城**：可扩展的插件系统
  - 支持自定义JS+HTML插件
  - 内置Shell脚本执行API
  - 脚本管理（上传/编辑/删除）
  - 插件导入/导出功能
- **OTA更新**：空中固件升级
- **恢复出厂**：恢复设备默认设置
- **Web终端**：远程Shell访问
- **AT调试**：直接AT命令接口

### UI特性
- **深色模式**：完整的深色/浅色主题支持
- **响应式设计**：移动端和桌面端优化
- **实时更新**：数据实时刷新
- **极客特性**：
  - 🏆 **全栈成就系统**：基于C后端的定时审计逻辑，Vue前端毛玻璃动效展示。
  - 📡 **蜂窝拓扑图**：SVG雷达图动态展示邻区基站博弈状态。
  - 🤖 **自动化流引擎**：轻量级IF-THEN规则，支持温控自愈、内存回收。
  - 💻 **极客日志推流**：WebSocket准秒级实时审计日志终端。
  - 🛡️ **内存守护者**：内核级VM调优、核心进程OOM保护，专为低内存设备打造。
- **中文界面**：原生中文语言支持

### 安全特性
- **后台认证**：密码保护的管理界面
  - 默认密码：`admin`（首次登录后建议修改）
  - Token认证机制，支持自动过期
  - 记住密码功能
  - 修改密码支持

## 项目架构

```
├── src/                    # 后端 (C语言)
│   ├── main.c              # 入口点
│   ├── mongoose.c/h        # HTTP服务器 (Mongoose)
│   ├── packed_fs.c         # 嵌入式静态文件
│   ├── handlers/           # HTTP API处理器
│   │   ├── http_server.c   # 路由定义
│   │   └── handlers.c      # API实现
│   └── system/             # 系统模块
│       ├── sysinfo.c       # 系统信息
│       ├── wifi.c          # WiFi控制
│       ├── sms.c           # 短信管理
│       ├── traffic.c       # 流量统计
│       ├── modem.c         # Modem控制
│       ├── ofono.c         # oFono D-Bus集成
│       ├── led.c           # LED控制
│       ├── charge.c        # 电池管理
│       ├── airplane.c      # 飞行模式
│       ├── usb_mode.c      # USB模式切换
│       ├── plugin.c        # 插件系统
│       ├── update.c        # OTA更新
│       ├── factory_reset.c # 恢复出厂
│       └── ...
└── web/                    # 前端 (Vue 3)
    ├── src/
    │   ├── App.vue         # 主应用
    │   ├── components/     # Vue组件
    │   ├── composables/    # Vue组合式函数
    │   └── plugins/        # 插件 (FontAwesome)
    ├── index.html
    ├── package.json
    ├── vite.config.js
    └── tailwind.config.js
```

## 环境要求

### 后端
- GCC交叉编译器 (aarch64-linux-gnu)
- GLib 2.0 (D-Bus支持)
- 目标平台：Linux aarch64（嵌入式设备）

### 前端
- Node.js 18+
- npm 或 yarn

## 编译说明

### 前端编译
```bash
cd web
npm install
npm run build
```

### 后端编译
```bash
# 将前端打包到C源码
cd src
# 从 web/dist 生成 packed_fs.c

# 交叉编译到aarch64
make
```

### Makefile配置
后端使用交叉编译，目标平台为aarch64-linux-gnu。请确保工具链正确配置。

## API接口

| 接口 | 方法 | 描述 |
|------|------|------|
| `/api/sysinfo` | GET | 系统信息 |
| `/api/wifi/config` | GET/POST | WiFi配置 |
| `/api/wifi/clients` | GET | 已连接客户端 |
| `/api/sms/list` | GET | 短信列表 |
| `/api/sms/send` | POST | 发送短信 |
| `/api/traffic/stats` | GET | 流量统计 |
| `/api/traffic/limit` | POST | 设置流量限制 |
| `/api/modem/info` | GET | Modem信息 |
| `/api/band/current` | GET | 当前频段信息 |
| `/api/led/status` | GET/POST | LED控制 |
| `/api/airplane` | GET/POST | 飞行模式 |
| `/api/usb/mode` | GET/POST | USB模式切换 (CDC-ECM/CDC-NCM/RNDIS) |
| `/api/apn` | GET/POST | APN配置管理 |
| `/api/plugins` | GET/POST/DELETE | 插件管理 |
| `/api/scripts` | GET/POST/PUT/DELETE | 脚本管理 |
| `/api/shell` | POST | 执行Shell命令 |
| `/api/update/check` | GET | 检查更新 |
| `/api/update/install` | POST | 安装更新 |
| `/api/factory-reset` | POST | 恢复出厂设置 |
| `/api/reboot` | POST | 重启设备 |
| `/api/achievements` | GET | 获取成就进度 |
| `/api/automation/rules` | GET | 获取自动化规则 |
| `/api/automation/save` | POST | 保存/修改规则 |
| `/api/automation/delete` | POST | 删除规则 |
| `/api/ws/log` | WS | 极客日志实时流 |

## 依赖库

### 后端依赖
- [Mongoose](https://github.com/cesanta/mongoose) - 嵌入式HTTP服务器
- GLib/GIO - 与oFono的D-Bus通信

### 前端依赖
- Vue 3 - UI框架
- Vite - 构建工具
- TailwindCSS - 样式框架
- FontAwesome - 图标库

## 🌐 远程管理与网页控制

内置轻量级 Web Server，可通过浏览器访问控制界面。

**支持功能**：设备状态卡片、实时性能监控、网络控制与调试

| 版本 | 默认访问地址 |
|:---:|:---|
| UDX710 通用版 | `http://设备IP:9898` |
| SZ50 专用版 | `http://设备IP:80` |

```bash
# 启动程序（默认端口）
./server

# 自定义端口启动
./server 80
```

## 📜 开源协议

本项目采用 **GPLv3** 协议，这是强 Copyleft 协议：

| ✅ 允许 | ⚠️ 必须 | ❌ 禁止 |
|:---|:---|:---|
| 自由使用、修改、分发 | 保留版权声明 | 闭源商业化 |
| 分发修改版本 | 公开源代码（分发时） | 删除版权信息 |
| | 使用相同协议 | 更改为其他协议 |

详见 [LICENSE](LICENSE)

## 🙏 致谢

在此感谢以下贡献者对本项目的支持：

| 贡献者 | 贡献内容 |
|:---:|:---|
| **等不住** | 提供各种AT指令 |
| **黑衣剑士** | 提供USB模式切换 |
| **Voodoo** | Glib编译环境 |
| **1orz** | [project-cpe](https://github.com/1orz/project-cpe) 开源项目 |
| **LeoChen** | 项目作者 |

感谢各位网友的支持与反馈！

## ☕ 支持项目

本项目完全开源免费，如果你喜欢这个项目的话，也可以请我喝一杯咖啡~

| 支付宝 | 微信赞赏 | QQ群 |
|:---:|:---:|:---:|
| <img src="docs/alipay.png" width="200" /> | <img src="docs/wechat.png" width="200" /> | <img src="docs/qq_group.png" width="200" /> |

## 💬 社区讨论

欢迎加入群聊一起讨论！

- **QQ群**: 1029148488

欢迎提交 Issue / Pull Request 一起完善项目 💡
