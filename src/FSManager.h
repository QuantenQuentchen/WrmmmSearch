#include <iostream>
#include <filesystem>
#include <chrono>
#include "wrmmmDBManager.h"

class wrmmmDBManager;

class FSManager {

	private:
		std::filesystem::path searchRootpath;
        std::string searchRootpathStr;
		wrmmmDBManager* dbManager;

		std::vector<std::filesystem::path> excludedSubdirs;

		void sortExcludedDirs();

	public:
		void updateDB();

		FSManager(std::filesystem::path searchRootpath, wrmmmDBManager* dbManager);

		void setExcludedSubdirs(std::vector<std::filesystem::path> excludedSubdirs);
		void addExcludedSubdir(const std::filesystem::path& excludedSubdir);
		void removeExcludedSubdir(const std::filesystem::path& excludedSubdir);
};