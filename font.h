#pragma once


#include "Fonts/Picopixel.h"
#include "Fonts/FreeSerifBoldItalic24pt7b.h"


struct FontSetting {
    const GFXfont *font;
    int textSize;
    uint16_t digitWidth;
    uint16_t digitHeight;
};


static FontSetting initFont(const GFXfont *font, int textSize) {
    FontSetting f;
    f.font = font;
    f.textSize = textSize;

    display.setFont(f.font);
    display.setTextSize(f.textSize);
    
    {
        // Assume fixed width font
        int16_t junk;
        display.getTextBounds("X", 0, 0, &junk, &junk, &f.digitWidth, &f.digitHeight);
    }

    return f;
}


static void setCursor(FontSetting &f, int x, int y) {
    display.setFont(f.font);
    display.setTextSize(f.textSize);
    
    if (f.font) {
        // Custom fonts set to baseline, not top-left like default
        y += f.digitHeight;
    }

    display.setCursor(x, y);
}


FontSetting mainFont = initFont(nullptr, 2);
FontSetting annotationFont = initFont(&Picopixel, 2);
