#pragma once


struct TabBrowserDirectoryState {
    int currPage = 0;
    int currRow = 0;
    bool collapsedPages = false;
};

std::map<std::string, TabBrowserDirectoryState> pathStateLookup;


std::string currPath = "/";
bool directoryInited = false;

struct DirEntry {
    char name[13];
    int32_t position;
};

PSVector<DirEntry> filesInDir;



static void loadTabFile(std::string filename) {
    SdFile file;

    if (!file.open(filename.c_str(), O_RDONLY)) {
        display.println("error opening file");
        updateDisplay();
        while(1) delay(100);
    }

    tabFilename = filename;
    if (tabContents != nullptr) free(tabContents);

    int len = file.fileSize();
    tabContents = (char*)malloc(len+1);
    file.read(tabContents, len);
    tabContents[len] = 0;
}


static bool hasTabExtension(char buf[13]) {
    int len = strlen(buf);
    if (len < 3) return false;
    char *ext = buf + len - 3;
    
    return strcmp(ext, "TXT") == 0 || strcmp(ext, "TAB") == 0;
}

static bool hasTabExtension(SdFile &file) {
    char buf[13];
    file.getSFN(buf);
    
    return hasTabExtension(buf);
}


static void chooseTab() {
    display.clearDisplay();
    updateDisplay(true);

    int lineSpacing = 2;
    int cursorWidth = 30;
    int headerHeight = mainFont.digitHeight + 4;

    int rowsPerPage = (screenHeight - headerHeight) / (mainFont.digitHeight + lineSpacing);
    int rowHeight = mainFont.digitHeight + lineSpacing;


    auto chdir = [&](std::string newDir){
        currPath = newDir;
        directoryInited = false;
    };

    restartDirectory:

    bool newPathState = false;

    if (pathStateLookup.find(currPath) == pathStateLookup.end()) {
        pathStateLookup.insert({});
        newPathState = true;
    }

    auto &pathState = pathStateLookup[currPath];

    auto largeEnoughToCollapsePages = [&]() {
        return (filesInDir.size() / rowsPerPage) > 10;
    };

    auto filesToPages = [&](int numFiles){
        return (numFiles + rowsPerPage - 1) / rowsPerPage;
    };

    auto numFiles = [&]() {
        if (pathState.collapsedPages) return filesToPages(filesInDir.size());
        else return (int)filesInDir.size();
    };

    auto numPages = [&]() {
        int n = filesToPages(filesInDir.size());
        if (pathState.collapsedPages) return filesToPages(n);
        else return n;
    };



    while(1) {
        SdFile dir;
        if (!dir.open(currPath.c_str())) {
            std::string errMsg = "open dir failed: ";
            errMsg += currPath;
            sd.errorHalt(errMsg.c_str());
        }

        if (!directoryInited) {
            filesInDir.clear();
            
            while (1) {
                SdFile file;
                
                int32_t position = dir.curPosition();
                if (!file.openNext(&dir, O_RDONLY)) break;
    
                DirEntry entry;
                file.getSFN(entry.name);
                entry.position = position;
                if (!file.isDir() && !hasTabExtension(entry.name)) continue;

                filesInDir.push_back(entry);
            }

            // Too slow on really big directories :(
            if (filesInDir.size() < 100) {
                std::sort(filesInDir.begin(), filesInDir.end(), [](DirEntry &a, DirEntry &b){ return memcmp(a.name, b.name, 13) < 0; });
            }

            // Reset in case directory size changed somehow
            if (pathState.currPage >= numPages()) {
                pathState.currPage = pathState.currRow = 0;
            }

            // Collapsed page mode
            if (largeEnoughToCollapsePages() && newPathState) pathState.collapsedPages = true;

            directoryInited = true;
        }

        while (1) {
            display.clearDisplay();

            setCursor(mainFont, 10, 1);
            display.print(currPath.c_str());
            display.drawThickLine(0, headerHeight - 1, screenWidth, headerHeight - 1, BLACK, 2);

            int fileIndex;
            if (pathState.collapsedPages) fileIndex = pathState.currPage * rowsPerPage * rowsPerPage;
            else fileIndex = pathState.currPage * rowsPerPage;

            int actualRows = 0;
            std::vector<std::string> filenames;

            while (actualRows < rowsPerPage) {
                if (fileIndex >= filesInDir.size()) break;

                dir.seekSet(filesInDir[fileIndex].position);

                if (pathState.collapsedPages) fileIndex += rowsPerPage;
                else fileIndex++;

                SdFile file;
                if (!file.openNext(&dir, O_RDONLY)) break;
                
                setCursor(mainFont, cursorWidth + 2, headerHeight + (actualRows * rowHeight) + (lineSpacing / 2));

                {
                    char buf[256];
                    file.getName(buf, sizeof(buf));
                    std::string filename(buf);
                    bool isDir = file.isDir();
                    if (isDir) filename += '/';

                    if (pathState.collapsedPages) {
                        display.print("Page ");
                        display.print(fileIndex / rowsPerPage);
                        display.print(": ");
                        display.print(abbreviate(filename, screenWidth/mainFont.digitWidth - 15).c_str());
                    } else {
                        if (isDir) display.print(filename.c_str());
                        else display.print(abbreviate(filename, screenWidth/mainFont.digitWidth - 10).c_str());
                        filenames.push_back(std::move(filename));
                    }
                }

                actualRows++;
            }

            if (pathState.currRow >= actualRows) pathState.currRow = actualRows - 1;
            

            while (1) {
                auto drawCursor = [&](bool draw){
                    display.fillTriangle(0, headerHeight + pathState.currRow * rowHeight,
                                         cursorWidth, headerHeight + (pathState.currRow*rowHeight) + (rowHeight/2),
                                         0, headerHeight + (pathState.currRow*rowHeight) + rowHeight,
                                         draw ? BLACK : WHITE);
                };

                drawCursor(true);

                updateDisplay();

                drawCursor(false);

                int gesture = getGesture();

                if (gesture == GESTURE_DOWN || gesture == GESTURE_DOWN_LONG || gesture == GESTURE_DOWN_DOUBLE) {
                    if (gesture == GESTURE_DOWN) pathState.currRow++;
                    else if (gesture == GESTURE_DOWN_LONG) pathState.currRow += rowsPerPage/3;
                    else if (gesture == GESTURE_DOWN_DOUBLE) pathState.currRow += rowsPerPage;

                    if (pathState.currRow >= actualRows) {
                        pathState.currPage++;
                        pathState.currRow = pathState.currRow - actualRows;
                        if (pathState.currPage >= numPages()) pathState.currPage = 0;
                        break;
                    }
                } else if (gesture == GESTURE_UP || gesture == GESTURE_UP_LONG || gesture == GESTURE_UP_DOUBLE) {
                    if (gesture == GESTURE_UP) pathState.currRow--;
                    else if (gesture == GESTURE_UP_LONG) pathState.currRow -= rowsPerPage/3;
                    else if (gesture == GESTURE_UP_DOUBLE) pathState.currRow -= rowsPerPage;

                    if (pathState.currRow < 0) {
                        pathState.currPage--;
                        pathState.currRow = rowsPerPage + pathState.currRow;
                        if (pathState.currPage < 0) pathState.currPage = numPages() - 1;
                        break;
                    }
                } else if (gesture == GESTURE_SELECT) {
                    if (pathState.collapsedPages) {
                        pathState.currPage = (pathState.currPage * rowsPerPage) + pathState.currRow;
                        pathState.currRow = 0;
                        pathState.collapsedPages = false;
                        goto restartDirectory;
                    }

                    // Click on tab

                    std::string &file = filenames[pathState.currRow];
                
                    if (file[file.size() - 1] == '/') {
                        chdir(currPath + file);
                        goto restartDirectory;
                    }
                
                    loadTabFile(currPath + filenames[pathState.currRow]);
                    return;
                } else if (gesture == GESTURE_SELECT_LONG) {
                    if (largeEnoughToCollapsePages() && !pathState.collapsedPages) {
                        pathState.currRow = pathState.currPage % rowsPerPage;
                        pathState.currPage = pathState.currPage / rowsPerPage;
                        pathState.collapsedPages = true;
                        goto restartDirectory;
                    }

                    if (currPath == "/") {
                        // do nothing. maybe go back to tab screen, if there is one?
                    } else {
                        std::string newPath = currPath.substr(0, currPath.size() - 1);
                        auto found = newPath.find_last_of('/');
                        newPath = newPath.substr(0, found+1);
                        chdir(newPath);
                        goto restartDirectory;
                    }
                }
            }
        }
    }
}
