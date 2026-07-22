#ifndef GPS_HPP
#define GPS_HPP

#include <cstdint>
#include "types.hpp"

namespace Gps {

    struct Position {
        double latitude;
        double longitude;
        float altitude;
    };

    struct Data {
        Position position;
        
        float speed;
        float course;

        uint8_t satellites;
        float hdop;

        DateTime utcTime;
    };

    void begin();
    void update();
    
    bool hasFix();
    bool isConnected();

    const Data& getData();
}

#endif // GPS_HPP