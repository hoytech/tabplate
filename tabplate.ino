#include <algorithm>
#include <map>

#include "Inkplate.h"


Inkplate display(INKPLATE_1BIT);

int currRefresh = 0;

void updateDisplay(bool logicalBreakpoint = false) {
    currRefresh++;

    if (currRefresh > 50 || (logicalBreakpoint && currRefresh > 10)) {
        currRefresh = 0;
        display.display();
    } else {
        display.partialUpdate();
    }
}



uint16_t screenWidth;
uint16_t screenHeight;


// Tab viewer
std::string tabFilename;
char *tabContents = nullptr;
int selectedPage = 0;
int lastPage = -1;






#include "font.h"
#include "util.h"
#include "parseTab.h"
#include "splash.h"
#include "gesture.h"
#include "browse.h"
#include "render.h"




void setup() {
    Serial.begin(9600);
    // 800x600

    /*
    Serial.print("Total heap: "); Serial.println(ESP.getHeapSize());
    Serial.print("Free heap: "); Serial.println(ESP.getFreeHeap());
    Serial.print("Total PSRAM: "); Serial.println(ESP.getPsramSize());
    Serial.print("Free PSRAM: "); Serial.println(ESP.getFreePsram());
    */

    display.begin();
    display.setRotation(0);
    setCursor(mainFont, 0, 0);

    screenWidth = display.width();
    screenHeight = display.height();
    
    // picocom /dev/ttyUSB0
    /*
    Serial.print("Screen Width = ");
    Serial.println(screenWidth);
    Serial.print("Screen Height = ");
    Serial.println(screenHeight);
    */

    display.clearDisplay();
    display.display();

    drawSplashScreen();
    delay(4000);

    display.clearDisplay();
    updateDisplay();
    

    if (!display.sdCardInit()) {
        display.println("SD card error: Insert SD card and restart");
        updateDisplay();
        while(1) delay(100);
    }

    chooseTab();

    display.clearDisplay();
    drawScreen();
    updateDisplay();
}

void loop() {
    int origPage = selectedPage;

    int gesture = getGesture();

    if (gesture == GESTURE_UP) selectedPage--;
    else if (gesture == GESTURE_DOWN) selectedPage++;
    else if (gesture == GESTURE_SELECT) {
      chooseTab();
      display.clearDisplay();
      updateDisplay(true);
      selectedPage = 0;
      origPage = 1; // force a drawScreen(). FIXME: refactor this loop
    }
    else return;

    if (selectedPage < 0) selectedPage = lastPage;
    if (selectedPage > lastPage) selectedPage = 0;

    if (origPage != selectedPage) {
        display.clearDisplay();
        drawScreen();
        updateDisplay();
    }
}
