ESP32-S3 智能家居环境监测节点 (Smart Home Sensor Hub)
本项目是一个基于 ESP32-S3 和 ESP-IDF v5.x 框架开发的工业级物联网（IoT）环境监测终端。
项目采用了高内聚、低耦合的模块化四层架构，集成了本地显示、网页配网、传感器数据融合采集以及低功耗电源管理，具备完整的“产品级”雏形。
✨ 核心特性 (Features)🕸️ 
无 App 配网 (SoftAP WebConfig)：内置 HTTP 服务器与前端网页，设备上电自动开启热点，手机浏览器直连即可完成 Wi-Fi 和云端配置，并具有平滑的连接状态反馈机制。
☁️ MQTT 稳定上云：采用 APSTA 混合模式平滑切换网络，连接成功后释放 AP 资源。数据封装为标准 JSON 格式，支持对接阿里云、OneNET 或私有 MQTT Broker。
🌡️ 多传感器并发采集：
DHT11 (温湿度)：加入 3 次容错重试机制，抵抗底层系统时序干扰。
MQ-2 (烟雾/可燃气体)：基于 ESP-IDF v5 最新 adc_oneshot 驱动，内置多次采样抗抖动算法，并将原始模拟量自动换算为直观的 PPM 浓度值。
📺 OLED 本地交互：内嵌自定义 I2C OLED 驱动与多级字库（6x8 / 8x16），支持屏幕自适应排版，实时显示系统启动日志、配网进度与传感器数值。
🔋 工业级电源管理 (低功耗)：开启 FreeRTOS Tickless Idle 与 Auto Light Sleep。
Wi-Fi 强制开启 Modem-sleep 并在配网后优化 DTIM 监听间隔。在保证 5 秒高频上报的同时，大幅降低 CPU 发热与平均功耗。
📂 软件架构 (Architecture)代码采用四层架构，彻底解耦硬件驱动与业务逻辑，极易进行二次开发与移植：
├── main/                   # 系统入口与应用层逻辑
│   ├── main.c              # NVS初始化、电源管理配置及各模块调度
│   ├── app_network.c       # MQTT 链路维护、IP 监听、数据打包上报
│   └── app_sensor.c        # 传感器采集任务与时序调度
├── components/BSP/         # 板级支持包 (硬件驱动层)
│   ├── DHT11/              # 单总线温湿度传感器驱动
│   ├── MQ2/                # ADC 气体传感器驱动及 PPM 换算
│   ├── OLED/               # I2C SSD1306 屏幕驱动与取模字库
│   ├── LED/                # 状态指示灯驱动
│   └── WebConfig/          # SoftAP 热点建网及 WebSocket 网页交互
└── CMakeLists.txt
🔌 硬件接线指南 (Wiring)请根据以下引脚定义连接你的传感器和显示屏：模块设备ESP32-S3 引脚备注说明DHT11 (DATA)GPIO 4建议在 DATA 引脚与 3.3V 间加上 4.7k 上拉电阻MQ-2 (AO)GPIO 5 (ADC1_CH4)模拟输出端，用于采集气体浓度连续变化MQ-2 (VCC)3.3V / 5V注意：5V供电发热量大，若采用电池建议增加 MOS 管控制电源OLED (SDA)GPIO 17I2C 数据线OLED (SCL)GPIO 18I2C 时钟线OLED (VCC)3.3V严禁接 5V，防止烧毁 GPIO复位按键 (可选)EN / RST -> GND连接一个轻触按键到 GND，用于强制物理重启🛠️ 编译与烧录 (Build & Flash)1. 环境准备本项目强依赖于 ESP-IDF v5.x（推荐 v5.0 或更高版本），因为 MQ-2 驱动使用了全新的 esp_adc/adc_oneshot.h API。2. Menuconfig 必选项配置为了使低功耗模块正常工作，请务必在终端执行 idf.py menuconfig 或在 VS Code 中点击齿轮图标，并开启以下选项：Component config -> Power Management -> 勾选 Support for power managementComponent config -> FreeRTOS -> Kernel -> configTICKLESS_IDLE 设置为 Enabled3. 编译与运行idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor  # 将 COMx 替换为你的实际串口号
📱 使用说明 (Usage)设备上电：屏幕将显示 System Boot...，随后开启名为 ESP32_Config 的开放 Wi-Fi 热点。手机配网：手机连接 ESP32_Config。浏览器访问 http://192.168.4.1。在弹出的网页中输入你家中的路由器账号密码并提交。状态监控：OLED 屏幕将依次提示 WiFi Connected! -> MQTT Connected!。随后屏幕开始每 5 秒刷新一次温度、湿度、气体 PPM 浓度及云端在线状态。云端数据：设备将向 MQTT Topic 发送如下格式的 JSON 数据包：{
    "temp": 26,
    "humi": 55,
    "gas_ppm": 125.4
}
🚀 未来规划 (To-Do List)
 NVS 持久化：将配网网页传来的 Wi-Fi 凭证及 MQTT 服务器信息保存至 NVS Flash，实现掉电自动重连。
 SmartConfig 配网：增加微信/ ESP-Touch 兼容的一键配网功能，与 WebConfig 形成双冗余。
硬件电源隔离：增加 GPIO 控制的 MOS 管电路，彻底切断 MQ-2 闲置状态下的供电。If you find this project helpful, please give it a ⭐️!