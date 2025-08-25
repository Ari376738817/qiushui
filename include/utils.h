#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

namespace Utils {
    
    // 字符串操作
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string trim(const std::string& str);
    std::string toLower(const std::string& str);
    std::string toUpper(const std::string& str);
    bool startsWith(const std::string& str, const std::string& prefix);
    bool endsWith(const std::string& str, const std::string& suffix);
    
    // URL处理
    std::string urlDecode(const std::string& str);
    std::string urlEncode(const std::string& str);
    
    // JSON处理
    std::string escapeJsonString(const std::string& str);
    std::string createJsonObject(const std::map<std::string, std::string>& data);
    std::string createJsonArray(const std::vector<std::string>& data);
    
    // 时间处理
    std::string getCurrentTimestamp();
    std::string formatTimestamp(long long timestamp);
    long long getCurrentTimeMillis();
    
    // 随机数生成
    std::string generateUUID();
    int randomInt(int min, int max);
    std::string randomString(int length);
    
    // 文件操作
    bool fileExists(const std::string& filename);
    std::string readFile(const std::string& filename);
    bool writeFile(const std::string& filename, const std::string& content);
    bool createDirectory(const std::string& path);
    
    // 网络相关
    std::string getLocalIP();
    bool isValidIP(const std::string& ip);
    int parsePort(const std::string& portStr);
    
    // 数据转换
    template<typename T>
    std::string toString(T value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    
    template<typename T>
    T fromString(const std::string& str) {
        std::istringstream iss(str);
        T value;
        iss >> value;
        return value;
    }
    
    // 特殊化字符串转换
    template<>
    std::string fromString<std::string>(const std::string& str);
    
    // 日志格式化
    std::string formatLogMessage(const std::string& level, const std::string& message, 
                                const std::string& source = "");
    
    // 配置解析
    std::map<std::string, std::string> parseConfigFile(const std::string& content);
    std::string getConfigValue(const std::map<std::string, std::string>& config, 
                              const std::string& key, const std::string& defaultValue = "");
}
