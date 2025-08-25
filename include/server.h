#pragma once
#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "router.h"
#include "database.h"

// 前向声明
class Router;
class Database;

// HTTP请求结构
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::string body;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> params;
    
    // 获取查询参数
    std::string getQueryParam(const std::string& key) const;
    
    // 获取头部信息
    std::string getHeader(const std::string& key) const;
    
    // 获取路径参数
    std::string getParam(const std::string& key) const;
};

// HTTP响应结构
struct HttpResponse {
    int statusCode;
    std::map<std::string, std::string> headers;
    std::string body;
    
    HttpResponse();
    
    // 设置状态码
    HttpResponse& status(int code);
    
    // 设置头部
    HttpResponse& header(const std::string& key, const std::string& value);
    
    // 设置JSON响应
    HttpResponse& json(const std::string& jsonData);
    
    // 设置文本响应
    HttpResponse& text(const std::string& text);
    
    // 转换为HTTP响应字符串
    std::string toString() const;
};

// API服务器类
class ApiServer {
public:
    ApiServer(const std::string& host = "127.0.0.1", int port = 8080);
    ~ApiServer();
    
    // 启动服务器
    void start();
    
    // 停止服务器
    void stop();
    
    // 路由注册
    void get(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler);
    void post(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler);
    void put(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler);
    void del(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler);
    
    // 获取数据库实例
    Database* getDatabase() const { return database_.get(); }
    
private:
    std::string host_;
    int port_;
    std::atomic<bool> running_;
    std::unique_ptr<Router> router_;
    std::unique_ptr<Database> database_;
    std::thread serverThread_;
    SOCKET serverSocket_;
    
    // 初始化Winsock
    bool initializeWinsock();
    
    // 清理Winsock
    void cleanupWinsock();
    
    // 创建服务器socket
    bool createSocket();
    
    // 服务器主循环
    void serverLoop();
    
    // 处理客户端连接
    void handleClient(SOCKET clientSocket);
    
    // 解析HTTP请求
    HttpRequest parseRequest(const std::string& requestData);
    
    // 发送HTTP响应
    bool sendResponse(SOCKET clientSocket, const HttpResponse& response);
    
    // 解析URL
    void parseUrl(const std::string& url, std::string& path, std::string& query);
    
    // 解析查询字符串
    std::map<std::string, std::string> parseQueryString(const std::string& query);
};
