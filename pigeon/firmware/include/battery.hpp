#pragma once

#include <cstdint>

namespace Battery {

    enum class Status {
        UNKNOWN_VOLTAGE,
        USB_VOLTAGE,
        NOMINAL_VOLTAGE,
        LOW_VOLTAGE,
        CRITICAL_VOLTAGE
    };

    void begin();
    void update();

    float getVoltage();
    Status getStatus();
}