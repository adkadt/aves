#include "system/system.hpp"
#include "config.hpp"

#include <Arduino.h>
#include <cstdint>

namespace {
    System::Mode readModeSwitch();
    uint32_t errorMask(System::Error error);
    uint32_t warningMask(System::Warning warning);
    uint32_t eventMask(System::Event event);

    System::Mode lastMode = Config::PRIMARY_MODE;
    System::Mode currentMode = Config::SECONDARY_MODE;
    
    System::State lastState = System::State::OFF;
    System::State currentState = System::State::STARTUP;
    System::State nextState = System::State::STARTUP;
    
    uint32_t currentErrors = 0;
    uint32_t nextErrors = 0;

    uint32_t currentWarnings = 0;
    uint32_t nextWarnings = 0;

    uint32_t currentEvents = 0;
    uint32_t nextEvents = 0;
}

void System::begin() {
    pinMode(Pins::MODE_SWITCH, INPUT);

    // Mode
    lastMode = readModeSwitch();
    currentMode = lastMode;

    // State
    lastState = State::OFF;
    currentState = State::STARTUP;
    nextState = State::STARTUP;

    // Error
    currentErrors = 0;
    nextErrors = 0;

    // Warnings
    currentWarnings = 0;
    nextWarnings = 0;

    // Event
    currentEvents = 0;
    nextEvents = 0;
}


void System::update() {
    lastMode = currentMode;
    currentMode = readModeSwitch();

    lastState = currentState;
    currentState = nextState;

    currentErrors = nextErrors;

    currentWarnings = nextWarnings;

    currentEvents = nextEvents;
    nextEvents = 0;
}


// Mode Functions
System::Mode System::getMode() {
    return currentMode;
}

bool System::modeChanged() {
    return lastMode != currentMode;
}

System::Mode System::getPreviousMode() {
    return lastMode;
}


// State Functions
System::State System::getState() {
    return currentState;
}

void System::setState(System::State state) {
    nextState = state;
}

bool System::stateChanged() {
    return lastState != currentState;
}

System::State System::getPreviousState() {
    return lastState;
}


// Error Functions 
void System::setError(System::Error error) {
    nextErrors |= errorMask(error);
    System::setState(System::State::ERROR);
}

void System::clearError(System::Error error) {
    nextErrors &= ~errorMask(error);
}

bool System::hasError(System::Error error) {
    return (currentErrors & errorMask(error)) != 0;
}

bool System::hasError() {
    return currentErrors != 0;
}


// Warning Functions
void System::setWarning(System::Warning warning) {
    nextWarnings |= warningMask(warning);
}

void System::clearWarning(System::Warning warning) {
    nextWarnings &= ~warningMask(warning);
}

bool System::hasWarning(System::Warning warning) {
    return (currentWarnings & warningMask(warning)) != 0;
}

bool System::hasWarning() {
    return currentWarnings != 0;
}


// Event Functions
void System::setEvent(System::Event event) {
    nextEvents |= eventMask(event);
}

bool System::hasEvent(System::Event event) {
    return (currentEvents & eventMask(event)) != 0;
}

bool System::hasEvent() {
    return currentEvents != 0;
}


// private implementation
namespace {
    System::Mode readModeSwitch() {
        if (digitalRead(Pins::MODE_SWITCH) == HIGH)
            return Config::PRIMARY_MODE;
        else
            return Config::SECONDARY_MODE;
    }

    uint32_t errorMask(System::Error error) {
        if (error == System::Error::NONE)
        return 0;
        
        return 1u << (static_cast<uint8_t>(error) - 1);
    }
    
    uint32_t warningMask(System::Warning warning) {
        if (warning == System::Warning::NONE)
        return 0;
        
        return 1u << (static_cast<uint8_t>(warning) - 1);
    }

    uint32_t eventMask(System::Event event) {
        if (event == System::Event::NONE)
            return 0;
    
        return 1u << (static_cast<uint8_t>(event) - 1);
    }
}