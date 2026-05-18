#include <Arduino.h>
#include <optional>
#include "BmpManager.h"
#include "Altimeter.h"

#define BOOST_TIME 4 // seconds to ignore pressure for apogee readings
#define DROGUE_DROP_M 3 // the meters vertically down from apogee to deploy drogue chute

enum class State: uint8_t {
    PAD_IDLE,
    BOOST,
    COAST,
    DROGUE_DEPLOYMENT,
    MAIN_DEPLOYMENT,
    LANDED
};
State current_state = State::PAD_IDLE;

BmpManager bmp;
Altimeter altimeter;

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // !Important! [REMOVE FOR FLIGHT] Waits for serial

    if (!bmp.begin()) {
        Serial.println("--BMP FAILED TO START--");
    }

    bmp.setMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_240_HZ);
    bmp.setTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setPressureOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setIIRCoeff(BMP5XX_IIR_FILTER_COEFF_3);

    altimeter.setGroundMode(true);
}

double apogee = 0.0;

double boostStart_ms;
int stateCounter = 0;

void loop() {
    bool bmp_update = false;
    
    // gather raw altitude data
    BmpData raw_bmp_data;
    if (bmp.update()) {
        raw_bmp_data = bmp.getBmpData();
        bmp_update = true;
    }

    if (bmp_update) {
        altimeter.update(raw_bmp_data.altitude);
        double agl = altimeter.getAltitudeAGL();

        // enter current state
        switch (current_state) {
            case State::PAD_IDLE:
                if (agl > 15) {
                    stateCounter++;
                    if (stateCounter >= 3) {
                        current_state = State::BOOST;
                        boostStart_ms = millis();
                        stateCounter = 0;
                        Serial.println("STATE-CHANGE: BOOST");
                    }
                } else {
                    stateCounter = 0;
                }
                break;

            case State::BOOST:
                if (millis() - boostStart_ms > BOOST_TIME*1000) {
                    current_state = State::COAST;
                    Serial.println("STATE-CHANGE: COAST");
                }
                break;

            case State::COAST:
                if (agl > apogee) {
                    apogee = agl;
                }

                if (agl < (apogee - DROGUE_DROP_M)) {
                    stateCounter++;

                    if (stateCounter >= 3) {
                        // deploy drogue
                        stateCounter = 0;
                        current_state = State::DROGUE_DEPLOYMENT;
                        Serial.println("STATE-CHANGE: DROGUE_DEPLOYMENT");
                    }
                } else {
                    stateCounter = 0;
                }

                break;

            case State::DROGUE_DEPLOYMENT:
                break;
            case State::MAIN_DEPLOYMENT:
                break;
            case State::LANDED:
                break;
            default:
                break;
        }

        // push info to flash or SD card

    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 1000) {
        last_ping = millis();
        Serial.printf("Alive Ping: %d\n", millis() / 1000);
    }
}