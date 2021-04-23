#pragma once

// External return values

#define GESTURE_UP 1
#define GESTURE_SELECT 2
#define GESTURE_DOWN 3
#define GESTURE_UP_LONG 4
#define GESTURE_SELECT_LONG 5
#define GESTURE_DOWN_LONG 6
#define GESTURE_UP_DOUBLE 7
#define GESTURE_DOWN_DOUBLE 8

// Internal states

#define GESTURE_STATE_NONE 0
#define GESTURE_STATE_PAD1_PRESSED 1
#define GESTURE_STATE_PAD2_PRESSED 2
#define GESTURE_STATE_PAD3_PRESSED 3
#define GESTURE_STATE_PAD12_LOCKED 4
#define GESTURE_STATE_PAD23_LOCKED 5

const unsigned long longPressTime = 500;

unsigned long gestureStartTime = 0;
int gestureState = GESTURE_STATE_NONE;

static int getGesture() {
    while(1) {
        bool p1 = display.readTouchpad(PAD1);
        bool p2 = display.readTouchpad(PAD2);
        bool p3 = display.readTouchpad(PAD3);
        bool p4 = display.digitalReadMCP(13);
        if (!p4) p3 = true;
        
        if (gestureState == GESTURE_STATE_NONE) {
            if (p1) gestureState = GESTURE_STATE_PAD1_PRESSED;
            if (p2) gestureState = GESTURE_STATE_PAD2_PRESSED;
            if (p3) gestureState = GESTURE_STATE_PAD3_PRESSED;
            if (gestureState) gestureStartTime = millis();
            continue;
        }

        unsigned long duration = millis() - gestureStartTime;

        if (gestureState == GESTURE_STATE_PAD1_PRESSED) {    
            if (p1) {
                if (p2) {
                    gestureState = GESTURE_STATE_PAD12_LOCKED;
                    continue;
                }
                if (duration > longPressTime) return GESTURE_UP_LONG;
            } else {
                gestureState = GESTURE_STATE_NONE;
                if (duration < longPressTime) return GESTURE_UP;
            }
        } else if (gestureState == GESTURE_STATE_PAD2_PRESSED) {
            if (p2) {
                if (p1) {
                    gestureState = GESTURE_STATE_PAD12_LOCKED;
                    continue;
                }
                if (p3) {
                    gestureState = GESTURE_STATE_PAD23_LOCKED;
                    continue;
                }
                if (duration > longPressTime) return GESTURE_SELECT_LONG;
            } else {
                gestureState = GESTURE_STATE_NONE;
                if (duration < longPressTime) return GESTURE_SELECT;
            }
        } else if (gestureState == GESTURE_STATE_PAD3_PRESSED) {
            if (p3) {
                if (p2) {
                    gestureState = GESTURE_STATE_PAD23_LOCKED;
                    continue;
                }
                if (duration > longPressTime) return GESTURE_DOWN_LONG;
            } else {
                gestureState = GESTURE_STATE_NONE;
                if (duration < longPressTime) return GESTURE_DOWN;
            }
        } else if (gestureState == GESTURE_STATE_PAD12_LOCKED) {
            if (p1 && p2) return GESTURE_UP_DOUBLE;
            else if (!p1 && !p2) gestureState = GESTURE_STATE_NONE;
        } else if (gestureState == GESTURE_STATE_PAD23_LOCKED) {
            if (p2 && p3) return GESTURE_DOWN_DOUBLE;
            else if (!p2 && !p3) gestureState = GESTURE_STATE_NONE;
        }
    }
}
