#ifndef LED_HPP
#define LED_HPP

namespace Led {
    
    enum class State {
        GPS_SEARCHING,
        GPS_LOCK,
        TRANSMITTING,
        ERROR,
        OFF
    };

    void begin();
    void update();
    void setState(State state);
}

#endif // LED_HPP