#ifndef WRMMMSEARCH_SEARCHMANAGER_H
#define WRMMMSEARCH_SEARCHMANAGER_H

#include <iostream>
#include <map>
#include <optional>
#include <mutex>
#include <thread>
#include <functional>
#include "FSManager.h"

class SearchManager {

private:
    wrmmmDBManager* dbManager;

    std::unordered_map<std::filesystem::path, FSManager*> indexes;

    void searchWorker(std::string term, std::string root, std::function<void(std::vector<std::string>)> callback);

    void updateWorker(FSManager* fsManager, std::function<void()> callback);

    bool isUpToDate(std::filesystem::path root);

public:
    SearchManager(wrmmmDBManager* dbManager);
    ~SearchManager();

    bool isValidPath(const std::string& pathString);

    void updateIndex(std::filesystem::path root, std::function<void()> callback);
    void search(std::string term, std::string root, std::function<void(std::vector<std::string>)> callback);

};

#endif //WRMMMSEARCH_SEARCHMANAGER_H
