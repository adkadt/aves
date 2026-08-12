#pragma once

namespace System {
    void begin();
    void update();

    // Modes
    enum class Mode {
        FLIGHT,
        GROUND
    };

    // States
    enum class State {
        OFF,
        STARTUP,
        GPS_SEARCHING,
        GPS_LOCK,
        TRANSMITTING,
        ERROR
    };

    // Errors
    enum class Error {
        NONE,

        // GPS
        GPS_DISCONNECTED,
        GPS_TIMEOUT,

        // LoRa
        LORA_DISCONNECTED,
        LORA_RECEIVE_FAILED,
        LORA_TRANSMIT_FAILED,
        LORA_PACKET_TOO_LARGE
    };

    // Events
    enum class Event : uint32_t {
        NONE = 0,

        // LoRa
        LORA_TRANSMITTING,
        LORA_TRANSMITTED,
        LORA_RECEIVED,
        LORA_PACKET_INVALID,
        LORA_PACKET_TOO_LARGE,

        // GPS
        GPS_LOCKED,
        GPS_LOST,

        // Battery
        BATTERY_LOW,
        BATTERY_CRITICAL
    };

    // Modes
    Mode getMode();
    bool modeChanged();
    Mode getPreviousMode();

    // States
    State getState();
    void setState(State state);
    bool stateChanged();
    State getPreviousState();

    // Errors
    Error getError();
    void setError(Error error);
    bool errorChanged();
    Error getPreviousError();

    // Events
    void setEvent(Event event);
    bool hasEvent(Event event);
}
