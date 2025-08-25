#include "server.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// HttpRequest 方法实现
std::string HttpRequest::getQueryParam(const std::string& key) const {
    auto it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

std::string HttpRequest::getHeader(const std::string& key) const {
    auto it = headers.find(key);
    return (it != headers.end()) ? it->second : "";
}

std::string HttpRequest::getParam(const std::string& key) const {
    auto it = params.find(key);
    return (it != params.end()) ? it->second : "";
}

// HttpResponse 方法实现
HttpResponse::HttpResponse() : statusCode(200) {
    headers["Content-Type"] = "text/plain";
    headers["Server"] = "APIManager/1.0";
}

HttpResponse& HttpResponse::status(int code) {
    statusCode = code;
    return *this;
}

HttpResponse& HttpResponse::header(const std::string& key, const std::string& value) {
    headers[key] = value;
    return *this;
}

HttpResponse& HttpResponse::json(const std::string& jsonData) {
    headers["Content-Type"] = "application/json";
    body = jsonData;
    return *this;
}

HttpResponse& HttpResponse::text(const std::string& text) {
    headers["Content-Type"] = "text/plain";
    body = text;
    return *this;
}

std::string HttpResponse::toString() const {
    std::ostringstream oss;
    
    // 状态行
    std::string statusText;
    switch (statusCode) {
        case 200: statusText = "OK"; break;
        case 201: statusText = "Created"; break;
        case 400: statusText = "Bad Request"; break;
        case 404: statusText = "Not Found"; break;
        case 500: statusText = "Internal Server Error"; break;
        default: statusText = "Unknown"; break;
    }
    
    oss << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    
    // 头部
    for (const auto& header : headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    // 内容长度
    oss << "Content-Length: " << body.length() << "\r\n";
    
    // 空行和正文
    oss << "\r\n" << body;
    
    return oss.str();
}

// ApiServer 方法实现
ApiServer::ApiServer(const std::string& host, int port) 
    : host_(host), port_(port), running_(false), serverSocket_(INVALID_SOCKET) {
    router_ = std::make_unique<Router>();
    database_ = std::make_unique<Database>("api_manager.db");
}

ApiServer::~ApiServer() {
    stop();
    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
    }
}

bool ApiServer::initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    return true;
}

void ApiServer::cleanupWinsock() {
    WSACleanup();
}

bool ApiServer::createSocket() {
    serverSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket_ == INVALID_SOCKET) {
        std::cerr << "创建socket失败: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        std::cerr << "设置socket选项失败" << std::endl;
        return false;
    }
    
    // 绑定地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(host_.c_str());
    serverAddr.sin_port = htons(port_);
    
    if (bind(serverSocket_, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "绑定地址失败: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    // 监听连接
    if (listen(serverSocket_, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "监听失败: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    return true;
}

void ApiServer::start() {
    if (running_) return;
    
    // 初始化Winsock
    if (!initializeWinsock()) {
        throw std::runtime_error("Winsock初始化失败");
    }
    
    // 创建socket
    if (!createSocket()) {
        cleanupWinsock();
        throw std::runtime_error("Socket创建失败");
    }
    
    // 连接数据库
    if (!database_->connect()) {
        std::cout << "警告: 数据库连接失败，但服务器将继续运行" << std::endl;
    } else {
        std::cout << "数据库连接成功" << std::endl;
        database_->initializeTables();
    }
    
    running_ = true;
    std::cout << "服务器启动成功，监听地址: " << host_ << ":" << port_ << std::endl;
    
    // 服务器主循环
    serverLoop();
}

void ApiServer::stop() {
    running_ = false;
    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }
    cleanupWinsock();
}

void ApiServer::serverLoop() {
    while (running_) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(serverSocket_, (sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            if (running_) {
                std::cerr << "接受连接失败: " << WSAGetLastError() << std::endl;
            }
            continue;
        }
        
        // 在新线程中处理客户端
        std::thread([this, clientSocket]() {
            handleClient(clientSocket);
        }).detach();
    }
}

void ApiServer::handleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::string requestData(buffer);
        
        // 解析HTTP请求
        HttpRequest request = parseRequest(requestData);
        
        // 创建响应
        HttpResponse response;
        
        // 路由匹配
        if (!router_->route(request.method, request.path, request, response)) {
            response.status(404).text("404 Not Found");
        }
        
        // 发送响应
        sendResponse(clientSocket, response);
    }
    
    closesocket(clientSocket);
}

HttpRequest ApiServer::parseRequest(const std::string& requestData) {
    HttpRequest request;
    
    std::istringstream iss(requestData);
    std::string line;
    
    // 解析请求行
    if (std::getline(iss, line)) {
        std::istringstream lineStream(line);
        lineStream >> request.method;
        
        std::string url;
        lineStream >> url;
        
        parseUrl(url, request.path, request.query);
        request.params = parseQueryString(request.query);
    }
    
    // 解析头部
    while (std::getline(iss, line) && line != "\r" && line != "") {
        if (line.back() == '\r') line.pop_back();
        
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);
            
            // 去除前导空格
            value.erase(0, value.find_first_not_of(" \t"));
            
            request.headers[Utils::toLower(key)] = value;
        }
    }
    
    // 解析正文
    std::string body;
    while (std::getline(iss, line)) {
        body += line + "\n";
    }
    if (!body.empty()) {
        request.body = body.substr(0, body.length() - 1); // 移除最后的换行符
    }
    
    return request;
}

bool ApiServer::sendResponse(SOCKET clientSocket, const HttpResponse& response) {
    std::string responseStr = response.toString();
    int bytesSent = send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
    return bytesSent == responseStr.length();
}

void ApiServer::parseUrl(const std::string& url, std::string& path, std::string& query) {
    size_t queryPos = url.find('?');
    if (queryPos != std::string::npos) {
        path = url.substr(0, queryPos);
        query = url.substr(queryPos + 1);
    } else {
        path = url;
        query = "";
    }
}

std::map<std::string, std::string> ApiServer::parseQueryString(const std::string& query) {
    std::map<std::string, std::string> params;
    
    if (query.empty()) return params;
    
    std::vector<std::string> pairs = Utils::split(query, '&');
    for (const auto& pair : pairs) {
        size_t equalPos = pair.find('=');
        if (equalPos != std::string::npos) {
            std::string key = pair.substr(0, equalPos);
            std::string value = pair.substr(equalPos + 1);
            params[Utils::urlDecode(key)] = Utils::urlDecode(value);
        }
    }
    
    return params;
}

// 路由注册方法
void ApiServer::get(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler) {
    router_->addRoute("GET", path, handler);
}

void ApiServer::post(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler) {
    router_->addRoute("POST", path, handler);
}

void ApiServer::put(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler) {
    router_->addRoute("PUT", path, handler);
}

void ApiServer::del(const std::string& path, std::function<void(const HttpRequest&, HttpResponse&)> handler) {
    router_->addRoute("DELETE", path, handler);
}
