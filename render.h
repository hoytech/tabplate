#pragma once


static void drawScreen() {
    int marginWidth = 10;
    int barSpacing = 10;
    int digitPadding = 1;
    int headerHeight = mainFont.digitHeight + (digitPadding * 2) + 1;

    uint16_t colWidth = mainFont.digitWidth + (digitPadding * 2);
    uint16_t rowHeight = mainFont.digitHeight + (digitPadding * 2);



    Tab tab = Tab(tabContents);

    
    int currPage = 0;
    int currX = marginWidth;
    int currY = headerHeight + 8;
    int currColIndex = 0;

    while (currColIndex < tab.cols.size()) {
        int colsAvail = (screenWidth - currX - marginWidth) / colWidth;

        if (currColIndex + colsAvail >= tab.cols.size()) colsAvail = tab.cols.size() - currColIndex;

        if (colsAvail == 0) break;

        // Find a breakpoint

        {
            int backup = 0;

            while (backup < colsAvail && backup < 20) {
                auto &prevCol = tab.cols[currColIndex + colsAvail - 1 - backup];

                if (prevCol.breakpoint || prevCol.allStringLinesMatch('|')) {
                    colsAvail -= backup;
                    break;
                }

                backup++;
            }
        }

        // Find out how much space avail

        int maxLines, maxAlignment;
        tab.getRangeSize(currColIndex, colsAvail, &maxLines, &maxAlignment);

        if (currY + (maxLines * rowHeight) + barSpacing > screenHeight) {
            currY = headerHeight;
            currPage++;
            continue;
        }

        // Do render

        if (currPage == selectedPage) {
            int lastCol = std::min(currColIndex + colsAvail, (int)tab.cols.size()) - 1;
            
            int lineMask[maxLines] = {}; // ignore characters up to these colIndex's
                
            for (int colIndex = currColIndex; colIndex <= lastCol; colIndex++) {
                Column currCol = tab.cols[colIndex];

                int midPointHori = currX + (colWidth / 2);

                // Repeats
                if (currCol.stringLinesCmp("-*--*-") || currCol.stringLinesCmp("--**--") ||
                    currCol.stringLinesCmp("-o--o-") || currCol.stringLinesCmp("--oo--")) {
                    currCol.dashOutStrings();
                    display.fillCircle(midPointHori, currY + (maxAlignment + 2) * rowHeight, mainFont.digitWidth/2, BLACK);
                    display.fillCircle(midPointHori, currY + (maxAlignment + 4) * rowHeight, mainFont.digitWidth/2, BLACK);
                }

                for (int line = 0; line < maxLines; line++) {
                    if (colIndex != 0 && lineMask[line] >= colIndex) continue;
                    
                    int offset = line - maxAlignment + currCol.alignment;

                    if (offset >= 0 && currCol.numLines > offset) {
                        char c = currCol.col[offset];

                        int midPointVert = currY + (line * rowHeight) + (rowHeight / 2);

                        bool skipDraw = false;
                        
                        if (c == '-') {
                            skipDraw = true;
                            display.drawLine(currX, midPointVert, currX + colWidth, midPointVert, BLACK);
                        } else if (c == '_') {
                            skipDraw = true;
                            int vert = midPointVert + (rowHeight/2) - digitPadding;
                            display.drawThickLine(currX, vert, currX + colWidth, vert, BLACK, 1.4);
                        } else if (c == '=' || c == '~') {
                            skipDraw = true;
                            display.drawThickLine(currX, midPointVert, currX + colWidth, midPointVert, BLACK, 3);
                        } else if (c == '|' || c == ':') {
                            skipDraw = true;
                            int startY = currY + (line * rowHeight);
                            int endY = currY + (line * rowHeight) + rowHeight;
                            float thickness = 1;

                            if (colIndex > 0 && tab.cols[colIndex-1].allStringLinesMatch('|')) {
                                // End of piece
                                thickness = 3;
                            }

                            bool addBar = offset >= currCol.alignment && offset <= currCol.alignment + 5;

                            if (offset == currCol.alignment) {
                                startY += rowHeight/2;
                            } else if (offset == currCol.alignment + 5) {
                                endY -= rowHeight/2;
                            }

                            display.drawThickLine(midPointHori, startY, midPointHori, endY, BLACK, thickness);

                            if (addBar) {
                                int startX;
                                if (colIndex == currColIndex) startX = currX + (colWidth/2);
                                else startX = currX;

                                int endX;
                                if (colIndex == currColIndex + colsAvail - 1) endX = currX + (colWidth/2);
                                else endX = currX + colWidth;

                                display.drawLine(startX, midPointVert, endX, midPointVert, BLACK);
                            }
                        } else if (c == '<' || c == '>') {
                            // Crescendo/Diminuendo
                            int nextColIndex = colIndex + 1;
                            char prevChar = c;
                            while (nextColIndex <= lastCol && tab.cols[nextColIndex].numLines > offset) {
                                 char nextChar = tab.cols[nextColIndex].col[offset];
                                 if (nextChar != ' ' && nextChar != c) break;
                                 if (nextChar == ' ' && prevChar == ' ') break;
                                 prevChar = nextChar;
                                 nextColIndex++;
                            }
                            if (prevChar == ' ') nextColIndex--; // back it up

                            if (nextColIndex > colIndex + 1) {
                                skipDraw = true;
                                lineMask[line] = nextColIndex;
                                int endX = currX + ((nextColIndex - colIndex) * colWidth);
                                if (c == '<') {
                                    display.drawThickLine(currX, midPointVert, endX, midPointVert - (rowHeight/2), BLACK, 1.5);
                                    display.drawThickLine(currX, midPointVert, endX, midPointVert + (rowHeight/2), BLACK, 1.5);
                                } else {
                                    display.drawThickLine(currX, midPointVert - (rowHeight/2), endX, midPointVert, BLACK, 1.5);
                                    display.drawThickLine(currX, midPointVert + (rowHeight/2), endX, midPointVert, BLACK, 1.5);
                                }
                            }
                        }
                        
                        if (!skipDraw) {
                            FontSetting &font = (offset >= currCol.alignment && offset < currCol.alignment + 6) ? mainFont : annotationFont;
                            setCursor(font, currX + digitPadding + 1, currY + (line * rowHeight) + digitPadding + 1);
                            display.print(currCol.col[offset]);
                        }
                    }
                }

                currX += colWidth;
            }
        }

        currX = marginWidth;
        currColIndex += colsAvail;
        currY += (maxLines * rowHeight) + barSpacing;

        if (currColIndex > 0 && currColIndex < tab.cols.size() - 1 && tab.cols[currColIndex - 1].allStringLinesMatch('|')) {
            // Previous breakpoint was at a bar, so start new line with a bar too.
            // FIXME: this will duplicate any annotations also :(
            currColIndex--;
        }
    }
    

    // Render header

    display.drawThickLine(0, headerHeight - 1, screenWidth, headerHeight - 1, BLACK, 2);
    setCursor(mainFont, digitPadding, digitPadding);
    display.print(selectedPage + 1);
    display.print(" / ");
    display.print(currPage + 1);
    display.print("  :  ");
    display.print(abbreviate(tabFilename, screenWidth/mainFont.digitWidth/2).c_str());

    {
        float voltage = display.readBattery();
        setCursor(mainFont, screenWidth - digitPadding - (5 * mainFont.digitWidth), digitPadding);
        display.print(voltage, 2);
        display.print("V");
    }

    lastPage = currPage;
}
