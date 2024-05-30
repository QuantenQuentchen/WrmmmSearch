#include "SearchManager.h"

void SearchManager::updateWorker(FSManager* fsManager, std::function<void()> callback) {
    fsManager->updateDB();
    std::cout << "Finished updating index" << std::endl;
    if (callback != nullptr) {
        (callback)();
    }
}

void SearchManager::updateIndex(std::filesystem::path root, std::function<void()> callback) {
    auto it = indexes.find(root);
    FSManager* fsManager;
    if(it == indexes.end()) {
         fsManager = new FSManager(root, dbManager);
    }else
    {
        fsManager = it->second;
    }

    std::thread(&SearchManager::updateWorker, this, fsManager, callback).detach();
}

void SearchManager::searchWorker(std::string term, std::string root, std::function<void(std::vector<std::string>)> callback) {
    std::vector<std::string> results = dbManager->search(term, root);
    if (callback != nullptr) {
        (callback)(results);
    }
}

void SearchManager::search(std::string term, std::string root, std::function<void(std::vector<std::string>)> callback) {
    std::thread(&SearchManager::searchWorker, this, term, root, callback).detach();
}

bool SearchManager::isValidPath(const std::string& pathString) {
    std::filesystem::path path(pathString);
    return std::filesystem::exists(path);
}

SearchManager::SearchManager(wrmmmDBManager* dbManager) {
    this->dbManager = dbManager;
}
SearchManager::~SearchManager() {
    for (auto& index : indexes) {
        delete index.second;
    }
}

bool SearchManager::isUpToDate(std::filesystem::path root){
    return false;
}
