#include "database.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>

Database::Database(const std::string& dbPath) 
    : dbPath_(dbPath), db_(nullptr), connected_(false) {}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    if (connected_) return true;
    
    int result = sqlite3_open(dbPath_.c_str(), &db_);
    if (result != SQLITE_OK) {
        setLastError("无法打开数据库: " + std::string(sqlite3_errmsg(db_)));
        sqlite3_close(db_);
        db_ = nullptr;
        return false;
    }
    
    connected_ = true;
    std::cout << "数据库连接成功: " << dbPath_ << std::endl;
    return true;
}

void Database::disconnect() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
    connected_ = false;
}

bool Database::execute(const std::string& sql) {
    if (!connected_) {
        setLastError("数据库未连接");
        return false;
    }
    
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result != SQLITE_OK) {
        setLastError("SQL执行失败: " + std::string(errorMsg));
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

ResultSet Database::query(const std::string& sql) {
    ResultSet results;
    
    if (!connected_) {
        setLastError("数据库未连接");
        return results;
    }
    
    sqlite3_stmt* stmt = prepareStatement(sql);
    if (!stmt) {
        return results;
    }
    
    // 获取列数
    int columnCount = sqlite3_column_count(stmt);
    
    // 获取列名
    std::vector<std::string> columnNames;
    for (int i = 0; i < columnCount; ++i) {
        columnNames.push_back(getColumnName(stmt, i));
    }
    
    // 获取数据
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        RowData row;
        for (int i = 0; i < columnCount; ++i) {
            row[columnNames[i]] = getColumnValue(stmt, i);
        }
        results.push_back(row);
    }
    
    sqlite3_finalize(stmt);
    return results;
}

long long Database::insert(const std::string& sql) {
    if (!execute(sql)) {
        return -1;
    }
    
    return sqlite3_last_insert_rowid(db_);
}

int Database::update(const std::string& sql) {
    if (!execute(sql)) {
        return -1;
    }
    
    return sqlite3_changes(db_);
}

int Database::remove(const std::string& sql) {
    return update(sql);
}

bool Database::beginTransaction() {
    return execute("BEGIN TRANSACTION");
}

bool Database::commitTransaction() {
    return execute("COMMIT");
}

bool Database::rollbackTransaction() {
    return execute("ROLLBACK");
}

void Database::setLastError(const std::string& error) {
    lastError_ = error;
    std::cerr << "数据库错误: " << error << std::endl;
}

bool Database::executeStatement(const std::string& sql) {
    char* errorMsg = nullptr;
    int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMsg);
    
    if (result != SQLITE_OK) {
        setLastError("SQL执行失败: " + std::string(errorMsg));
        sqlite3_free(errorMsg);
        return false;
    }
    
    return true;
}

sqlite3_stmt* Database::prepareStatement(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    
    if (result != SQLITE_OK) {
        setLastError("SQL准备失败: " + std::string(sqlite3_errmsg(db_)));
        return nullptr;
    }
    
    return stmt;
}

bool Database::bindParameters(sqlite3_stmt* stmt, const std::vector<std::string>& params) {
    for (size_t i = 0; i < params.size(); ++i) {
        int result = sqlite3_bind_text(stmt, i + 1, params[i].c_str(), -1, SQLITE_TRANSIENT);
        if (result != SQLITE_OK) {
            setLastError("参数绑定失败: " + std::string(sqlite3_errmsg(db_)));
            return false;
        }
    }
    return true;
}

std::string Database::getColumnValue(sqlite3_stmt* stmt, int column) {
    if (sqlite3_column_type(stmt, column) == SQLITE_NULL) {
        return "";
    }
    
    const unsigned char* value = sqlite3_column_text(stmt, column);
    return value ? reinterpret_cast<const char*>(value) : "";
}

std::string Database::getColumnName(sqlite3_stmt* stmt, int column) {
    const char* name = sqlite3_column_name(stmt, column);
    return name ? name : "";
}

bool Database::initializeTables() {
    if (!connected_) {
        return false;
    }
    
    // 创建用户表
    std::string createUsersTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!execute(createUsersTable)) {
        return false;
    }
    
    // 创建API记录表
    std::string createApiLogsTable = R"(
        CREATE TABLE IF NOT EXISTS api_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            method TEXT NOT NULL,
            path TEXT NOT NULL,
            status_code INTEGER NOT NULL,
            response_time INTEGER,
            ip_address TEXT,
            user_agent TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!execute(createApiLogsTable)) {
        return false;
    }
    
    // 创建配置表
    std::string createConfigTable = R"(
        CREATE TABLE IF NOT EXISTS config (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL,
            description TEXT,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!execute(createConfigTable)) {
        return false;
    }
    
    // 插入默认配置
    std::string insertDefaultConfig = R"(
        INSERT OR IGNORE INTO config (key, value, description) VALUES
        ('server_name', 'APIManager', '服务器名称'),
        ('version', '1.0.0', '版本号'),
        ('max_connections', '100', '最大连接数'),
        ('log_level', 'INFO', '日志级别')
    )";
    
    execute(insertDefaultConfig);
    
    std::cout << "数据库表初始化完成" << std::endl;
    return true;
}
