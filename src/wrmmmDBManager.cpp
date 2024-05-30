#include "wrmmmDBManager.h"

void wrmmmDBManager::bulkInsertFilePath(std::vector<FilePathData> filePathData, std::string rootPath) {
    std::cout << "Bulk inserting file path data for " << rootPath << std::endl;
    createIndexedTableFilePath(rootPath);
    std::string tableName = getFilePathTable(rootPath);
    if (tableName.empty()) {
        return;
    }

    db << "begin;";
    for (auto &data: filePathData) {
        db << "INSERT OR REPLACE INTO " + tableName + " (id, path, file, timestamp) values (?, ?, ?, ?);"
           << data.id
           << data.path
           << data.file
           << data.timestamp;
    }
    db << "commit;";

}

void wrmmmDBManager::bulkInsertVisitedFile(std::vector<VisitedFileData> visitedFileData, std::string rootPath) {
    createIndexedVisitedFile(rootPath);
    std::string tableName = getVisitedFileTable(rootPath);
    if (tableName.empty()) {
        return;
    }

    db << "begin;";
    for (auto& data : visitedFileData) {
        db << "insert into "+tableName+" (path) values (?);"
            << data.path;
    }
    db << "commit;";
}

void wrmmmDBManager::recreateTables(std::string rootPath) {
    dropTableFilePath(rootPath);
    dropTableVisitedFile(rootPath);
    createIndexedTableFilePath(rootPath);
    createIndexedVisitedFile(rootPath);
}

void wrmmmDBManager::dropTableFilePath(std::string rootPath){
    std::string tableName = getFilePathTable(rootPath);
    if (tableName.empty()) {
        return;
    }
	db << "drop table if exists "+tableName+";";
    db << "drop index if exists "+tableName+"_file_index;";
    db << "drop index if exists "+tableName+"_path_index;";
    db << "delete from MasterIndex where path = ?;"
        << rootPath;
}

void wrmmmDBManager::dropTableVisitedFile(std::string rootPath){
    std::string tableName = getVisitedFileTable(rootPath);
    if (tableName.empty()) {
        return;
    }
	db << "drop table if exists "+tableName+";";
    db << "drop index if exists "+tableName+"_path_index;";
    db << "delete from TempMasterIndex where path = ?;"
        << rootPath;
}

void wrmmmDBManager::dropUnvisitedFiles(std::string rootPath){
    std::string tableName = getFilePathTable(rootPath);
    if (tableName.empty()) {
        return;
    }
    db << "delete from "+tableName+" where path not in (select path from "+getVisitedFileTable(rootPath)+");";
}

wrmmmDBManager::wrmmmDBManager(std::string dbPath) : db(dbPath) {
	createMasterIndex();
    createTempMasterIndex();
}

wrmmmDBManager::~wrmmmDBManager() {
}

idTimestamp wrmmmDBManager::selectIdTimestamp(std::string path, std::optional<std::string> file, std::string rootPath) {
    std::string tableName = getFilePathTable(rootPath);
    if (tableName.empty()) {
        return { std::nullopt, std::nullopt };
    }
    std::optional<int> id;
    std::optional<time_t> timestamp;
    db << "select id, timestamp from " + tableName + " where path = ? AND file = ?;"
        << path
        << file
        >> [&](std::optional<int> row_id, std::optional<time_t> row_timestamp) {
        id = row_id;
        timestamp = row_timestamp;
    };

    return  { id, timestamp };
}

std::vector<std::string> wrmmmDBManager::search(std::string term, std::string rootPath) {
    std::vector<std::string> results;
    std::string tableName = getFilePathTable(rootPath);
    if (tableName.empty()) {
        return results;
    }
    db << "select path from " +tableName+" where file = ?;"
        << term
        >> [&](std::string path) {
        results.push_back(path);
    };

    return results;
}

void wrmmmDBManager::createMasterIndex() {
    db << "CREATE TABLE IF NOT EXISTS MasterIndex(path TEXT PRIMARY KEY, table_name TEXT, timestamp INTEGER);";
}

void wrmmmDBManager::createTempMasterIndex() {
    db << "CREATE TABLE IF NOT EXISTS TempMasterIndex(path TEXT PRIMARY KEY, table_name TEXT, timestamp INTEGER);";
}

void wrmmmDBManager::createIndexedTableFilePath(std::string rootPath) {
    std::cout << "Creating file path table for " << rootPath << std::endl;
    std::string tableName = "FilePathData_" + std::to_string(std::hash<std::string>{}(rootPath));
    db << "CREATE TABLE IF NOT EXISTS " + tableName + "(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT, file TEXT, timestamp INTEGER);";
    db << "CREATE INDEX IF NOT EXISTS "+tableName+"_path_index ON " + tableName + "(path);";
    db << "CREATE INDEX IF NOT EXISTS "+tableName+"_file_index ON " + tableName + "(file);";
    db << "INSERT OR REPLACE INTO MasterIndex (path, table_name, timestamp) values (?, ?, ?);"
        << rootPath
        << tableName
        << 0;
    std::cout << "Created table " << tableName << std::endl;
}

void wrmmmDBManager::createIndexedVisitedFile(std::string rootPath) {
    std::cout << "Creating visited file table for " << rootPath << std::endl;
    std::string tableName = "VisitedFileData_" + std::to_string(std::hash<std::string>{}(rootPath));
    db << "CREATE TABLE IF NOT EXISTS " +tableName+ "(id INTEGER PRIMARY KEY AUTOINCREMENT, path TEXT);";
    db << "CREATE INDEX IF NOT EXISTS "+tableName+"_path_index ON " + tableName + "(path);";
    db << "INSERT OR REPLACE INTO TempMasterIndex (path, table_name, timestamp) values (?, ?, ?);"
        << rootPath
        << tableName
        << 0;
}

std::string wrmmmDBManager::getFilePathTable(std::string rootPath) {
    std::string tableName;
    db << "select table_name from MasterIndex where path = ?;"
        << rootPath
        >> [&](std::string table_name) {
        tableName = table_name;
    };

    return tableName;
}

std::string wrmmmDBManager::getVisitedFileTable(std::string rootPath) {
    std::string tableName;
    db << "select table_name from TempMasterIndex where path = ?;"
        << rootPath
        >> [&](std::string table_name) {
        tableName = table_name;
    };

    return tableName;
}

std::vector<std::string> wrmmmDBManager::getAllIndexedRoots() {
    std::vector<std::string> roots;
    db << "select path from MasterIndex;"
        >> [&](std::string path) {
        roots.push_back(path);
    };

    return roots;
}

time_t wrmmmDBManager::getTimestamp(std::string rootPath) {
    time_t timestamp;
    db << "select timestamp from MasterIndex where path = ?;"
        << rootPath
        >> [&](time_t ts) {
        timestamp = ts;
    };

    return timestamp;
}