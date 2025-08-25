#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>
#include <algorithm>
#include <cctype>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <direct.h>

namespace Utils {

// 字符串操作
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    
    return tokens;
}

std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) return false;
    return str.substr(0, prefix.length()) == prefix;
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

// URL处理
std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int value;
            std::istringstream iss(str.substr(i + 1, 2));
            iss >> std::hex >> value;
            result += static_cast<char>(value);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

std::string urlEncode(const std::string& str) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    
    for (char c : str) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            escaped << '%' << std::setw(2) << int((unsigned char)c);
        }
    }
    
    return escaped.str();
}

// JSON处理
std::string escapeJsonString(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\b': result += "\\b"; break;
            case '\f': result += "\\f"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

std::string createJsonObject(const std::map<std::string, std::string>& data) {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& pair : data) {
        if (!first) oss << ",";
        oss << "\"" << escapeJsonString(pair.first) << "\":\"" << escapeJsonString(pair.second) << "\"";
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

std::string createJsonArray(const std::vector<std::string>& data) {
    std::ostringstream oss;
    oss << "[";
    
    for (size_t i = 0; i < data.size(); ++i) {
        if (i > 0) oss << ",";
        oss << "\"" << escapeJsonString(data[i]) << "\"";
    }
    
    oss << "]";
    return oss.str();
}

// 时间处理
std::string getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return oss.str();
}

std::string formatTimestamp(long long timestamp) {
    auto time_t = static_cast<time_t>(timestamp / 1000);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

long long getCurrentTimeMillis() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

// 随机数生成
std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::ostringstream oss;
    oss << std::hex;
    
    for (int i = 0; i < 8; ++i) oss << dis(gen);
    oss << "-";
    for (int i = 0; i < 4; ++i) oss << dis(gen);
    oss << "-";
    for (int i = 0; i < 4; ++i) oss << dis(gen);
    oss << "-";
    for (int i = 0; i < 4; ++i) oss << dis(gen);
    oss << "-";
    for (int i = 0; i < 12; ++i) oss << dis(gen);
    
    return oss.str();
}

int randomInt(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

std::string randomString(int length) {
    static const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(charset) - 2);
    
    std::string result;
    result.reserve(length);
    
    for (int i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    
    return result;
}

// 文件操作
bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

bool writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    return file.good();
}

bool createDirectory(const std::string& path) {
    return _mkdir(path.c_str()) == 0;
}

// 网络相关
std::string getLocalIP() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return "127.0.0.1";
    }
    
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        WSACleanup();
        return "127.0.0.1";
    }
    
    struct addrinfo hints, *result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(hostname, nullptr, &hints, &result) != 0) {
        WSACleanup();
        return "127.0.0.1";
    }
    
    std::string ip = "127.0.0.1";
    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
            struct sockaddr_in* sockaddr_ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            ip = inet_ntoa(sockaddr_ipv4->sin_addr);
            break;
        }
    }
    
    freeaddrinfo(result);
    WSACleanup();
    return ip;
}

bool isValidIP(const std::string& ip) {
    struct sockaddr_in sa;
    return inet_pton(AF_INET, ip.c_str(), &(sa.sin_addr)) != 0;
}

int parsePort(const std::string& portStr) {
    try {
        int port = std::stoi(portStr);
        if (port < 1 || port > 65535) {
            return 8080; // 默认端口
        }
        return port;
    } catch (...) {
        return 8080; // 默认端口
    }
}

// 特殊化字符串转换
template<>
std::string fromString<std::string>(const std::string& str) {
    return str;
}

// 日志格式化
std::string formatLogMessage(const std::string& level, const std::string& message, 
                            const std::string& source) {
    std::ostringstream oss;
    oss << "[" << getCurrentTimestamp() << "] ";
    oss << "[" << level << "] ";
    if (!source.empty()) {
        oss << "[" << source << "] ";
    }
    oss << message;
    return oss.str();
}

// 配置解析
std::map<std::string, std::string> parseConfigFile(const std::string& content) {
    std::map<std::string, std::string> config;
    std::istringstream iss(content);
    std::string line;
    
    while (std::getline(iss, line)) {
        line = trim(line);
        
        // 跳过空行和注释
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        size_t equalPos = line.find('=');
        if (equalPos != std::string::npos) {
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            // 移除引号
            if (value.length() >= 2 && 
                ((value[0] == '"' && value[value.length()-1] == '"') ||
                 (value[0] == '\'' && value[value.length()-1] == '\''))) {
                value = value.substr(1, value.length() - 2);
            }
            
            config[key] = value;
        }
    }
    
    return config;
}

std::string getConfigValue(const std::map<std::string, std::string>& config, 
                          const std::string& key, const std::string& defaultValue) {
    auto it = config.find(key);
    return (it != config.end()) ? it->second : defaultValue;
}

} // namespace Utils
