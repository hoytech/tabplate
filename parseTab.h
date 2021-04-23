#pragma once

#include <ctype.h>

#include <cstring>
#include <limits>
#include <utility>
#include <functional>

#include "util.h"


#define LINE_SCAN_WINDOW 16


struct Column {
    int numLines;
    int alignment;
    char col[LINE_SCAN_WINDOW] = { 'X', };
    bool breakpoint = false;

    bool allLinesMatch(char test) {
        return allLinesMatch([&](char c){ return test == c; });
    }

    bool allLinesMatch(std::function<bool(char)> cb) {
        for (int i = 0; i < numLines; i++) {
            if (!cb(col[i])) return false;
        }

        return true;
    }

    bool allStringLinesMatch(char test) {
        return allStringLinesMatch([&](char c){ return test == c; });
    }

    bool allStringLinesMatch(std::function<bool(char)> cb) {
        for (int i = 0; i < 6; i++) {
            if (!cb(col[alignment + i])) return false;
        }

        return true;
    }

    bool stringLinesCmp(char *test) {
        return memcmp(test, &col[alignment], 6) == 0;
    }

    void dashOutStrings() {
        for (int i = 0; i < 6; i++) {
            col[alignment + i] = '-';
        }
    }
};


static void parseTab(const char *tab, PSVector<Column> &output) {
    output.clear();

    const char *currp = tab;

    while (*currp) {
        while(*currp == '\n' || *currp == '\r') currp++; // isWhitespace
        if (!*currp) return;

        const char *textBlockStart = currp;
        int numLines = 0;
        int maxLineLen = 0;
        const char *lines[LINE_SCAN_WINDOW];
        int lineLens[LINE_SCAN_WINDOW];
        int lineScores[LINE_SCAN_WINDOW];

        do {
            lines[numLines] = currp;
            lineScores[numLines] = 0;

            bool seenNonWhitespace = false;

            while (*currp && *currp != '\n') {
                char c = *currp;

                if (c == '-' || c == '=' || (c >= '0' && c <= '9')) lineScores[numLines]++;
                else if (c == ' ') lineScores[numLines] -= 3;

                if (!isspace(c)) seenNonWhitespace = true;

                currp++;
            }

            if (!seenNonWhitespace) break;

            int lineLen = currp - lines[numLines];
            lineLens[numLines] = lineLen;

            if (lineLen > 0) numLines++;

            if (lineLen > maxLineLen) maxLineLen = lineLen;

            if (!*currp) break;
            currp++; // advance past newline
        } while (numLines < LINE_SCAN_WINDOW);

        if (numLines < 6) continue;

        // Found a line group, now try to find out where the string content is

        int lineAlignments[LINE_SCAN_WINDOW - 5];
        int bestAlignment = -1;
        int bestScore = std::numeric_limits<int>::min();

        {
            for (int testAlignment = 0; testAlignment < numLines - 5; testAlignment++) {
                int alignedScore = 0;

                for (int i = testAlignment; i < testAlignment + 6; i++) {
                    alignedScore += lineScores[i];
                }

                lineAlignments[testAlignment] = alignedScore;

                if (alignedScore > bestScore) {
                    bestScore = alignedScore;
                    bestAlignment = testAlignment;
                }
            }
        }


        auto pushColumns = [&](int pushLineStart, int pushNumLines, int pushAlignment){
            while (1) {
                Column newCol;

                newCol.numLines = pushNumLines;
                newCol.alignment = pushAlignment;

                int endedLines = 0;

                for (int i = 0; i < pushNumLines; i++) {
                    char c = *lines[pushLineStart + i];

                    if (c == '\0' || c == '\n') {
                        endedLines++;
                        newCol.col[i] = ' ';
                    } else {
                        lines[pushLineStart + i]++;
                        if (isspace(c)) c = ' ';
                        newCol.col[i] = c;
                    }
                }

                if (endedLines == pushNumLines) break;

                output.push_back(newCol);
            }

            if (output.size()) output.back().breakpoint = true;
        };

        if (numLines == LINE_SCAN_WINDOW) {
            // No-annotation mode: couldn't find any line-breaks, so just extract string lines

            int found = -1;

            for (int testAlignment = 0; testAlignment < numLines - 5; testAlignment++) {
                if (lineAlignments[testAlignment] > 30) {
                    int numAcceptableLines = 0;
                    for (int i = 0; i < 6; i++) {
                        if (lineScores[testAlignment + i] > 30) numAcceptableLines++;
                    }

                    if (numAcceptableLines == 6) {
                        found = testAlignment;
                        break;
                    }
                }
            }

            if (found != -1) {
                pushColumns(found, 6, 0);
                if (found + 6 < LINE_SCAN_WINDOW) currp = lines[found + 5];
            } else {
                currp = lines[LINE_SCAN_WINDOW - 5];
            }
        } else {
            // Annotation mode: assume this block has one set of strings and the rest are annotations

            if (bestScore < 30) continue; // Probably just junk text
            pushColumns(0, numLines, bestAlignment);
        }
    }
}


static void cleanupCols(PSVector<Column> &inputCols, PSVector<Column> &output) {
    output.clear();

    for (auto &input : inputCols) {
        if (output.size() > 0 && output.back().breakpoint && input.allLinesMatch(' ')) {
            // Leading whitespace, skip it
            continue;
        }

        if (output.size() && output.back().breakpoint) {
            if (input.allStringLinesMatch([](char c){ return !!isalpha(c); })) {
                // This is probably an EADGBE column at the beginning of the line
                input.dashOutStrings();
                input.breakpoint = true;
            } else if (input.allStringLinesMatch('|')) {
                // Redundant bar line caused by breakpoint
                input.dashOutStrings();
                input.breakpoint = true;
            }
        }

        output.push_back(input);

        while (output.size() > 0 && output.back().breakpoint && output.back().allLinesMatch(' ')) {
            // Trailing whitespace. Skip and propagate breakpoint
            output.pop_back();
            if (output.size()) output.back().breakpoint = true;
        }
    }

    // Collapse runs of multiple breakpoints to earliest occurrence

    for (int i = output.size(); i > 0; i--) {
        if (output[i - 1].breakpoint) output[i].breakpoint = false;
    }
}




struct Tab {
    Tab(const char *tab) {
        PSVector<Column> tempCols;
        parseTab(tab, tempCols);
        cleanupCols(tempCols, cols);
    }

    PSVector<Column> cols;

    void getRangeSize(int colStart, int numCols, int *outMaxLines, int *outMaxAlignment) {
        int maxAlignment = -1;
        int maxLines = 0;

        for (int i = colStart; i < colStart + numCols; i++) {
            auto &c = cols[i];
            if (c.numLines > maxLines) maxLines = c.numLines;
            if (c.alignment > maxAlignment) maxAlignment = c.alignment;
        }

        if (outMaxLines) *outMaxLines = maxLines;
        if (outMaxAlignment) *outMaxAlignment = maxAlignment;
    }
};
