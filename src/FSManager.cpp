#include "FSManager.h"
void FSManager::updateDB() {
    std::vector<FilePathData> filePathData;
    std::vector<VisitedFileData> visitedFileData;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(searchRootpath)){//}, std::filesystem::directory_options::skip_permission_denied | std::filesystem::directory_options::follow_directory_symlink)) {
        const std::filesystem::path path = entry.path();

        std::string fileName = path.filename().string();

        if (std::binary_search(excludedSubdirs.begin(), excludedSubdirs.end(), fileName)) {
            continue;
        }
        std::string pathName = path.string();

        if (entry.is_directory()) {

            idTimestamp rowidTimestamp = dbManager->selectIdTimestamp(pathName, fileName, searchRootpathStr);

            std::filesystem::file_time_type ftime = entry.last_write_time();
            auto s_clock_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
            std::time_t last_modified_time = std::chrono::system_clock::to_time_t(s_clock_time_point);
            if(rowidTimestamp.timestamp == last_modified_time){
                continue;
            }

            filePathData.emplace_back(rowidTimestamp.id, pathName, std::nullopt, last_modified_time);
        }
        else if (entry.is_regular_file()) {
            std::string parent_path = path.parent_path().string();
            idTimestamp rowidTimestamp = dbManager->selectIdTimestamp(parent_path, fileName, searchRootpathStr);
            filePathData.emplace_back(rowidTimestamp.id, parent_path, fileName, std::nullopt);
        }

        VisitedFileData data = {pathName};
        visitedFileData.push_back(data);
    }
    dbManager->bulkInsertFilePath(filePathData, searchRootpathStr);
    std::cout << "Bulk Inserted " << filePathData.size() << " files\n";
    dbManager->bulkInsertVisitedFile(visitedFileData, searchRootpathStr);
    std::cout << "Bulk Inserted " << visitedFileData.size() << " visited files\n";
    dbManager->dropUnvisitedFiles(searchRootpathStr);
}

FSManager::FSManager(std::filesystem::path searchRootpath, wrmmmDBManager* dbManagerConstr){
    this->dbManager = dbManagerConstr;
	this->searchRootpath = searchRootpath;
    this->searchRootpathStr = searchRootpath.string();
}

void FSManager::sortExcludedDirs(){

    std::sort(excludedSubdirs.begin(), excludedSubdirs.end(), [](std::filesystem::path a, std::filesystem::path b) {
		return a.u8string() < b.u8string();
	});
}
void FSManager::addExcludedSubdir(const std::filesystem::path& excludedSubdir) {
	this->excludedSubdirs.emplace_back(excludedSubdir);
	sortExcludedDirs();
}
void FSManager::removeExcludedSubdir(const std::filesystem::path& excludedSubdir) {
	this->excludedSubdirs.erase(std::remove(this->excludedSubdirs.begin(), this->excludedSubdirs.end(), excludedSubdir), this->excludedSubdirs.end());
}
void FSManager::setExcludedSubdirs(std::vector<std::filesystem::path> excludedSubdirs) {
	this->excludedSubdirs = excludedSubdirs;
	sortExcludedDirs();
}