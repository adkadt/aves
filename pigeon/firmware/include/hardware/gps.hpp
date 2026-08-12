#ifndef GPS_HPP
#define GPS_HPP

#include <cstdint>
#include "types.hpp"

namespace Gps {

    enum class Status {
        DISCONNECTED,
        SEARCHING,
        LOCKED
    };

    struct Position {
        double latitude;
        double longitude;
        float altitude;
    };

    struct Data {
        Position rawPosition;
        Position position;
        
        float speed;
        float course;

        uint8_t satellites;
        float hdop;

        DateTime utcTime;
    };

    void begin();
    void update();

    Status getStatus();
    const Data& getData();
}

#endif // GPS_HPP