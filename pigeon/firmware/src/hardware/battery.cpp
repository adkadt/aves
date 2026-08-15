#include "system/system.hpp"
#include "config.hpp"
#include "hardware/battery.hpp"

#include <Arduino.h>

namespace {
    // ADC
    constexpr adc_attenuation_t ADC_ATTENUATION = ADC_11db;

    Battery::Status batStatus = Battery::Status::UNKNOWN_VOLTAGE;

    void updateStatus();

    float calcBatteryVoltage(float senseVoltage);
    void filterBatteryVoltage(float batteryVoltage);

    float filteredBatteryVoltage;
}

void Battery::begin() {
    pinMode(Pins::VBAT_SENSE, INPUT);
    
    analogSetPinAttenuation(
        Pins::VBAT_SENSE, 
        ADC_ATTENUATION
    );

    Battery::update();
}

void Battery::update() {
    uint32_t senseMillivolts  = analogReadMilliVolts(Pins::VBAT_SENSE);
    float senseVoltage = senseMillivolts / 1000.0f;

    float batteryVoltage = calcBatteryVoltage(senseVoltage);

    filterBatteryVoltage(batteryVoltage);
    updateStatus();
}

float Battery::getVoltage() {
    return filteredBatteryVoltage;
}

Battery::Status Battery::getStatus() {
    return batStatus;
}

namespace {
    void updateStatus() {
        if (filteredBatteryVoltage >= Config::Battery::MIN_USB_VOLTAGE) {
            batStatus = Battery::Status::USB_VOLTAGE;
        } else if (filteredBatteryVoltage >= Config::Battery::LOW_VOLTAGE) {
            batStatus = Battery::Status::NOMINAL_VOLTAGE;
        } else if (filteredBatteryVoltage >= Config::Battery::CRITICAL_VOLTAGE) {
            batStatus = Battery::Status::LOW_VOLTAGE;
        } else if (filteredBatteryVoltage < Config::Battery::MEASURE_THRESHOLD_VOLTAGE) {
            batStatus = Battery::Status::UNKNOWN_VOLTAGE;
        } else {
            batStatus = Battery::Status::CRITICAL_VOLTAGE;
        }
    }

    float calcBatteryVoltage(float senseVoltage) {
        return senseVoltage *
               (Config::Battery::DIVIDER_TOP_RESISTOR + 
                Config::Battery::DIVIDER_BOTTOM_RESISTOR) /
                Config::Battery::DIVIDER_BOTTOM_RESISTOR;
    }

    void filterBatteryVoltage(float batteryVoltage) {
        if (batStatus == Battery::Status::UNKNOWN_VOLTAGE) {
            filteredBatteryVoltage = batteryVoltage;
            return;
        }

        filteredBatteryVoltage = (batteryVoltage * Config::Battery::VOLTAGE_FILTER_ALPHA ) +
            (filteredBatteryVoltage * (1 - Config::Battery::VOLTAGE_FILTER_ALPHA ));
    }
}
