# myNetworkDisk

## 1. 项目概述

本项目是一个基于 **Qt/C++** 的桌面端「网络盘」应用：客户端通过 **TCP** 与自建服务端通信，完成用户注册与登录、好友关系（搜索、添加、删除、在线状态）、**私聊与群聊**、以及个人网盘目录下的 **文件与文件夹管理**（浏览、新建、重命名、删除、上传、下载）。此外支持将服务器上已存储的文件 **分享给好友**，由对方确认后服务端在磁盘上复制到其用户目录。

**主要特性：**

- 自定义二进制 **PDU** 协议，统一承载控制消息与可变长负载；上传/下载时通过「传输数据模式」与 PDU 分帧配合，减轻粘包对文件流的影响。
- 服务端为 **单进程多连接**：`MyTcpServer` 监听并接受连接，每个客户端对应一个 `MyTcpSocket`；需要点对点转发的能力（加好友、私聊、分享邀请、群聊广播等）通过 `forwardPDU` 按用户名查找目标连接。
- 用户数据落在 **MySQL**（账号、在线状态、好友关系）；每个用户在服务器本地磁盘下有独立根目录（见 `MyTcpSocket` 中 `ROOT_PATH`），网盘内容以文件系统形式存储。
- 客户端采用多处 **单例**（`TcpClient`、`mainMenu`、`shareFile` 等），便于登录后主界面、子窗口与全局 `QTcpSocket` 协同。

## 2. 技术栈

| 类别       | 技术                                                         |
| ---------- | ------------------------------------------------------------ |
| 客户端界面 | Qt Widgets（`.ui` 界面、`QListWidget`、`QStackedWidget` 等） |
| 网络通信   | Qt Network（`QTcpSocket` / `QTcpServer`），自定义 PDU        |
| 服务端逻辑 | Qt 事件循环 + 每连接一个 `MyTcpSocket` 处理业务              |
| 数据库     | MySQL，Qt SQL 模块（`QMYSQL` 驱动、`QSqlQuery`）             |
| 构建与语言 | qmake（`.pro`）、C++11                                       |

## 3. 项目架构

**整体结构：** 桌面客户端（`TcpClient` 工程）与桌面服务端（`TcpServer` 工程）分离部署；二者通过 **IP/端口**（嵌入资源的 `client.config` / `server.config`）建立 TCP 连接，报文格式由共享思路的 `protocol.h`（两端各有一份，内容对齐）定义。

**数据流概要：**

1. **账号与好友：** 客户端发送 `ENUM_MSG_TYPE_*` 请求 PDU；服务端在 `MyTcpSocket::recvMsg` 中解析类型并分派到 `handle*Request`；涉及库表的操作经 `OperateDB` 访问 MySQL；需要通知另一用户时调用 `MyTcpServer::forwardPDU` 写出同一套 PDU。
2. **网盘文件：** 路径以登录成功后服务端下发的用户根目录为基准（见登录回复中 `caMsg`）；列表、进入目录、返回上级等通过字符串路径在 PDU 的 `caData`/`caMsg` 中传递；实际上传下载在协商后进入 **纯数据收发阶段**（服务端 `m_isTransferData` 与按字节计量读取），避免与后续控制 PDU 混淆。
3. **分享：** 发送方提交好友名与服务器端源路径；服务端向好友发 `SHARE_FILE_INVITE`；接收方接受后服务端 **递归复制** 文件或目录到接收方目录（`copyRecursively`），并向双方返回 `SHARE_FILE_RESULT`。

**模块关系（概念）：**

```
[ TcpClient UI: tcpclient / mainmenu / privatechat / sharefile ]
        |  QTcpSocket（TcpClient 单例）
        v
[ TCP ]
        ^
        |  每连接 MyTcpSocket
[ MyTcpServer 单例 ] ---> [ OperateDB 单例 ] ---> [ MySQL ]
                    ---> [ 本地目录 ROOT_PATH/用户名 ]
```

## 4. 目录结构

**源代码与配置**

```
myNetworkDisk/
├── README.md                 # 项目说明
├── creatDatabase.sql         # MySQL 库表结构（userinfo、friendinfo）
├── TcpServer/                # 服务端工程
│   ├── TcpServer.pro         # qmake 工程：core/gui/network/sql
│   ├── main.cpp              # 入口：QApplication、TcpServer 窗口
│   ├── tcpserver.* / tcpserver.ui
│   ├── mytcpserver.*         # QTcpServer 子类单例，incomingConnection、forwardPDU
│   ├── mytcpsocket.*         # 每客户端连接的业务处理（PDU 分派、文件与聊天等）
│   ├── operatedb.*           # MySQL 单例封装
│   ├── protocol.*            # PDU、消息枚举、makePDU
│   ├── config.qrc / server.config   # 监听地址与端口（资源嵌入）
│   └── ...
└── TcpClient/                # 客户端工程
    ├── TcpClient.pro
    ├── main.cpp              # 入口：TcpClient 单例 show
    ├── tcpclient.* / tcpclient.ui   # 登录/注册界面与 socket、PDU 收发分派
    ├── mainmenu.* / mainmenu.ui     # 主界面：好友、群聊、文件列表与操作
    ├── privatechat.* / privatechat.ui   # 私聊窗口
    ├── sharefile.* / sharefile.ui       # 分享给好友的勾选与确认
    ├── protocol.*
    ├── config.qrc / client.config     # 服务器地址与端口
    ├── icons.qrc、icons/              # 文件列表文件夹/文件图标
    └── ...
```

