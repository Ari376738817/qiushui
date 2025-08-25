#pragma once
#include <string>
#include <functional>
#include <vector>
#include <regex>
#include <map>
#include "server.h"

// 路由结构
struct Route {
    std::string method;
    std::string path;
    std::string pattern;
    std::regex pathRegex;
    std::vector<std::string> paramNames;
    std::function<void(const HttpRequest&, HttpResponse&)> handler;
    
    Route(const std::string& method, const std::string& path, 
          std::function<void(const HttpRequest&, HttpResponse&)> handler);
};

// 路由器类
class Router {
public:
    Router();
    ~Router();
    
    // 添加路由
    void addRoute(const std::string& method, const std::string& path, 
                  std::function<void(const HttpRequest&, HttpResponse&)> handler);
    
    // 路由匹配
    bool route(const std::string& method, const std::string& path, 
               HttpRequest& request, HttpResponse& response);
    
    // 获取所有路由
    const std::vector<Route>& getRoutes() const { return routes_; }
    
private:
    std::vector<Route> routes_;
    
    // 将路径转换为正则表达式
    std::string pathToRegex(const std::string& path, std::vector<std::string>& paramNames);
    
    // 提取路径参数
    bool extractParams(const std::string& path, const Route& route, 
                      std::map<std::string, std::string>& params);
    
    // 匹配路径
    bool matchPath(const std::string& path, const Route& route);
};
