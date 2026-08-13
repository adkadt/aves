#include <Adafruit_NeoPixel.h>
#include <cstdint>

#include "system/system.hpp"
#include "hardware/led.hpp"
#include "config.hpp"

namespace {
    Adafruit_NeoPixel boardLed = Adafruit_NeoPixel(1, Pins::LED, NEO_RGB + NEO_KHZ800);
    
    constexpr uint32_t BLINK_INTERVAL = 1000;

    constexpr uint32_t FADE_INTERVAL_MS = 20;
    constexpr uint8_t FADE_AMOUNT = 2;

    constexpr uint8_t BRIGHTNESS_MAX = 50; // 255
    constexpr uint8_t BRIGHTNESS_MIN = 0; // 0
    
    // State Colors
    constexpr uint32_t COLOR_STARTUP = 0x0000FFu; // blue
    constexpr uint32_t COLOR_GPS_SEARCHING = 0x0000FFu; // blue
    constexpr uint32_t COLOR_GPS_LOCK = 0x00FF00u; // green
    constexpr uint32_t COLOR_OFF = 0x000000u; // black
    
    // Error Colors
    constexpr uint32_t COLOR_ERROR = 0xFF0000u; // red
    
    // Warning Colors
    constexpr uint32_t COLOR_WARNING = 0xFFBF00u; // amber

    // Event Colors
    constexpr uint32_t COLOR_LORA_TX = 0x00BFFFu; // cyan
    constexpr uint32_t COLOR_LORA_RX = 0xFF8000u; // orange
    
    enum class Pattern {
        NONE,
        SOLID,
        BLINK,
        PULSE,
        FLASH,
        SEQUENCE
    };

    struct Indication {
        Pattern pattern = Pattern::NONE;
        uint32_t color = COLOR_OFF;
        uint32_t duration = 0;
        uint32_t start = 0;
    };

    struct LedRuntime  {
        uint32_t lastUpdate = 0;
        uint32_t lastFadeUpdate = 0;

        uint8_t brightness = BRIGHTNESS_MIN;
        bool fadingUp = true;
    };

    LedRuntime runtime;
    Indication indication;

    void setIndication(Pattern pattern, uint32_t color, uint32_t duration = 0);
    void updateIndication();

    void updateError();
    void updateWarning();
    void handleEvents();
    void updateState();

    // helpers
    void updateSolid();
    void updateBlink();
    void updatePulse();
    void updateFlash();
    void updateSequence();

}

void Led::begin() {
    pinMode(Pins::LED, OUTPUT);

    boardLed.begin();
    boardLed.show();

    runtime = {};
    indication = {};
}

void Led::update() {
    uint32_t now = millis();

    // handle temporary indication
    if (indication.duration > 0) {
        if (now - indication.start < indication.duration) {
            updateIndication();
            return;
        } else {
            indication = {};
        }
    }

    if (System::hasError()) {
        // errors
        updateError();
    } else if (System::hasWarning()) {
        // warnings
        updateWarning();
    } else if (System::hasEvent()) {
        // handle new events
        handleEvents();
    } else {
        // normal state
        updateState();
    }

    // Handle current event indication
    updateIndication();
}


// private implementation
namespace {
    void setIndication(Pattern pattern, uint32_t color, uint32_t duration) {
        if (indication.pattern == pattern &&
            indication.color == color &&
            indication.duration == duration
        ) {
            return;
        }

        uint32_t now = millis();

        indication.pattern = pattern;
        indication.color = color;
        indication.duration = duration;
        indication.start = now;

        runtime.lastUpdate = now;
        runtime.lastFadeUpdate = now;
        runtime.brightness = BRIGHTNESS_MIN;
        runtime.fadingUp = true;
    }

    void updateIndication() {
        switch (indication.pattern) {
            case Pattern::SOLID:
                updateSolid();
                break;

            case Pattern::BLINK:
                updateBlink();
                break;

            case Pattern::PULSE:
                updatePulse();
                break;
                
            case Pattern::FLASH:
                updateFlash();
                break;

            case Pattern::SEQUENCE:
                updateSequence();
                break;

            case Pattern::NONE:
                break;
        }
    }

