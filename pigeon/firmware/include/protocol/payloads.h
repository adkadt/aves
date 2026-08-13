#pragma once

#include <stdint.h>

namespace Payload {

    struct __attribute__((packed)) Heartbeat {
        uint32_t uptime;

        uint8_t mode;
        uint8_t state;
        uint8_t batteryStatus;
        uint8_t gpsStatus;

        uint32_t errors;
        uint32_t warnings;
    };

    struct __attribute__((packed)) GpsTelemetry {
        // Position
        int32_t latitude;
        int32_t longitude;
        int32_t altitude;

        // Movement
        uint32_t speed;
        uint32_t course;

        // GPS quality
        uint8_t satellites;
        uint8_t status;
    };
}