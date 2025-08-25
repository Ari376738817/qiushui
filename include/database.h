#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

// SQLite前向声明
struct sqlite3;
struct sqlite3_stmt;

// 数据库行数据
using RowData = std::map<std::string, std::string>;

// 数据库结果集
using ResultSet = std::vector<RowData>;

// 数据库类
class Database {
public:
    Database(const std::string& dbPath);
    ~Database();
    
    // 连接数据库
    bool connect();
    
    // 断开连接
    void disconnect();
    
    // 检查连接状态
    bool isConnected() const { return connected_; }
    
    // 执行SQL语句（无返回值）
    bool execute(const std::string& sql);
    
    // 查询数据
    ResultSet query(const std::string& sql);
    
    // 插入数据并返回最后插入的行ID
    long long insert(const std::string& sql);
    
    // 更新数据并返回影响的行数
    int update(const std::string& sql);
    
    // 删除数据并返回影响的行数
    int remove(const std::string& sql);
    
    // 开始事务
    bool beginTransaction();
    
    // 提交事务
    bool commitTransaction();
    
    // 回滚事务
    bool rollbackTransaction();
    
    // 获取最后错误信息
    std::string getLastError() const { return lastError_; }
    
    // 初始化数据库表
    bool initializeTables();
    
private:
    std::string dbPath_;
    sqlite3* db_;
    bool connected_;
    std::string lastError_;
    
    // 设置错误信息
    void setLastError(const std::string& error);
    
    // 执行SQL语句
    bool executeStatement(const std::string& sql);
    
    // 准备SQL语句
    sqlite3_stmt* prepareStatement(const std::string& sql);
    
    // 绑定参数到语句
    bool bindParameters(sqlite3_stmt* stmt, const std::vector<std::string>& params);
    
    // 从结果集获取列值
    std::string getColumnValue(sqlite3_stmt* stmt, int column);
    
    // 获取列名
    std::string getColumnName(sqlite3_stmt* stmt, int column);
};