    void updateError() {
        
        if (System::hasError(System::Error::GPS_DISCONNECTED)) {
            setIndication(Pattern::SOLID, COLOR_ERROR);
            return;
        } 
        
        if (System::hasError(System::Error::LORA_DISCONNECTED)) {
            setIndication(Pattern::BLINK, COLOR_ERROR);
            return;
        } 
        
        if (System::hasError(System::Error::BATTERY_CRITICAL)) {
            setIndication(Pattern::PULSE, COLOR_ERROR);
            return;
        }
    }
    
    void updateWarning() {
        if (System::hasWarning(System::Warning::GPS_WEAK_SIGNAL)) {
            setIndication(Pattern::SOLID, COLOR_WARNING);
            return;
        } 
        
        if (System::hasWarning(System::Warning::LORA_WEAK_SIGNAL)) {
            setIndication(Pattern::BLINK, COLOR_WARNING);
            return;
        } 
        
        if (System::hasWarning(System::Warning::BATTERY_LOW)) {
            setIndication(Pattern::PULSE, COLOR_WARNING);
            return;
        }
    }
  
    void handleEvents() {
        if (System::hasEvent(System::Event::LORA_RECEIVED)) {
            setIndication(Pattern::FLASH, COLOR_LORA_RX, 200);
            return;
        } 
        
        if (System::hasEvent(System::Event::LORA_TRANSMIT_STARTED)) {
            setIndication(Pattern::FLASH, COLOR_LORA_TX, 200);
            return;
        }
    }

    void updateState() {
        switch (System::getState()) {
            case System::State::STARTUP:
                ::setIndication(Pattern::SOLID, COLOR_STARTUP);
                break;
            case System::State::GPS_SEARCHING:
                ::setIndication(Pattern::PULSE, COLOR_GPS_SEARCHING);
                break;
            case System::State::GPS_LOCK:
                ::setIndication(Pattern::SOLID, COLOR_GPS_LOCK);
                break;
            default:
                ::setIndication(Pattern::SOLID, COLOR_OFF);
                break;
        }
    }

    void updateSolid() {
        boardLed.setPixelColor(0, indication.color);
        boardLed.setBrightness(BRIGHTNESS_MAX);
        boardLed.show();
    }

    void updateBlink() {
        uint32_t now = millis();

        if (now - runtime.lastUpdate >= BLINK_INTERVAL) {
            runtime.lastUpdate = now;

            if (runtime.brightness <= BRIGHTNESS_MIN) {
                runtime.brightness = BRIGHTNESS_MAX;
            } else {
                runtime.brightness = BRIGHTNESS_MIN;
            }
       
            boardLed.setPixelColor(0, indication.color);
            boardLed.setBrightness(runtime.brightness);
            boardLed.show();
        }
    }

    void updatePulse() {
        uint32_t now = millis();
            
        if (now - runtime.lastFadeUpdate >= FADE_INTERVAL_MS) {
            runtime.lastFadeUpdate = now;

            if (runtime.fadingUp) {
                if (runtime.brightness > BRIGHTNESS_MAX - FADE_AMOUNT) {
                    runtime.brightness = BRIGHTNESS_MAX;
                    runtime.fadingUp = false;
                } else {
                    runtime.brightness += FADE_AMOUNT;
                }
            } else {
                if (runtime.brightness < BRIGHTNESS_MIN + FADE_AMOUNT) {
                    runtime.brightness = BRIGHTNESS_MIN;
                    runtime.fadingUp = true;
                } else {
                    runtime.brightness -= FADE_AMOUNT;
                }
            }
            boardLed.setPixelColor(0, indication.color);
            boardLed.setBrightness(runtime.brightness);
            boardLed.show();
        }
    }

    void updateFlash() {  
        uint32_t now = millis();

        if (now - indication.start < indication.duration) {
            boardLed.setPixelColor(0, indication.color);
            boardLed.setBrightness(BRIGHTNESS_MAX);
            boardLed.show();
        }
    }

    void updateSequence() {
        // todo later
    }
}