#pragma once

#include <cstdint>

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
    enum class Error : uint8_t {
        NONE = 0,

        // GPS
        GPS_DISCONNECTED,
        
        // LoRa
        LORA_DISCONNECTED,        

        // Battery
        BATTERY_CRITICAL
    };
    
    // Warnings
    enum class Warning : uint8_t {
        NONE = 0,
        
        // GPS
        GPS_WEAK_SIGNAL,
        
        // LoRa
        LORA_WEAK_SIGNAL,
        
        // Battery
        BATTERY_LOW
    };
    
    // Events
    enum class Event : uint8_t {
        NONE = 0,
        
        // LoRa
        LORA_TRANSMIT_STARTED,
        LORA_TRANSMIT_FINISHED,
        LORA_TRANSMIT_FAILED,
        LORA_RECEIVED,
        LORA_RECEIVE_FAILED,
        LORA_PACKET_INVALID,
        LORA_PACKET_TOO_LARGE,
        
        // GPS
        GPS_LOCKED,
        GPS_LOST,
        GPS_TIMEOUT,
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
    void setError(Error error);
    void clearError(Error error);
    bool hasError(Error error);
    bool hasError();

    // Warnings
    void setWarning(Warning warning);
    void clearWarning(Warning warning);
    bool hasWarning(Warning warning);
    bool hasWarning();

    // Events
    void setEvent(Event event);
    bool hasEvent(Event event);
    bool hasEvent();
}
