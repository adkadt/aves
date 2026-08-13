#include <Arduino.h>
#include <SPI.h>

#include "config.hpp"

#include "system/system.hpp"
#include "modes/modes.hpp"

#include "hardware/led.hpp"
#include "hardware/buzzer.hpp"
#include "hardware/gps.hpp"
#include "hardware/lora.hpp"
#include "hardware/battery.hpp"

void updateSystemState();
void updateMode();

void setup() {
    System::begin();
    Led::begin();
    Buzzer::begin();
    
    Serial.begin(Config::BAUD_RATE);
    // while (!Serial) {};
    Serial.println("Serial initialized");

    SPI.begin(Pins::LORA_SCK, Pins::LORA_MISO, Pins::LORA_MOSI);
    
    Battery::begin();
    Gps::begin();
    LoRa::begin();

    // start system mode
    switch (System::getMode()) {
        case System::Mode::FLIGHT:
            Mode::Flight::begin();
            break;

        case System::Mode::GROUND:
            Mode::Ground::begin();
            break;

        case System::Mode::DEBUG:
            Mode::Debug::begin();
            break;

        default:
            break;
    }
}



void loop() {
    // update all systems
    System::update();
    Led::update();
    Buzzer::update();
    
    Battery::update();
    Gps::update();
    LoRa::update();

    // updated overall system state
    updateSystemState();

    // run mode loop
    updateMode();
}

void updateSystemState() {
    switch (Gps::getStatus()) {
        case Gps::Status::DISCONNECTED:
            System::setError(System::Error::GPS_DISCONNECTED);
            break;
        case Gps::Status::LOCKED:
            System::setState(System::State::GPS_LOCK);
            break;
        case Gps::Status::SEARCHING:
            System::setState(System::State::GPS_SEARCHING);
            break;
        default:
            break;
    }

    switch (LoRa::getStatus()) {
        case LoRa::Status::DISCONNECTED:
            System::setError(System::Error::LORA_DISCONNECTED);
            break;
        default:
            break;
    }

    switch (Battery::getStatus()) {
        case Battery::Status::CRITICAL_VOLTAGE:
            System::setError(System::Error::BATTERY_CRITICAL);
            break;
        case Battery::Status::LOW_VOLTAGE:
            System::setWarning(System::Warning::BATTERY_LOW);
            break;
        default:
            break;
    }
}

void updateMode() {
    // start mode upon switch
    if (System::modeChanged()) {
        switch (System::getMode()) {
            case System::Mode::FLIGHT:
                Mode::Flight::begin();
                break;
            case System::Mode::GROUND:
                Mode::Ground::begin();
                break;
            case System::Mode::DEBUG:
                Mode::Debug::begin();
                break;
            default:
                break;
        }
    }

    // run mode loop
    switch (System::getMode()) {
        case System::Mode::FLIGHT:
            Mode::Flight::update();
            break;
            
        case System::Mode::GROUND:
            Mode::Ground::update();
            break;
            
        case System::Mode::DEBUG:
            Mode::Debug::update();
            break;
        
        default:
            break;
    }
}