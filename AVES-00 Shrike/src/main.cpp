#include <Arduino.h>
#include <optional>
#include "BmpManager.h"
#include "Altimeter.h"
#include <SPI.h>
#include "SdFat.h"

#define BOOST_TIME_MS 4000 // seconds to ignore pressure for apogee readings
#define DROGUE_DROP_M 3 // the meters vertically down from apogee to deploy drogue chute
#define FIRE_TIME_MS 2000
#define MAIN_DEPLOYMENT_ALT 150 // meters AGL to deploy main chute at

#define DROGUE_PIN 13 // ! Set to actual pin when not testing
#define MAIN_PIN 13 // ! Set to actual pin when not testing

#define SD_CS_PIN 23
#define SD_WRITE_DELAY_MS 1000

// -----------------------
// State Machine Variables
// -----------------------

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
State current_state = State::PAD_IDLE;

int stateCounter = 0;
bool stateInit = false;
unsigned long stateStart_ms = 0;

// ---------------------
// Flight Data Variables
// ---------------------

double apogee = 0.0;

// --------------
// System Objects
// --------------
BmpManager bmp;
Altimeter altimeter;

SdFat SD;
FsFile flightLog;
bool sdActive = false;

// ---------------------
// Function Declarations
// ---------------------
void handlePadIdle(double agl, bool bmp_update);
void handleBoost(double agl, bool bmp_update);
void handleCoast(double agl, bool bmp_update);
void handleDrogueDeployment(double agl, bool bmp_update);
void handleDrogueDescent(double agl, bool bmp_update);
void handleMainDeployment(double agl, bool bmp_update);
void handleMainDescent(double agl, bool bmp_update);
void handleLanded(double agl, bool bmp_update);
void transitionState(State next_state);

void setupSD();

void setup() {
    pinMode(DROGUE_PIN, OUTPUT);
    pinMode(MAIN_PIN, OUTPUT);
    
    digitalWrite(DROGUE_PIN, LOW);
    digitalWrite(MAIN_PIN, LOW);

    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // ! [REMOVE FOR FLIGHT] Waits for serial

    if (!bmp.begin()) {
        Serial.println("--BMP FAILED TO START--");
    }

    bmp.setMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_80_HZ);
    bmp.setTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.setPressureOSR(BMP5XX_OVERSAMPLING_8X);
    bmp.setIIRCoeff(BMP5XX_IIR_FILTER_COEFF_3);

    altimeter.setGroundMode(true);

    setupSD();
}

void loop() {
    bool bmp_update = false;
    
    // gather raw altitude data
    BmpData raw_bmp_data;
    if (bmp.update()) {
        raw_bmp_data = bmp.getBmpData();
        altimeter.update(raw_bmp_data.altitude);
        bmp_update = true;

        if (sdActive) {
            flightLog.print(millis());
            flightLog.print(",");
            flightLog.print(static_cast<int>(current_state));
            flightLog.print(",");
            flightLog.print(altimeter.getAltitudeAGL(), 2);
            flightLog.print(",");
            flightLog.print(raw_bmp_data.pressure, 2);
            flightLog.print(",");
            flightLog.println(apogee, 2);
        }
    }
    double agl = altimeter.getAltitudeAGL();

    // enter current state
    switch (current_state) {
        case State::PAD_IDLE:           handlePadIdle(agl, bmp_update);             break;
        case State::BOOST:              handleBoost(agl, bmp_update);               break;
        case State::COAST:              handleCoast(agl, bmp_update);               break;
        case State::DROGUE_DEPLOYMENT:  handleDrogueDeployment(agl, bmp_update);    break;
        case State::DROGUE_DESCENT:     handleDrogueDescent(agl, bmp_update);       break;
        case State::MAIN_DEPLOYMENT:    handleMainDeployment(agl, bmp_update);      break;
        case State::MAIN_DESCENT:       handleMainDescent(agl, bmp_update);         break;
        case State::LANDED:             handleLanded(agl, bmp_update);              break;
        default:                        break;
    }

    static unsigned long lastSdWrite = 0;
    if (millis() - lastSdWrite >= SD_WRITE_DELAY_MS) {
        flightLog.sync();
        lastSdWrite = millis();
    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 1000) {
        last_ping = millis();
        Serial.printf("Alive Ping: %d\n", millis() / 1000);
    }
}

