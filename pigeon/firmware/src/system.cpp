#include "system.hpp"

namespace {
    System::State lastState = System::State::STARTUP;
    System::State currentState = System::State::STARTUP;
    System::State nextState = System::State::STARTUP;

    System::Mode lastMode = System::Mode::FLIGHT;
    System::Mode currentMode = System::Mode::FLIGHT;

    System::Mode readModeSwitch();
}

void System::begin() {
    lastState = State::STARTUP;
    currentState = State::STARTUP;
    nextState = State::STARTUP;

    lastMode = readModeSwitch();
    currentMode = lastMode;
}

void System::update() {
    lastState = currentState;
    currentState = nextState;
    
    lastMode = currentMode;
    currentMode = readModeSwitch();
}

System::State System::getState() {
    return currentState;
}

System::Mode System::getMode() {
    return currentMode;
}

void System::setState(System::State state) {
    nextState = state;
}

bool System::stateChanged() {
    return lastState != currentState;
}

bool System::modeChanged() {
    return lastMode != currentMode;
}

namespace {
    System::Mode readModeSwitch() {
        // eventually add mode switch reading.
        return System::Mode::FLIGHT;
    }
}