## 5. 核心文件说明

### 5.1 入口与配置

| 文件                                                 | 作用                                                         |
| ---------------------------------------------------- | ------------------------------------------------------------ |
| `TcpServer/main.cpp`                                 | 启动 GUI 应用并显示 `TcpServer`；`main` 内对 `OperateDB::getInstance().init()` 的调用被注释，数据库实际在 `OperateDB` 单例构造时完成连接参数与 `open`（首次使用单例时生效）。 |
| `TcpClient/main.cpp`                                 | 以 `TcpClient::getInstance().show()` 启动登录窗口（注释说明单例模式）。 |
| `TcpServer/server.config`、`TcpClient/client.config` | 文本形式两行：IP 与端口；由各自 `config.qrc` 打包为 `:/server.config`、`:/client.config`，供 `TcpServer::loadConfig` / `TcpClient::loadConfig` 读取。 |
| `TcpServer/TcpServer.pro`、`TcpClient/TcpClient.pro` | 声明模块（`network`、`sql`）、源文件与资源。                 |

### 5.2 协议与数据模型

| 文件                   | 作用                                                         |
| ---------------------- | ------------------------------------------------------------ |
| `protocol.h`（两端）   | 定义 `ENUM_MSG_TYPE`（注册、登录、好友、聊天、文件、分享等）、结果字符串宏、`FileInfo`、`PDU`（定长头 + 柔性数组 `caMsg`）及 `makePDU`。 |
| `protocol.cpp`（两端） | `makePDU`：按 `sizeof(PDU)+uiMsgLen` 分配并清零，设置 `uiPDULen`/`uiMsgLen`。 |
| `creatDatabase.sql`    | 创建库 `networkdiskdb` 中表 `userinfo`（用户名、密码、在线标记）、`friendinfo`（用户与好友 id 外键）。 |

### 5.3 服务端核心业务

| 文件                | 作用                                                         |
| ------------------- | ------------------------------------------------------------ |
| `mytcpserver.h/cpp` | 单例 `MyTcpServer`：`incomingConnection` 创建 `MyTcpSocket` 并加入列表；`forwardPDU` 按登录用户名向对应 socket 写 PDU；槽 `deleteSocket` 清理断开连接。 |
| `mytcpsocket.h/cpp` | 连接级逻辑：`recvMsg` 先处理上传文件流（`m_isTransferData`），再按 `uiPDULen` 读满 PDU 并 `switch(uiMsgType)`；各 `handle*` 实现注册、登录、好友、私聊/群聊转发、目录与文件 CRUD、上传下载数据、分享邀请/接受/拒绝及磁盘复制。`ROOT_PATH` 为用户文件存储根路径（需与部署环境一致）。 |
| `operatedb.h/cpp`   | 单例：用户增删查、在线状态、在线用户列表、好友查询与维护等 SQL；构造函数中配置 MySQL 连接参数（主机、端口、库名、用户名密码在源码中，部署时需自行修改）。 |

### 5.4 客户端核心业务

| 文件                | 作用                                                         |
| ------------------- | ------------------------------------------------------------ |
| `tcpclient.h/cpp`   | 单例：加载配置、连接服务器、`recvMsg` 与服务端对称地按消息类型分发到 `handle*Respond`/`handle*Request`；维护当前网盘路径 `m_myCurPath`、下载缓冲等；登录成功后打开 `mainMenu` 并隐藏自身。 |
| `mainmenu.h/cpp`    | 主界面：在线用户、好友、群聊区、文件列表；按钮触发各类 PDU 发送；使用 `QTimer::singleShot` 错开多个初始请求以降低粘包风险；管理 `privateChat` 窗口映射与文件上传/下载路径。 |
| `privatechat.h/cpp` | 双人私聊界面，通过协议与 socket 发送私聊内容（具体字段与 `ENUM_MSG_TYPE_PRIVATE_CHAT_*` 一致）。 |
| `sharefile.h/cpp`   | 单例：选择好友、确认后将分享源路径等发往服务器以触发分享流程。 |

### 5.5 部署与扩展

- **MySQL**：需预先执行 `creatDatabase.sql`，并保证 `operatedb.cpp` 中的连接信息与运行环境一致。
- **服务端用户目录**：注册成功会在 `ROOT_PATH` 下创建用户名子目录；登录成功回复中的路径需与客户端后续操作一致。
- **路径常量**：`ROOT_PATH` 当前为源码中的绝对路径宏，换机器部署时必须修改并重新编译，或改为可配置项。
- **两端 protocol**：新增消息类型时需同步修改客户端与服务端的 `protocol.h` 及各自处理分支，避免不一致。