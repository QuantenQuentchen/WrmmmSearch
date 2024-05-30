#include <iostream>
#include "wrmmmGUI.h"

wrmmmGUI::wrmmmGUI() {
    auto refBuilder = Gtk::Builder::create_from_file("wrmmm.xml");


    auto searchIcon = Gtk::make_managed<Gtk::Picture>();
    searchIcon->set_filename("magnify.svg");

    auto rootIcon = Gtk::make_managed<Gtk::Picture>();
    rootIcon->set_filename("refresh.svg");

    //Gtk::Widget* pRoot = nullptr;
    auto pRoot = refBuilder->get_widget<Gtk::Box>("main");

    searchNotebook = refBuilder->get_widget<Gtk::Notebook>("searchNotebook");
    searchEntry = refBuilder->get_widget<Gtk::SearchEntry>("searchEntry");
    searchRootEntry = refBuilder->get_widget<Gtk::SearchEntry>("searchRootEntry");

    refreshRootButton = refBuilder->get_widget<Gtk::Button>("refreshRootButton");
    refreshRootButton->set_child(*rootIcon);

    searchButton = refBuilder->get_widget<Gtk::Button>("searchButton");
    searchButton->set_child(*searchIcon);

    updateButton = refBuilder->get_widget<Gtk::Button>("updateButton");

    set_title("wrmmm");

    if (pRoot)
    {
        set_child(*pRoot);
    }
    searchNotebook->signal_switch_page().connect([this](Gtk::Widget* page, guint page_num) {
        currentPage = pagesByNum[page_num];
        if(currentPage){
        }
    });

    dispatcher.connect([this]() {
        dispatchTask work;
        {
            std::lock_guard<std::mutex> lock(workQueueMutex);
            if (!workQueue.empty()) {
                work = workQueue.front();
            } else {
                return;
            }

        }
        if(work.results.has_value()) {
            populateSearchHits(work.results.value(), work.root);
        }
        if(work.isSearching.has_value()) {
            setSearchingState(work.isSearching.value(), work.root);
        }
        if(work.isIndexing.has_value()) {
            setIndexingState(work.isIndexing.value(), work.root);
        }
        if(work.isUpdating.has_value()) {
            setUpdatingState(work.isUpdating.value(), work.root);
        }
        {
            std::lock_guard<std::mutex> lock(workQueueMutex);
            workQueue.pop();
        }
    });

}

void wrmmmGUI::addDispatchWork(std::filesystem::path root, std::optional<std::vector<std::string>> results,
                               std::optional<bool> isSearching, std::optional<bool> isIndexing, std::optional<bool> isUpdating) {
    std::lock_guard<std::mutex> lock(workQueueMutex);
    workQueue.push({root, results, isSearching, isIndexing, isUpdating});
}

void wrmmmGUI::populateSearchHits(std::vector<std::string> hits, std::filesystem::path root) {
    Gtk::ListBox* searchHits = pagesByPath[root]->listBox;
    searchHits->remove_all();
    for (auto hit : hits) {
        auto listItem = Gtk::make_managed<Gtk::ListBoxRow>();
        auto refBuilder = Gtk::Builder::create_from_file("ListItem.xml");

        auto listItemRoot = refBuilder->get_widget<Gtk::Box>("main");
        auto listItemLabel = refBuilder->get_widget<Gtk::Label>("listItemLabel");
        auto listItemButton = refBuilder->get_widget<Gtk::Button>("listItemButton");
        listItemLabel->set_text(hit);
        listItem->set_child(*listItemRoot);
        searchHits->append(*listItem);
    }
    if(hits.empty()){
        auto listItem = Gtk::make_managed<Gtk::ListBoxRow>();
        auto refBuilder = Gtk::Builder::create_from_file("ListItem.xml");

        auto listItemRoot = refBuilder->get_widget<Gtk::Box>("main");
        auto listItemLabel = refBuilder->get_widget<Gtk::Label>("listItemLabel");
        auto listItemButton = refBuilder->get_widget<Gtk::Button>("listItemButton");
        listItemButton->set_visible(false);
        listItemLabel->set_text("No results found");
        listItem->set_child(*listItemRoot);
        searchHits->append(*listItem);
    }
}



std::string wrmmmGUI::getSearchTerm() {
    std::string input = reinterpret_cast<const char*>(searchEntry->get_text().c_str());
    std::string escaped_input;
    std::transform(input.begin(), input.end(), std::back_inserter(escaped_input),
                   [](char8_t c) { return (c == u8'\\') ? u8'\\' : c; });
    return escaped_input;
}

std::string wrmmmGUI::getSearchRoot() {
    std::string input = reinterpret_cast<const char*>(searchRootEntry->get_text().c_str());
    std::string escaped_input;
    std::transform(input.begin(), input.end(), std::back_inserter(escaped_input),
                   [](char8_t c) { return (c == u8'\\') ? u8'\\' : c; });
    return escaped_input;
}