// ---------------------
// State Logic Functions
// ---------------------

void handlePadIdle(double agl, bool bmp_update) {
    if (bmp_update) {
        if (agl > 15) {
            stateCounter++;
            if (stateCounter >= 3) {
                transitionState(State::BOOST);
            }
        } else {
            stateCounter = 0;
        }
    }
}


void handleBoost(double agl, bool bmp_update) {
    if (millis() - stateStart_ms > BOOST_TIME_MS) {
        transitionState(State::COAST);
    }
}


void handleCoast(double agl, bool bmp_update) {
    if (bmp_update) {
        if (agl > apogee) {
            apogee = agl;
        }
        
        if (agl < (apogee - DROGUE_DROP_M)) {
            stateCounter++;
            if (stateCounter >= 3) {
                transitionState(State::DROGUE_DEPLOYMENT);
            }
        } else {
            stateCounter = 0;
        }
    }
}


void handleDrogueDeployment(double agl, bool bmp_update) {
    if (!stateInit) {
        digitalWrite(DROGUE_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(DROGUE_PIN, LOW);
        transitionState(State::DROGUE_DESCENT);
    }
}


void handleDrogueDescent(double agl, bool bmp_update) {
    if (bmp_update) {
        if (agl < MAIN_DEPLOYMENT_ALT) {
            stateCounter++;
            if (stateCounter >= 3) {
                transitionState(State::MAIN_DEPLOYMENT);
            }
        } else {
            stateCounter = 0;
        }
    }
}


void handleMainDeployment(double agl, bool bmp_update) {
    if (!stateInit) {
        digitalWrite(MAIN_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(MAIN_PIN, LOW);
        transitionState(State::MAIN_DESCENT);
    }
}


void handleMainDescent(double agl, bool bmp_update) {
    static double lastAgl = 0.0;
    static unsigned long lastCheck_ms = 0.0;

    if (!stateInit) {
        lastAgl = agl;
        lastCheck_ms = millis();
        stateInit = true;
    }
    
    if (millis() - lastCheck_ms >= 1000) {
        double verticalDiff = std::abs(agl - lastAgl);

        if (verticalDiff < 2.0) {
            stateCounter++;
            if (stateCounter >= 3) {
                transitionState(State::LANDED);
            }
        } else {
            stateCounter = 0;
        }

        lastAgl = agl;
        lastCheck_ms = millis();
    }
}


void handleLanded(double agl, bool bmp_update) {
    if (!stateInit) {
        if (sdActive) {
            flightLog.close();
            Serial.println("FLIGHT LOG SAVED");
        }
        stateInit = true;
    }
}


void transitionState(State next_state) {
    stateCounter = 0;
    stateStart_ms = millis();
    stateInit = false;

    current_state = next_state;
    Serial.printf("STATE-CHANGE: %d\n", static_cast<int>(next_state));
}


// -----------------
// SD Card Functions
// -----------------

void setupSD() {
    // Set SdFat to use SPI1 at 16 MHz
    SdSpiConfig sdConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI1);

    if (!SD.begin(sdConfig)) {
        Serial.println("--SD CARD FAILED TO MOUNT--");
        return;
    }

    // find next possible flight log number
    char filename[15];
    strcpy(filename, "FLIGHT00.CSV");
    for (uint8_t i = 0; i < 100; i++) {
        filename[6] = '0' + i / 10;
        filename[7] = '0' + i % 10;
        if (!SD.exists(filename)) {
            break;
        }
    }

    flightLog = SD.open(filename, O_WRITE | O_CREAT);
    if (flightLog) {
        flightLog.println("Time(ms),State,AGL_Alt(m),Raw_Pressure(Pa),Apogee(m)");
        flightLog.sync(); // force save to the card
        sdActive = true;
        Serial.printf("SD Card Active. Logging to: %s\n", filename);
    }
}