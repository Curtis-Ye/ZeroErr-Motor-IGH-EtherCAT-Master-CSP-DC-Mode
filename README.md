# ZeroErr-Motor-IGH-EtherCAT-Master-CSP-DC-Mode

基于 IGH EtherCAT Master 的零差云控关节模组 CSP（Cyclic Synchronous Position）模式开发，目标是研究并实现 DC（Distributed Clocks，分布式时钟）同步控制。

## 项目背景

零差云控官方提供了一套 CSV（Cyclic Synchronous Velocity）模式的示例代码。本项目在官方示例基础上进行深入分析与二次开发：
1. **全面解析**官方代码的每一部分（头文件、预定义宏、功能函数、主循环、状态机等）
2. 将 CSV 模式改造为 **CSP 模式**
3. 启用 **DC 分布式时钟**，实现高精度同步控制

## 硬件环境

| 组件                     | 数量 |
| ------------------------ | ---- |
| 零差云控关节模组         | x1   |
| 零差云控 800W 电源       | x1   |
| Ubuntu 系统主机          | x1   |
| RJ45 网线-关节模组转接线 | x1   |

## 软件环境

- **操作系统**: Ubuntu（Linux）
- **EtherCAT 主站**: [IGH EtherCAT Master](https://gitlab.com/etherlab.org/ethercat)（stable-1.5）
- **构建工具**: CMake（>= 3.5）

## 项目结构

```
.
├── README.md
├── src/
│   └── 零差云控官方代码/
│       ├── igh_driver.cpp          # 官方 CSV 模式示例代码
│       ├── CMakeLists.txt          # CMake 构建文件
│       └── 例程解读/               # 代码逐行解析文档
│           ├── 头文件解析.md        # 11 个头文件逐一分析
│           ├── 预定义标识符解析.md   # 宏定义与条件编译说明
│           ├── 功能函数解析.md      # ODwrite / initDrive / timespec 等函数解析
│           ├── 控制字与状态字解析.md # CiA402 状态机与控制字说明
│           ├── 主函数解析1.md       # CPU 亲和性、实时调度、内存锁定
│           ├── 主函数解析2.md       # 主站请求、从站配置、PDO 映射
│           ├── 主函数解析3.md       # Domain 配置、DC 时钟初始化
│           ├── 主函数解析4.md       # 第一个 while(1)：启动至 OP 状态
│           ├── 主函数解析5.md       # 第二个 while(1)：主控制循环
│           └── CMake文件解析.md     # CMakeLists.txt 逐行解释
└── files/
    ├── DC时钟.md                   # 分布式时钟原理讲解
    ├── CPU亲和性.md                # CPU 亲和性与实时调度笔记
    ├── IGH安装.md                  # IGH EtherCAT Master 安装教程
    └── ecrt.h相关函数.md            # ecrt.h头文件相关函数介绍
```

## 关键技术概念

### EtherCAT 通信模型

```
对象字典 (Object Dictionary)
    ↓
PDO Mapping（决定传输哪些对象字典条目）
    ↓
PDO（过程数据对象，周期性实时数据交换）
    ↓
Domain（IGH 中的共享内存缓冲区，映射 PDO 数据）
    ↓
你的应用程序（通过指针直接读写 Domain 内存）
```

- **SDO（Service Data Object）**: 非周期性通信，用于配置参数（如模式切换、报警复位）
- **PDO（Process Data Object）**: 周期性实时通信，用于传输控制指令和反馈数据
- **RxPDO / TxPDO**: 以从站视角定义，RxPDO 为主站→从站，TxPDO 为从站→主站

### DC 分布式时钟

- DC 时钟是一个 64 位纳秒计数器，并非日历时间
- **Sync0**: 主同步脉冲信号，由硬件触发，用于在精确时刻同步所有从站执行指令
- 同步策略有两种：
  - **SYNC_REF_TO_MASTER**: PC 主站时钟为参考（本项目采用）
  - **SYNC_MASTER_TO_REF**: 从站 0 时钟为参考

### CiA402 状态机

| 状态字   | 含义             | 对应控制字            |
| -------- | ---------------- | --------------------- |
| `0x0008` | 故障状态         | `0x0080` 故障复位     |
| `0x0040` | 伺服无故障       | `0x0006` 关闭         |
| `0x0021` | 伺服准备好       | `0x0007` 打开伺服使能 |
| `0x0023` | 等待打开伺服使能 | `0x000F` 伺服运行     |
| `0x0027` | 伺服运行         | `0x000F` 保持运行     |

## 构建与运行

### 1. 安装 IGH EtherCAT Master

详见 [files/IGH安装.md](files/IGH安装.md)

### 2. 启动 EtherCAT 服务

```bash
sudo /etc/init.d/ethercat start
```

### 3. 编译项目

```bash
cd src/零差云控官方代码
mkdir build && cd build
cmake ..
make
```

### 4. 运行

```bash
sudo ./igh_driver
```

按 `Ctrl+C` 退出（程序会先释放主站资源再终止）。

## 参考资源

- [IGH EtherCAT Master 官方仓库](https://gitlab.com/etherlab.org/ethercat)
- [CMake 入门教程](https://subingwen.cn/cmake/CMake-primer/)
- 零差云控关节模组官方手册（对象字典、CiA402 状态机定义）

---

**本项目持续更新中。**