void wrmmmGUI::addSearchPageIfNotExist(std::filesystem::path root, bool switchToPage) {

    if (pagesByPath.find(root) == pagesByPath.end()) {
        auto refBuilder = Gtk::Builder::create_from_file("SearchTab.xml");
        auto searchTabRoot = refBuilder->get_widget<Gtk::ScrolledWindow>("main");
        auto searchHits = refBuilder->get_widget<Gtk::ListBox>("searchHits");
        auto loadingOverlay = refBuilder->get_widget<Gtk::Box>("loadingOverlay");
        auto loadingSpinner = refBuilder->get_widget<Gtk::Spinner>("loadingSpinner");
        auto loadingLabel = refBuilder->get_widget<Gtk::Label>("loadingLabel");

        std::string nameString = "("+ root.root_name().string() +") \\"+ root.filename().string() +"\\";
        searchNotebook->append_page(*searchTabRoot, nameString);
        int num = searchNotebook->get_n_pages() - 1;

        page* searchPage = new page{searchHits, loadingOverlay, loadingSpinner, loadingLabel, root, num};

        pagesByPath[root] = searchPage;
        pagesByNum[num] = searchPage;
        if(switchToPage){
            searchNotebook->set_current_page(num);
        }
        setSearchingState(false, root);
        setIndexingState(false, root);
    }
}

void wrmmmGUI::setSearchingState(bool searching, std::filesystem::path root) {
    page* subjectPage = pagesByPath[root];
    subjectPage->isSearching = searching;
    if (searching) {
        subjectPage->overlay->set_visible(true);
        subjectPage->loadingSpinner->start();
        subjectPage->loadingLabel->set_text("Searching...");
    } else {
        subjectPage->overlay->set_visible(false);
        subjectPage->loadingSpinner->stop();
    }
}

void wrmmmGUI::setIndexingState(bool indexing, std::filesystem::path root) {
    page* subjectPage = pagesByPath[root];
    subjectPage->isIndexing = indexing;
    std::cout << "Setting indexing state to " << indexing << " for " << root << std::endl;
    if (indexing) {
        subjectPage->overlay->set_visible(true);
        subjectPage->loadingSpinner->start();
        subjectPage->loadingLabel->set_text("Indexing...");
    } else {
        subjectPage->overlay->set_visible(false);
        subjectPage->loadingSpinner->stop();
    }
}

void wrmmmGUI::setUpdatingState(bool updating, std::filesystem::path root) {
    page* subjectPage = pagesByPath[root];
    subjectPage->isUpdating = updating;
    if (updating) {
        subjectPage->overlay->set_visible(true);
        subjectPage->loadingSpinner->start();
        subjectPage->loadingLabel->set_text("Updating...");
    } else {
        subjectPage->overlay->set_visible(false);
        subjectPage->loadingSpinner->stop();
    }
}

std::optional<bool> wrmmmGUI::currentIsSearching() {
    if(currentPage == nullptr){
        return std::nullopt;
    }
    return currentPage->isSearching;
}

std::optional<bool> wrmmmGUI::currentIsIndexing() {
    if(currentPage == nullptr){
        return std::nullopt;
    }
    return currentPage->isIndexing;
}

std::optional<bool> wrmmmGUI::currentIsUpdating() {
    if(currentPage == nullptr){
        return std::nullopt;
    }
    return currentPage->isUpdating;
}

std::optional<std::filesystem::path> wrmmmGUI::currentRoot() {
    if(currentPage == nullptr){
        return std::nullopt;
    }
    return currentPage->root;
}

std::optional<page*> wrmmmGUI::getPage(std::filesystem::path root) {
    if(pagesByPath.find(root) == pagesByPath.end()){
        return std::nullopt;
    }
    return pagesByPath[root];
}

std::optional<page*> wrmmmGUI::getPage(int num) {
    if(pagesByNum.find(num) == pagesByNum.end()){
        return std::nullopt;
    }
    return pagesByNum[num];
}

void wrmmmGUI::dispatchSearchingState(bool searching, std::filesystem::path root, std::optional<std::vector<std::string>> results) {
    addDispatchWork(root, results, searching, std::nullopt, std::nullopt);
    dispatcher.emit();
}
void wrmmmGUI::dispatchIndexingState(bool indexing, std::filesystem::path root) {
    addDispatchWork(root, std::nullopt, std::nullopt, indexing, std::nullopt);
    dispatcher.emit();
}
void wrmmmGUI::dispatchUpdatingState(bool updating, std::filesystem::path root) {
    addDispatchWork(root, std::nullopt, std::nullopt, std::nullopt, updating);
    dispatcher.emit();
}


wrmmmGUI::~wrmmmGUI() {
    for (auto& page : pagesByPath) {
        delete page.second->listBox;
        delete page.second->overlay;
        delete page.second->loadingSpinner;
        delete page.second->loadingLabel;
        page.second->listBox = nullptr;
        page.second->overlay = nullptr;
        page.second->loadingSpinner = nullptr;
        page.second->loadingLabel = nullptr;

        delete page.second;
        page.second = nullptr;
    }
    delete searchNotebook;
    delete searchEntry;
    delete searchRootEntry;
    delete refreshRootButton;
    delete searchButton;
    searchNotebook = nullptr;
    searchEntry = nullptr;
    searchRootEntry = nullptr;
    refreshRootButton = nullptr;
    searchButton = nullptr;
}