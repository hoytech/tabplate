#include "../parseTab.h"
#include <iostream>
#include <fstream>
#include <sstream>



void renderColumns(Tab &tab) {
    int maxLines, maxAlignment;
    tab.getRangeSize(0, tab.cols.size(), &maxLines, &maxAlignment);

    for (auto &c : tab.cols) {
        std::cout << (c.breakpoint ? 'X' : ' ');
    }
    std::cout << "\n";

    for (auto &c : tab.cols) {
        std::cout << (c.numLines-6);
    }
    std::cout << "\n";

    for (int i = 0; i < maxLines; i++) {
        for (auto &c : tab.cols) {
            int offset = i - maxAlignment + c.alignment;

            if (offset < 0 || c.numLines <= offset) std::cout << " ";
            else std::cout << c.col[offset];
        }

        std::cout << "\n";
    }
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "usage: tabplate <filename>" << std::endl;
        return 1;
    }

    std::ifstream t(argv[1]);
    std::stringstream buffer;
    buffer << t.rdbuf();

    Tab tab = Tab(buffer.str().c_str());

    renderColumns(tab);

    return 0;
}
