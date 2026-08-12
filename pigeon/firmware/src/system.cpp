#include "system.hpp"
#include <cstdint>

namespace {
    System::Mode readModeSwitch();
    uint32_t eventMask(System::Event event);

    System::Mode lastMode = System::Mode::FLIGHT;
    System::Mode currentMode = System::Mode::FLIGHT;
    
    System::State lastState = System::State::OFF;
    System::State currentState = System::State::STARTUP;
    System::State nextState = System::State::STARTUP;
    
    System::Error lastError = System::Error::NONE;
    System::Error currentError = System::Error::NONE;
    System::Error nextError = System::Error::NONE;

    uint32_t currentEvents = 0;
    uint32_t nextEvents = 0;
}

void System::begin() {
    // Mode
    lastMode = readModeSwitch();
    currentMode = lastMode;

    // State
    lastState = State::OFF;
    currentState = State::STARTUP;
    nextState = State::STARTUP;

    // Error
    lastError = System::Error::NONE;
    currentError = System::Error::NONE;
    nextError = System::Error::NONE;
}


void System::update() {
    lastMode = currentMode;
    currentMode = readModeSwitch();

    lastState = currentState;
    currentState = nextState;
    
    lastError = currentError;
    currentError = nextError;

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

    if (nextState != System::State::ERROR)
        System::setError(System::Error::NONE);
}

bool System::stateChanged() {
    return lastState != currentState;
}

System::State System::getPreviousState() {
    return lastState;
}


// Error Functions 
System::Error System::getError() {
    return currentError;
}

void System::setError(System::Error error) {
    nextError = error;

    if (nextError != System::Error::NONE)
        System::setState(System::State::ERROR);
}

bool System::errorChanged() {
    return lastError != currentError;
}

System::Error System::getPreviousError() {
    return lastError;
}


// Event Functions


void System::setEvent(System::Event event) {
    nextEvents |= eventMask(event);
}

bool System::hasEvent(System::Event event) {
    return (currentEvents & eventMask(event)) != 0;
}


// private implementation
namespace {
    System::Mode readModeSwitch() {
        // eventually add mode switch reading.
        return System::Mode::FLIGHT;
    }

    uint32_t eventMask(System::Event event) {
        if (event == System::Event::NONE)
            return 0;
    
        return 1u << (static_cast<uint8_t>(event) - 1);
    }
}