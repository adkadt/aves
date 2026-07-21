#include <Arduino.h>
#include "config.hpp"

#include "led.hpp"

void setup() {
    Serial.begin(Config::BAUD_RATE);
    Led::begin();
}

void loop() {
    unsigned long start;

    start = millis();
    Serial.println("GPS_SEARCHING");
    Led::setState(Led::State::GPS_SEARCHING);
    while (millis() - start < 10000) {
        Led::update();
    }

    start = millis();
    Serial.println("GPS_LOCK");
    Led::setState(Led::State::GPS_LOCK);
    while (millis() - start < 10000) {
        Led::update();
    }

    start = millis();
    Serial.println("TRANSMITTING");
    Led::setState(Led::State::TRANSMITTING);
    while (millis() - start < 10000) {
        Led::update();
    }

    start = millis();
    Serial.println("ERROR");
    Led::setState(Led::State::ERROR);
    while (millis() - start < 10000) {
        Led::update();
    }
    
    start = millis();
    Serial.println("OFF");
    Led::setState(Led::State::OFF);
    while (millis() - start < 5000) {
        Led::update();
    }    
}