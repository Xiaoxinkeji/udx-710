# 🛠️ UDX710-UOOLS 安装与部署指南

本文档将指导您如何将 **UDX710-UOOLS (极客版)** 部署到您的 MiFi 设备上。

## ⚠️ 风险提示
*   本操作涉及修改系统底层文件，存在一定的变砖风险。
*   请确保您具备基础的 Linux 和 ADB 命令行操作知识。
*   **请务必先备份原机的 `/home/root` 目录及其他重要数据。**

---

## 1. 准备工作

### 硬件
*   基于 展锐 UDX710 平台的设备 (如 SZ50, UZ801 等)
*   Windows/Linux/Mac 电脑
*   USB 数据线 (支持数据传输)

### 软件
*   **ADB 工具包**: 确保电脑已安装 ADB 驱动并配置好环境变量。
*   **Release 包**: 从 [GitHub Releases](https://github.com/LeoChen-CoreMind/UDX710-UOOLS/releases) 下载最新的 `udx710-uools-bundle.zip`。
*   **解压工具**: Bandizip, 7-Zip 或 WinRAR。

---

## 2. 连接设备

1.  开启设备的 **ADB 调试模式**（部分设备需配合特定后台或切卡密码开启）。
2.  将设备通过 USB 连接至电脑。
3.  打开终端 (CMD/PowerShell)，测试连接：

```bash
adb devices
```

如果看到类似 `XXXXXXXX device` 的输出，说明连接成功。

---

## 3. 部署文件

由于板载系统通常缺少 `unzip` 工具，**推荐在电脑端解压后直接推送文件夹**。

### 步骤 3.1: 解压
在电脑上解压 `udx710-uools-bundle.zip`，你会得到一个包含以下内容的文件夹（假设文件夹名为 `bundle`）：
*   `server` (二进制主程序)
*   `dist` (前端资源文件夹)
*   `start.sh` (启动脚本)

### 步骤 3.2: 推送文件
我们将把程序部署到 `/home/root/9898` 目录。

```bash
# 1.获取 Root 权限并挂载分区
adb root
adb remount

# 2. 清理旧版本 (如果是更新安装)
adb shell "rm -rf /home/root/9898"

# 3. 推送文件 (假设解压后的文件夹在当前目录 build_output 下)
# 注意：请根据实际解压路径修改 local_path
adb push ./build_output /home/root/9898

# 4. 修正权限
adb shell "chmod -R 777 /home/root/9898"
```

> **注意**: 如果 `adb push` 报错，可以尝试先推送到 `/tmp` 再移动，或分批推送。

---

## 4. 验证运行

在配置自启前，我们先手动运行一次以确保一切正常。

```bash
adb shell
cd /home/root/9898
./server
```

**预期输出**:
```text
=== ofono-server (C version) ===
[MEM] 正在应用系统级别内存优化...
...
Mongoose web server started on port 9898
```

此时，请在浏览器访问 `http://192.168.100.1:9898` (IP 请根据您设备的实际网关地址调整，通常是 192.168.0.1 或 192.168.100.1)。

如果能看到登录页面并能正常登录（默认密码 `admin`），说明部署成功！按 `Ctrl + C` 停止运行。

---

## 5. 配置开机自启

为了让面板在设备重启后自动运行，我们需要修改 `rc.local` 文件。

### 方法 A: 使用 ADB 修改 (推荐)

```bash
# 1. 拉取 rc.local 到本地
adb pull /etc/rc.local .

# 2. 使用文本编辑器(推荐 VSCode/Notepad++) 打开 rc.local
# 在 "exit 0" 之前添加以下行：
# sh /home/root/9898/start.sh &

# 3. 推送回设备
adb push rc.local /etc/rc.local
adb shell "chmod +x /etc/rc.local"
```

### 方法 B: 使用 vi 编辑器 (如果设备上有)

```bash
adb shell
vi /etc/rc.local
```
在文件末尾（`exit 0`之前）按 `i` 键进入编辑模式，添加：
```bash
sh /home/root/9898/start.sh &
```
按 `Esc`，输入 `:wq` 保存并退出。

---

## 6. 完成

重启设备：
```bash
adb reboot
```

等待设备启动并连接网络后，尝试访问管理面板。

---

## 常见问题 (FAQ)

**Q: `adb remount` 失败？**
A: 请确认设备已 Root。部分设备可能锁定了分区，无法写入 `/home` 以外的目录。本教程默认安装在 `/home/root` 下通常是可写的。

**Q: 运行 `./server` 提示 `Permission denied`？**
A: 请确保执行了 `chmod -R 777 /home/root/9898`。

**Q: 页面显示空白或 404？**
A: 请检查 `/home/root/9898/dist` 目录是否存在且包含前端文件 (`index.html` 等)。

**Q: 无法保存配置 (Database Error)？**
A: 检查 `/home/root/9898` 目录是否可写。数据库文件 `9898.db` 会在此目录生成。
