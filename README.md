# API管理系统 - C++版本

一个轻量级、高性能的C++ API管理系统，基于C++17标准库和SQLite3数据库。

## 🚀 特性

- **轻量级设计** - 零外部依赖（除SQLite3外）
- **高性能** - 基于Windows Socket的异步网络处理
- **RESTful API** - 支持GET、POST、PUT、DELETE等HTTP方法
- **路由系统** - 支持路径参数和查询字符串
- **数据库集成** - SQLite3数据库支持
- **配置管理** - JSON配置文件支持
- **控制台界面** - 友好的命令行交互界面

## 📋 系统要求

- Windows 10/11
- Visual Studio 2019+ 或 MinGW-w64
- C++17 兼容的编译器
- SQLite3 开发库

## 🛠️ 安装和编译

### 方法1: 使用CMake（推荐）

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译
cmake --build . --config Release
```

### 方法2: 使用Visual Studio

1. 打开CMakeLists.txt文件
2. Visual Studio会自动配置CMake项目
3. 选择Release配置并编译

### 方法3: 手动编译

```bash
# 使用g++编译（需要安装MinGW-w64）
g++ -std=c++17 -O2 -I./include src/*.cpp -lsqlite3 -lws2_32 -o api_manager.exe
```

## 🚀 运行

### 启动服务器

```bash
# 直接运行
api_manager.exe

# 或者指定配置文件
api_manager.exe --config config.json
```

### 控制台命令

启动后，您可以使用以下控制台命令：

- `help` - 显示帮助信息
- `status` - 显示服务器状态
- `routes` - 显示所有注册的路由
- `clear` - 清屏
- `quit` - 退出程序

## 📡 API接口

### 基础接口

| 方法 | 路径 | 描述 |
|------|------|------|
| GET | `/` | 欢迎页面 |
| GET | `/api/status` | 系统状态 |

### 用户管理接口

| 方法 | 路径 | 描述 |
|------|------|------|
| GET | `/api/users` | 获取用户列表 |
| POST | `/api/users` | 创建新用户 |
| GET | `/api/users/:id` | 获取指定用户 |
| PUT | `/api/users/:id` | 更新指定用户 |
| DELETE | `/api/users/:id` | 删除指定用户 |

### 示例请求

```bash
# 获取用户列表
curl http://127.0.0.1:8080/api/users

# 获取指定用户
curl http://127.0.0.1:8080/api/users/1

# 创建用户
curl -X POST http://127.0.0.1:8080/api/users

# 更新用户
curl -X PUT http://127.0.0.1:8080/api/users/1

# 删除用户
curl -X DELETE http://127.0.0.1:8080/api/users/1
```

## ⚙️ 配置

### 配置文件格式

编辑 `config.json` 文件来自定义配置：

```json
{
    "host": "127.0.0.1",        // 服务器监听地址
    "port": 8080,               // 服务器端口
    "database": "api_manager.db", // 数据库文件路径
    "log_level": "INFO",        // 日志级别
    "max_connections": 100,     // 最大连接数
    "timeout": 30,              // 连接超时时间（秒）
    "cors_enabled": true,       // 是否启用CORS
    "cors_origin": "*",         // CORS允许的源
    "cors_methods": "GET,POST,PUT,DELETE,OPTIONS", // 允许的HTTP方法
    "cors_headers": "Content-Type,Authorization"   // 允许的头部
}
```

## 🗄️ 数据库

系统使用SQLite3数据库，会自动创建以下表：

### users表
- `id` - 用户ID（主键）
- `username` - 用户名
- `email` - 邮箱
- `password_hash` - 密码哈希
- `created_at` - 创建时间
- `updated_at` - 更新时间

### api_logs表
- `id` - 日志ID（主键）
- `method` - HTTP方法
- `path` - 请求路径
- `status_code` - 响应状态码
- `response_time` - 响应时间
- `ip_address` - 客户端IP
- `user_agent` - 用户代理
- `created_at` - 创建时间

### config表
- `key` - 配置键（主键）
- `value` - 配置值
- `description` - 配置描述
- `updated_at` - 更新时间

## 🔧 开发

### 项目结构

```
APIManager/
├── include/           # 头文件
│   ├── server.h      # 服务器类
│   ├── router.h      # 路由器类
│   ├── database.h    # 数据库类
│   └── utils.h       # 工具函数
├── src/              # 源文件
│   ├── main.cpp      # 主程序
│   ├── server.cpp    # 服务器实现
│   ├── router.cpp    # 路由器实现
│   ├── database.cpp  # 数据库实现
│   └── utils.cpp     # 工具函数实现
├── CMakeLists.txt    # CMake构建配置
├── config.json       # 配置文件
└── README.md         # 项目说明
```

### 添加新路由

在 `main.cpp` 中添加新的API路由：

```cpp
// 添加新的GET路由
server->get("/api/items", [](const HttpRequest& req, HttpResponse& res) {
    res.json("{\"items\": []}");
});

// 添加新的POST路由
server->post("/api/items", [](const HttpRequest& req, HttpResponse& res) {
    res.status(201).json("{\"message\": \"Item created\"}");
});
```

### 扩展数据库

在 `database.cpp` 的 `initializeTables()` 方法中添加新表：

```cpp
// 创建新表
std::string createNewTable = R"(
    CREATE TABLE IF NOT EXISTS new_table (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    )
)";

if (!execute(createNewTable)) {
    return false;
}
```

## 🐛 故障排除

### 常见问题

1. **编译错误**: 确保使用C++17兼容的编译器
2. **链接错误**: 确保正确链接SQLite3和ws2_32库
3. **端口占用**: 修改config.json中的端口号
4. **数据库错误**: 检查数据库文件权限和路径

### 调试模式

编译时启用调试信息：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
```

## 📄 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 🤝 贡献

欢迎提交Issue和Pull Request来改进这个项目！

## 📞 支持

如果您遇到问题或有建议，请：

1. 查看本文档的故障排除部分
2. 提交GitHub Issue
3. 联系项目维护者

---

**享受使用C++ API管理系统！** 🎉

