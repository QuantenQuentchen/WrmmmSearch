#include "wrmmmSearch.h"
int main(int argc, char* argv[])
{
    putenv("GTK_THEME=Flat-Remix-GTK-Blue-Darkest");
    putenv("GTK_CSD=0");

    //TODO: Cleanup code, comparmentalize, put this somewhere else I mean
    //TODO: Look into copying indecies if already indexed
    //TODO: Little Update Button, that notifies, get update need based on root folder last edited
    //TODO: CSS!!!!

    std::setlocale(LC_ALL, "en_US.UTF-8");

    auto app = Gtk::Application::create("org.gtkmm.example");

    wrmmmGUI mainWindow = wrmmmGUI();

    wrmmmDBManager dbManager = wrmmmDBManager("wrmmm.db");
    SearchManager searchManager = SearchManager(&dbManager);

    std::vector<std::string> prevIndex = dbManager.getAllIndexedRoots();

    for (auto root : prevIndex) {
        mainWindow.addSearchPageIfNotExist(std::filesystem::path(root));
    }

    app->signal_activate().connect([&app, &mainWindow]() {
        mainWindow.set_visible(true);
        app->add_window(mainWindow);
    });


    mainWindow.updateButton->signal_clicked().connect([&mainWindow, &searchManager]() {
        std::optional<std::filesystem::path> currentRoot = mainWindow.currentRoot();
        if (!currentRoot.has_value()) {
            return;
        }
        if(mainWindow.currentIsUpdating().value()) {
            return;
        }
        mainWindow.dispatchUpdatingState(true, currentRoot.value());
        searchManager.updateIndex(currentRoot.value(), [&mainWindow, currentRoot]() {
            mainWindow.dispatchUpdatingState(false, currentRoot.value());
        });
    });

    mainWindow.refreshRootButton->signal_clicked().connect([&mainWindow, &searchManager]() {
        std::string root = mainWindow.getSearchRoot();
        if(!searchManager.isValidPath(root)) {
            return;
        }
        std::optional<page*> possibleExistingRoot = mainWindow.getPage(std::filesystem::path(root));
        if (!possibleExistingRoot.has_value()) {
            mainWindow.addSearchPageIfNotExist(std::filesystem::path(root), true);
        }
        std::optional<page*> Root = mainWindow.getPage(std::filesystem::path(root));
        if (!Root.has_value()) {
            return;
        }

        mainWindow.dispatchIndexingState(true, Root.value()->root);
        searchManager.updateIndex(Root.value()->root, [Root, &mainWindow]() {
            mainWindow.dispatchIndexingState(false, Root.value()->root);
        });
    });

    mainWindow.searchButton->signal_clicked().connect([&mainWindow, &searchManager]() {
        std::optional<std::filesystem::path> currentRoot = mainWindow.currentRoot();
        if (!currentRoot.has_value()) {
            return;
        }
        if(mainWindow.currentIsSearching().value()) {
            return;
        }
        mainWindow.dispatchSearchingState(true, mainWindow.currentRoot().value());
        std::string term = mainWindow.getSearchTerm();

        searchManager.search(term, currentRoot.value().string(), [currentRoot, &mainWindow](std::vector<std::string> hits) {
            mainWindow.dispatchSearchingState(false, currentRoot.value(), std::make_optional(hits));
        });
    });

    app->run(argc, argv);

    return 0;
}
