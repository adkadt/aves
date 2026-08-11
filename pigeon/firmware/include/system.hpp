#pragma once

namespace System {
    enum class State {
        STARTUP,
        GPS_SEARCHING,
        GPS_LOCK,
        TRANSMITTING,
        ERROR
    };

    enum class Mode {
        FLIGHT,
        GROUND
    };

    void begin();
    void update();

    State getState();
    Mode getMode();

    void setState(State state);

    bool stateChanged();
    bool modeChanged();
}
