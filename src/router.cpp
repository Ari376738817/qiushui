#include "router.h"
#include "utils.h"
#include <iostream>
#include <algorithm>

// Route 构造函数
Route::Route(const std::string& method, const std::string& path, 
             std::function<void(const HttpRequest&, HttpResponse&)> handler)
    : method(method), path(path), handler(handler) {
    
    // 将路径转换为正则表达式模式
    pattern = pathToRegex(path, paramNames);
    try {
        pathRegex = std::regex(pattern);
    } catch (const std::regex_error& e) {
        std::cerr << "正则表达式编译失败: " << e.what() << std::endl;
        // 使用简单的字符串匹配作为后备
        pattern = path;
    }
}

// Router 方法实现
Router::Router() {}

Router::~Router() {}

void Router::addRoute(const std::string& method, const std::string& path, 
                     std::function<void(const HttpRequest&, HttpResponse&)> handler) {
    routes_.emplace_back(method, path, handler);
    std::cout << "注册路由: " << method << " " << path << std::endl;
}

bool Router::route(const std::string& method, const std::string& path, 
                   HttpRequest& request, HttpResponse& response) {
    
    // 查找匹配的路由
    for (const auto& route : routes_) {
        if (route.method == method && matchPath(path, route)) {
            // 提取路径参数
            extractParams(path, route, request.params);
            
            // 调用处理器
            try {
                route.handler(request, response);
                return true;
            } catch (const std::exception& e) {
                std::cerr << "路由处理器异常: " << e.what() << std::endl;
                response.status(500).text("Internal Server Error");
                return true;
            }
        }
    }
    
    return false;
}

std::string Router::pathToRegex(const std::string& path, std::vector<std::string>& paramNames) {
    std::string regex = "^";
    std::string currentParam = "";
    bool inParam = false;
    
    for (size_t i = 0; i < path.length(); ++i) {
        char c = path[i];
        
        if (c == ':') {
            inParam = true;
            currentParam = "";
        } else if (inParam && (c == '/' || c == '?' || c == '#' || c == '&')) {
            // 参数结束
            if (!currentParam.empty()) {
                paramNames.push_back(currentParam);
                regex += "([^/]+)";
            }
            inParam = false;
            regex += std::string(1, c);
        } else if (inParam) {
            currentParam += c;
        } else {
            // 转义特殊字符
            if (c == '.' || c == '^' || c == '$' || c == '*' || c == '+' || c == '?' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|') {
                regex += "\\";
            }
            regex += c;
        }
    }
    
    // 处理最后一个参数
    if (inParam && !currentParam.empty()) {
        paramNames.push_back(currentParam);
        regex += "([^/]+)";
    }
    
    regex += "$";
    return regex;
}

bool Router::extractParams(const std::string& path, const Route& route, 
                          std::map<std::string, std::string>& params) {
    
    if (route.paramNames.empty()) {
        return true;
    }
    
    try {
        std::smatch matches;
        if (std::regex_match(path, matches, route.pathRegex)) {
            // matches[0] 是整个匹配的字符串，matches[1] 开始是捕获组
            for (size_t i = 0; i < route.paramNames.size() && i + 1 < matches.size(); ++i) {
                params[route.paramNames[i]] = matches[i + 1].str();
            }
            return true;
        }
    } catch (const std::regex_error& e) {
        std::cerr << "正则表达式匹配失败: " << e.what() << std::endl;
        return false;
    }
    
    return false;
}

bool Router::matchPath(const std::string& path, const Route& route) {
    // 如果正则表达式编译失败，使用简单的字符串匹配
    if (route.pattern == route.path) {
        return path == route.path;
    }
    
    try {
        return std::regex_match(path, route.pathRegex);
    } catch (const std::regex_error& e) {
        std::cerr << "路径匹配失败: " << e.what() << std::endl;
        return false;
    }
}
