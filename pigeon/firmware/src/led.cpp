#include <Adafruit_NeoPixel.h>
#include <cstdint>

#include "system.hpp"
#include "led.hpp"
#include "config.hpp"

namespace {
    Adafruit_NeoPixel boardLed = Adafruit_NeoPixel(1, Pins::LED, NEO_RGB + NEO_KHZ800);
    
    constexpr uint32_t FADE_INTERVAL_MS = 20;
    constexpr uint8_t FADE_AMOUNT = 2;
    constexpr uint8_t BRIGHTNESS_MAX = 150; // 255
    constexpr uint8_t BRIGHTNESS_MIN = 0; // 0
    
    // State Colors
    constexpr uint32_t COLOR_STARTUP = 0x0000FFu; // blue
    constexpr uint32_t COLOR_GPS_SEARCHING = 0x00FFFFu; // cyan
    constexpr uint32_t COLOR_GPS_LOCK = 0x00FF00u; // green
    constexpr uint32_t COLOR_TRANSMITTING = 0xFFFF00u; // yellow
    constexpr uint32_t COLOR_OFF = 0x000000u; // black
    
    // Error Colors
    constexpr uint32_t COLOR_ERROR = 0xFF0000u; // red
    
    // Warning Colors

    // Event Colors
    


    enum class Pattern {
        NONE,
        SOLID,
        BLINK,
        PULSE,
        FLASH,
        SEQUENCE
    };

    struct Indication {
        Pattern pattern;
        uint32_t color;
        uint32_t duration;
    };

    struct LedData {
        uint32_t lastUpdate;
        uint32_t lastFadeUpdate;

        uint8_t brightness;
        bool fadingUp;

        Indication indication;
        uint32_t indicationStart;
        uint32_t indicationDuration;
        uint32_t indicationColor;
    };
    LedData ledData;

    void solid(uint32_t color);
    void blink(uint32_t color);
    void pulse(uint32_t color);

    void flash(uint32_t color, uint32_t duration);

}

void Led::begin() {
    pinMode(Pins::LED, OUTPUT);

    boardLed.begin();
    boardLed.show();

    ledData.lastUpdate = 0;
    ledData.lastFadeUpdate = 0;
    ledData.brightness = 0;
    ledData.fadingUp = true;
}

void Led::update() {
    if (System::stateChanged()) {
        ledData.lastUpdate = millis();
        ledData.lastFadeUpdate = millis();
        ledData.brightness = BRIGHTNESS_MIN;
        ledData.fadingUp = true;
    }

    bool error_override = false

    switch (System::getState()) {
        case System::State::STARTUP:
            ::solid(COLOR_STARTUP);
            break;
        case System::State::GPS_SEARCHING:
            ::pulse(COLOR_GPS_SEARCHING);
            break;
        case System::State::GPS_LOCK:
            ::solid(COLOR_GPS_LOCK);
            break;
        case System::State::TRANSMITTING:
            ::blink(COLOR_TRANSMITTING);
            break;
        case System::State::ERROR:
            error_override = true;
            break;
        default:
            ::solid(COLOR_OFF);
            break;
    }

    if (error_override) {
    }
}


// private implementation
namespace {
    void solid(uint32_t color) {
        boardLed.setPixelColor(0, color);
        boardLed.setBrightness(BRIGHTNESS_MAX);
        boardLed.show();
    }

    void blink(uint32_t color) {
        uint32_t now = millis();

        if (now - ledData.lastUpdate >= 1000) {
            if (ledData.brightness <= BRIGHTNESS_MIN) {
                ledData.brightness = BRIGHTNESS_MAX;
            } else {
                ledData.brightness = BRIGHTNESS_MIN;
            }
            ledData.lastUpdate = now;
        }

        boardLed.setPixelColor(0, color);
        boardLed.setBrightness(ledData.brightness);
        boardLed.show();
    }

    void pulse(uint32_t color) {
        uint32_t now = millis();
            
        if (now - ledData.lastFadeUpdate >= FADE_INTERVAL_MS) {
            ledData.lastFadeUpdate = now;
            if (ledData.fadingUp) {
                if (ledData.brightness > BRIGHTNESS_MAX - FADE_AMOUNT) {
                    ledData.brightness = BRIGHTNESS_MAX;
                    ledData.fadingUp = false;
                } else {
                    ledData.brightness += FADE_AMOUNT;
                }
            } else {
                if (ledData.brightness < BRIGHTNESS_MIN + FADE_AMOUNT) {
                    ledData.brightness = BRIGHTNESS_MIN;
                    ledData.fadingUp = true;
                } else {
                    ledData.brightness -= FADE_AMOUNT;
                }
            }
        }

        boardLed.setPixelColor(0, color);
        boardLed.setBrightness(ledData.brightness);
        boardLed.show();
    }
}