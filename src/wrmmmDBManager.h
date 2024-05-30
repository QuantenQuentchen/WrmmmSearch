#include <sqlite_modern_cpp.h>
#include <optional>

struct FilePathData{
	std::optional<int> id;
	std::string path;
	std::optional<std::string> file;
	std::optional<time_t> timestamp;

};

struct VisitedFileData {
	std::string path;
};

struct idTimestamp {
	std::optional<int> id;
	std::optional<time_t> timestamp;
};

class wrmmmDBManager {
	private:
		sqlite::database db;

        void createMasterIndex();
        void createTempMasterIndex();

        void dropTableFilePath(std::string rootPath);
        void dropTableVisitedFile(std::string rootPath);
        void createIndexedTableFilePath(std::string rootPath);
        void createIndexedVisitedFile(std::string rootPath);

        std::string getFilePathTable(std::string rootPath);
        std::string getVisitedFileTable(std::string rootPath);

	public:
		
		explicit wrmmmDBManager(std::string dbPath);
		~wrmmmDBManager();


		void recreateTables(std::string rootPath);

		void bulkInsertFilePath(std::vector<FilePathData> filePathData, std::string rootPath);
		void bulkInsertVisitedFile(std::vector<VisitedFileData> visitedFileData, std::string rootPath);

		void dropUnvisitedFiles(std::string rootPath);

        std::vector<std::string> getAllIndexedRoots();

        time_t getTimestamp(std::string rootPath);

        std::vector<std::string> search(std::string term, std::string rootPath);

		idTimestamp selectIdTimestamp(std::string path, std::optional<std::string> file, std::string rootPath);
};