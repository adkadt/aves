#ifndef FLIGHTCONTROLLER_H
#define FLIGHTCONTROLLER_H

#include <Arduino.h>
#include "Altimeter.h"
#include "DataLogger.h"

constexpr unsigned long BOOST_TIME_MS = 2000; // milliseconds to ignore pressure for apogee readings
constexpr double DROGUE_DROP_M = 3.0; // the meters vertically down from apogee to deploy drogue chute
constexpr unsigned long FIRE_TIME_MS = 2000;
constexpr double MAIN_DEPLOYMENT_ALT = 150.0; // meters AGL to deploy main chute at
constexpr double MIN_DEPLOYMENT_ALT = 30.0; // the minimum AGL allowed to deploy a chute at

enum class State: uint8_t {
    PAD_IDLE, 
    BOOST, 
    COAST, 
    DROGUE_DEPLOYMENT, 
    DROGUE_DESCENT, 
    MAIN_DEPLOYMENT, 
    MAIN_DESCENT, 
    LANDED
};

class FlightController {
private:
    Altimeter& m_altimeter;
    DataLogger& m_logger;
    uint8_t m_droguePin;
    uint8_t m_mainPin;

    State m_currentState = State::PAD_IDLE;
    int m_stateCounter = 0;
    bool m_stateInit = false;
    unsigned long m_stateStartMs = 0;
    double m_apogee = 0.0;

    void handlePadIdle(double agl, bool altitudeUpdate);
    void handleBoost(double agl, bool altitudeUpdate);
    void handleCoast(double agl, bool altitudeUpdate);
    void handleDrogueDeployment(double agl, bool altitudeUpdate);
    void handleDrogueDescent(double agl, bool altitudeUpdate);
    void handleMainDeployment(double agl, bool altitudeUpdate);
    void handleMainDescent(double agl, bool altitudeUpdate);
    void handleLanded(double agl, bool altitudeUpdate);
    void transitionState(State next_state);

public:
    FlightController(Altimeter& altimeter, DataLogger& logger, uint8_t droguePin, uint8_t mainPin);
    State update(double agl, bool altitudeUpdate);
    State getState() const;
    double getApogee() const;
};

#endif
