#include <gtkmm.h>
#include <filesystem>
#include <queue>

struct page{
    Gtk::ListBox* listBox;
    Gtk::Box* overlay;
    Gtk::Spinner* loadingSpinner;
    Gtk::Label* loadingLabel;
    std::filesystem::path root;
    int num;
    bool isSearching = false;
    bool isIndexing = false;
    bool isUpdating = false;
};

class wrmmmGUI : public Gtk::Window
{
public:
    wrmmmGUI();
    ~wrmmmGUI() override;
    void populateSearchHits(std::vector<std::string>, std::filesystem::path root);

    std::optional<page*> getPage(std::filesystem::path root);
    std::optional<page*> getPage(int num);

    std::string getSearchTerm();
    std::string getSearchRoot();

    void addSearchPageIfNotExist(std::filesystem::path root, bool switchToPage = false);

    void dispatchSearchingState(bool searching, std::filesystem::path root,
                                std::optional<std::vector<std::string>> results = std::nullopt);
    void dispatchIndexingState(bool indexing, std::filesystem::path root);
    void dispatchUpdatingState(bool updating, std::filesystem::path root);



    std::optional<bool> currentIsSearching();
    std::optional<bool> currentIsIndexing();
    std::optional<bool> currentIsUpdating();
    std::optional<std::filesystem::path> currentRoot();

    Gtk::Button* searchButton;
    Gtk::Button* refreshRootButton;
    Gtk::Button* updateButton;
    void addDispatchWork(std::filesystem::path root, std::optional<std::vector<std::string>> results,
                         std::optional<bool> isSearching, std::optional<bool> isIndexing, std::optional<bool> isUpdating);


private:

    struct dispatchTask{
        std::filesystem::path root;
        std::optional<std::vector<std::string>> results;
        std::optional<bool> isSearching;
        std::optional<bool> isIndexing;
        std::optional<bool> isUpdating;
    };

    Glib::Dispatcher dispatcher;

    void setSearchingState(bool searching, std::filesystem::path root);
    void setIndexingState(bool indexing, std::filesystem::path root);
    void setUpdatingState(bool updating, std::filesystem::path root);

    std::queue<dispatchTask> workQueue;
    std::mutex workQueueMutex;

    Gtk::Notebook* searchNotebook;
    Gtk::SearchEntry* searchEntry;
    Gtk::SearchEntry* searchRootEntry;

    std::unordered_map<std::filesystem::path, page*> pagesByPath;
    std::unordered_map<int, page*> pagesByNum;

    page* currentPage = nullptr;
};