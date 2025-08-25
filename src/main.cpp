#include <iostream>
#include <signal.h>
#include <windows.h>
#include "server.h"
#include "database.h"
#include "utils.h"

// 全局服务器指针
ApiServer* g_server = nullptr;

// 信号处理函数
BOOL WINAPI signalHandler(DWORD signal) {
    if (g_server) {
        std::cout << "\n正在关闭服务器..." << std::endl;
        g_server->stop();
    }
    return TRUE;
}

// 设置控制台标题
void setConsoleTitle() {
    SetConsoleTitle(L"API管理系统 - C++版本");
}

// 显示欢迎信息
void showWelcome() {
    std::cout << "=========================================" << std::endl;
    std::cout << "           API管理系统 v1.0.0" << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << "基于C++17 + SQLite3 + Windows Socket" << std::endl;
    std::cout << "轻量级、高性能、易于部署" << std::endl;
    std::cout << "=========================================" << std::endl;
}

// 显示帮助信息
void showHelp() {
    std::cout << "\n可用命令:" << std::endl;
    std::cout << "  help     - 显示此帮助信息" << std::endl;
    std::cout << "  status   - 显示服务器状态" << std::endl;
    std::cout << "  routes   - 显示所有路由" << std::endl;
    std::cout << "  quit     - 退出程序" << std::endl;
    std::cout << "  clear    - 清屏" << std::endl;
}

// 显示服务器状态
void showStatus(const ApiServer* server) {
    if (!server) {
        std::cout << "服务器未初始化" << std::endl;
        return;
    }
    
    std::cout << "\n服务器状态:" << std::endl;
    std::cout << "  数据库: " << (server->getDatabase() && server->getDatabase()->isConnected() ? "已连接" : "未连接") << std::endl;
    std::cout << "  时间: " << Utils::getCurrentTimestamp() << std::endl;
}

// 显示所有路由
void showRoutes(const ApiServer* server) {
    if (!server) {
        std::cout << "服务器未初始化" << std::endl;
        return;
    }
    
    // 这里可以通过反射或其他方式获取路由信息
    std::cout << "\n已注册的路由:" << std::endl;
    std::cout << "  GET  /                    - 欢迎页面" << std::endl;
    std::cout << "  GET  /api/users           - 获取用户列表" << std::endl;
    std::cout << "  POST /api/users           - 创建新用户" << std::endl;
    std::cout << "  GET  /api/users/:id       - 获取指定用户" << std::endl;
    std::cout << "  PUT  /api/users/:id       - 更新指定用户" << std::endl;
    std::cout << "  DELETE /api/users/:id     - 删除指定用户" << std::endl;
    std::cout << "  GET  /api/status          - 系统状态" << std::endl;
}

// 处理控制台命令
void handleConsoleCommands(ApiServer* server) {
    std::string command;
    
    while (true) {
        std::cout << "\napi> ";
        std::getline(std::cin, command);
        
        if (command == "quit" || command == "exit") {
            break;
        } else if (command == "help") {
            showHelp();
        } else if (command == "status") {
            showStatus(server);
        } else if (command == "routes") {
            showRoutes(server);
        } else if (command == "clear") {
            system("cls");
            showWelcome();
        } else if (!command.empty()) {
            std::cout << "未知命令: " << command << std::endl;
            std::cout << "输入 'help' 查看可用命令" << std::endl;
        }
    }
}

int main() {
    try {
        // 设置控制台
        setConsoleTitle();
        showWelcome();
        
        // 设置信号处理
        SetConsoleCtrlHandler(signalHandler, TRUE);
        
        // 加载配置
        std::string host = "127.0.0.1";
        int port = 8080;
        
        // 检查配置文件
        if (Utils::fileExists("config.json")) {
            std::string configContent = Utils::readFile("config.json");
            auto config = Utils::parseConfigFile(configContent);
            host = Utils::getConfigValue(config, "host", host);
            port = Utils::fromString<int>(Utils::getConfigValue(config, "port", "8080"));
        }
        
        // 创建服务器
        g_server = new ApiServer(host, port);
        
        // 注册API路由
        g_server->get("/", [](const HttpRequest& req, HttpResponse& res) {
            res.json("{\"message\": \"欢迎使用API管理系统\", \"version\": \"1.0.0\", \"timestamp\": \"" + Utils::getCurrentTimestamp() + "\"}");
        });
        
        g_server->get("/api/users", [](const HttpRequest& req, HttpResponse& res) {
            res.json("{\"users\": [{\"id\": 1, \"name\": \"张三\", \"email\": \"zhangsan@example.com\"}, {\"id\": 2, \"name\": \"李四\", \"email\": \"lisi@example.com\"}]}");
        });
        
        g_server->post("/api/users", [](const HttpRequest& req, HttpResponse& res) {
            res.status(201).json("{\"message\": \"用户创建成功\", \"id\": 3, \"timestamp\": \"" + Utils::getCurrentTimestamp() + "\"}");
        });
        
        g_server->get("/api/users/:id", [](const HttpRequest& req, HttpResponse& res) {
            std::string id = req.getParam("id");
            res.json("{\"id\": " + id + ", \"name\": \"用户" + id + "\", \"email\": \"user" + id + "@example.com\"}");
        });
        
        g_server->put("/api/users/:id", [](const HttpRequest& req, HttpResponse& res) {
            std::string id = req.getParam("id");
            res.json("{\"message\": \"用户更新成功\", \"id\": " + id + ", \"timestamp\": \"" + Utils::getCurrentTimestamp() + "\"}");
        });
        
        g_server->del("/api/users/:id", [](const HttpRequest& req, HttpResponse& res) {
            std::string id = req.getParam("id");
            res.json("{\"message\": \"用户删除成功\", \"id\": " + id + ", \"timestamp\": \"" + Utils::getCurrentTimestamp() + "\"}");
        });
        
        g_server->get("/api/status", [](const HttpRequest& req, HttpResponse& res) {
            res.json("{\"status\": \"running\", \"uptime\": \"" + Utils::getCurrentTimestamp() + "\", \"version\": \"1.0.0\"}");
        });
        
        // 启动服务器
        std::cout << "\n正在启动API服务器..." << std::endl;
        std::cout << "服务器地址: http://" << host << ":" << port << std::endl;
        std::cout << "按 Ctrl+C 停止服务器" << std::endl;
        
        // 在新线程中启动服务器
        std::thread serverThread([g_server]() {
            g_server->start();
        });
        
        // 处理控制台命令
        handleConsoleCommands(g_server);
        
        // 停止服务器
        g_server->stop();
        serverThread.join();
        
        delete g_server;
        std::cout << "\n服务器已关闭，再见！" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "\n错误: " << e.what() << std::endl;
        std::cerr << "程序异常退出" << std::endl;
        return 1;
    }
    
    return 0;
}
