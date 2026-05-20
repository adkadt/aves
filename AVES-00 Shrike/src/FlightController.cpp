#include "FlightController.h"
#include <cmath>

FlightController::FlightController(Altimeter& altimeter, DataLogger& logger, uint8_t droguePin, uint8_t mainPin)
    : m_altimeter(altimeter), m_logger(logger), m_droguePin(droguePin), m_mainPin(mainPin) {}

State FlightController::update(double agl, bool altitudeUpdate) {
    switch (m_currentState) {
        case State::PAD_IDLE:           handlePadIdle(agl, altitudeUpdate);             break;
        case State::BOOST:              handleBoost(agl, altitudeUpdate);               break;
        case State::COAST:              handleCoast(agl, altitudeUpdate);               break;
        case State::DROGUE_DEPLOYMENT:  handleDrogueDeployment(agl, altitudeUpdate);    break;
        case State::DROGUE_DESCENT:     handleDrogueDescent(agl, altitudeUpdate);       break;
        case State::MAIN_DEPLOYMENT:    handleMainDeployment(agl, altitudeUpdate);      break;
        case State::MAIN_DESCENT:       handleMainDescent(agl, altitudeUpdate);         break;
        case State::LANDED:             handleLanded(agl, altitudeUpdate);              break;
    }
    return m_currentState;
}

State FlightController::getState() const {
    return m_currentState; 
}

double FlightController::getApogee() const {
    return m_apogee;
}

void FlightController::transitionState(State next_state) {
    m_stateCounter = 0;
    m_stateStartMs = millis();
    m_stateInit = false;
    m_currentState = next_state;
    Serial.printf("STATE-CHANGE: %d\n", static_cast<int>(next_state));
}

void FlightController::handlePadIdle(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
        if (agl > 4.5) {
            m_stateCounter++;
            if (m_stateCounter >= 10) {
                m_altimeter.setGroundMode(false); // Lock in ground altitude
                transitionState(State::BOOST);
            }
        } else {
            m_stateCounter = 0;
        }
    }
}

void FlightController::handleBoost(double agl, bool altitudeUpdate) {
    if (millis() - m_stateStartMs > BOOST_TIME_MS) {
        transitionState(State::COAST);
    }
}

void FlightController::handleCoast(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
        if (agl > m_apogee) {
            m_apogee = agl;
        }
        
        if (agl < (m_apogee - DROGUE_DROP_M)) {
            m_stateCounter++;
            if (m_stateCounter >= 10) {
                transitionState(State::DROGUE_DEPLOYMENT);
            }
        } else {
            m_stateCounter = 0;
        }
    }
}

void FlightController::handleDrogueDeployment(double agl, bool altitudeUpdate) {
    if (!m_stateInit) {
        if (m_apogee < MIN_DEPLOYMENT_ALT) {
            Serial.println("WARN: Drogue deployment aborted");
            transitionState(State::DROGUE_DESCENT);
            return;
        }
        digitalWrite(m_droguePin, HIGH);
        m_stateStartMs = millis();
        m_stateInit = true;
    } else if (millis() - m_stateStartMs >= FIRE_TIME_MS) {
        digitalWrite(m_droguePin, LOW);
        transitionState(State::DROGUE_DESCENT);
    }
}

void FlightController::handleDrogueDescent(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
        if (agl < MAIN_DEPLOYMENT_ALT) {
            m_stateCounter++;
            if (m_stateCounter >= 3) {
                transitionState(State::MAIN_DEPLOYMENT);
            }
        } else {
            m_stateCounter = 0;
        }
    }
}

void FlightController::handleMainDeployment(double agl, bool altitudeUpdate) {
    if (!m_stateInit) {
        if (m_apogee < MIN_DEPLOYMENT_ALT) {
            Serial.println("WARN: Main deployment aborted");
            transitionState(State::MAIN_DESCENT);
            return;
        }
        digitalWrite(m_mainPin, HIGH);
        m_stateStartMs = millis();
        m_stateInit = true;
    } else if (millis() - m_stateStartMs >= FIRE_TIME_MS) {
        digitalWrite(m_mainPin, LOW);
        transitionState(State::MAIN_DESCENT);
    }
}

void FlightController::handleMainDescent(double agl, bool altitudeUpdate) {
    static double lastAgl = 0.0;
    static unsigned long lastCheck_ms = 0;

    if (!m_stateInit) {
        lastAgl = agl;
        lastCheck_ms = millis();
        m_stateInit = true;
    }
    
    if (millis() - lastCheck_ms >= 1000) {
        double verticalDiff = std::abs(agl - lastAgl);

        if (verticalDiff < 2.0) {
            m_stateCounter++;
            if (m_stateCounter >= 3) {
                transitionState(State::LANDED);
            }
        } else {
            m_stateCounter = 0;
        }

        lastAgl = agl;
        lastCheck_ms = millis();
    }
}

void FlightController::handleLanded(double agl, bool altitudeUpdate) {
    if (!m_stateInit) {
        if (m_logger.isActive()) {
            m_logger.close();
            Serial.println("FLIGHT LOG SAVED");
        }
        m_stateInit = true;
    }
}
