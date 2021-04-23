#pragma once

#include "logo.h"


static void drawSplashScreen() {
    display.clearDisplay();
    
    int logoX = screenWidth/2 - logo_width/2;
    int logoY = screenHeight/2 - logo_height/2;
    
    display.drawImage(logo_bits, 100, 100, logo_width, logo_height, BLACK);

    display.setFont(&FreeSerifBoldItalic24pt7b);
    display.setTextSize(2);
    display.setCursor(logoX + 20, logoY + 70);
    display.print("Tabplate");

    display.setFont(nullptr);
    display.setTextSize(2);
    display.setCursor(logoX + logo_width - 300, logoY + logo_height - 100);
    display.print("version 1.0");
    display.setCursor(logoX + logo_width - 300, logoY + logo_height - 100 + 25);
    display.print("by Doug Hoyte");
    display.setCursor(logoX + logo_width - 300, logoY + logo_height - 100 + 50);
    display.print("hoytech.com/tabplate");
    
    updateDisplay();
}
