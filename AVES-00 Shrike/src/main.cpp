#include <Arduino.h>
#include <optional>
#include "BmpManager.h"
#include "Altimeter.h"
#include <SPI.h>
#include "SdFat.h"
#include <Adafruit_NeoPixel.h>
#include <cmath>

#define SIMULATION_MODE false

#define BOOST_TIME_MS 2000 // seconds to ignore pressure for apogee readings
#define DROGUE_DROP_M 3 // the meters vertically down from apogee to deploy drogue chute
#define FIRE_TIME_MS 2000
#define MAIN_DEPLOYMENT_ALT 150 // meters AGL to deploy main chute at

#define DROGUE_PIN 13 // ! Set to actual pin when not testing
#define MAIN_PIN 13 // ! Set to actual pin when not testing

#define SD_CS_PIN 23
#define SD_WRITE_INTERVAL_MS 500

#define NEOPIXEL_PIN 17
#define NUMPIXELS 1

#ifdef SIMULATION_MODE
    #warning "SIMULATION_MODE is enabled - disable before flight!"
    #include <cstdlib>
    double injectNoise(double altitude, double stddev_m = 0.4) {
        // Box-Muller transform for Gaussian noise
        double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double gaussian = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        return altitude + gaussian * stddev_m;
    }
#endif

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

SdFs SD;
FsFile flightLog;
FsFile simFile;
bool sdActive = false;

Adafruit_NeoPixel pixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ---------------------
// Function Declarations
// ---------------------
void handlePadIdle(double agl, bool altitudeUpdate);
void handleBoost(double agl, bool altitudeUpdate);
void handleCoast(double agl, bool altitudeUpdate);
void handleDrogueDeployment(double agl, bool altitudeUpdate);
void handleDrogueDescent(double agl, bool altitudeUpdate);
void handleMainDeployment(double agl, bool altitudeUpdate);
void handleMainDescent(double agl, bool altitudeUpdate);
void handleLanded(double agl, bool altitudeUpdate);
void transitionState(State next_state);
void setupSD();
void updateStateLED();
void fatalError(int num);

void setup() {
    pinMode(DROGUE_PIN, OUTPUT);
    pinMode(MAIN_PIN, OUTPUT);
    
    digitalWrite(DROGUE_PIN, LOW);
    digitalWrite(MAIN_PIN, LOW);

    pixel.begin();
    pixel.setBrightness(50);
    uint32_t color = pixel.Color(0, 255, 0);
    pixel.setPixelColor(0, color);
    pixel.show();

    Serial.begin(115200);    // USB Serial for Monitor
    
    if (SIMULATION_MODE) {
        while (!Serial) delay(10);      // ! [REMOVE FOR FLIGHT] Waits for serial
    } else {
        if (!bmp.begin()) {
            Serial.println("--BMP FAILED TO START--");
            fatalError(1);
        }
    
        bmp.setMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_80_HZ);
        bmp.setTempOSR(BMP5XX_OVERSAMPLING_1X);
        bmp.setPressureOSR(BMP5XX_OVERSAMPLING_8X);
        bmp.setIIRCoeff(BMP5XX_IIR_FILTER_COEFF_3);
    }


    altimeter.setGroundMode(true);

    setupSD();
    updateStateLED();
}

void loop() {
    bool altitudeUpdate = false;
    double rawSensorValue = 0.0;

    if (SIMULATION_MODE) {
        if (Serial.available()) {
            String input = Serial.readStringUntil('\n'); 
            rawSensorValue = input.toDouble();
            
            altimeter.update(rawSensorValue);
            altitudeUpdate = true;
        }

    } else {
        // gather raw altitude data
        if (bmp.update()) {
            BmpData raw_bmp_data;
            raw_bmp_data = bmp.getBmpData();
            rawSensorValue = raw_bmp_data.pressure;
            altimeter.update(raw_bmp_data.altitude);
            altitudeUpdate = true;
        }
    }

    if (altitudeUpdate && sdActive) {
        flightLog.print(millis());
        flightLog.print(",");
        flightLog.print(static_cast<int>(current_state));
        flightLog.print(",");
        flightLog.print(altimeter.getAltitudeAGL(), 2);
        flightLog.print(",");
        flightLog.print(rawSensorValue, 2);
        flightLog.print(",");
        flightLog.println(apogee, 2);
    }
    double agl = altimeter.getAltitudeAGL();

    // enter current state
    switch (current_state) {
        case State::PAD_IDLE:           handlePadIdle(agl, altitudeUpdate);             break;
        case State::BOOST:              handleBoost(agl, altitudeUpdate);               break;
        case State::COAST:              handleCoast(agl, altitudeUpdate);               break;
        case State::DROGUE_DEPLOYMENT:  handleDrogueDeployment(agl, altitudeUpdate);    break;
        case State::DROGUE_DESCENT:     handleDrogueDescent(agl, altitudeUpdate);       break;
        case State::MAIN_DEPLOYMENT:    handleMainDeployment(agl, altitudeUpdate);      break;
        case State::MAIN_DESCENT:       handleMainDescent(agl, altitudeUpdate);         break;
        case State::LANDED:             handleLanded(agl, altitudeUpdate);              break;
        default:                        break;
    }

    static unsigned long lastSdWrite = 0;
    if (millis() - lastSdWrite >= SD_WRITE_INTERVAL_MS) {
        if (sdActive) {
            flightLog.sync();
        }
        lastSdWrite = millis();
    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 250) { // Print 4 times a second
        last_ping = millis();
        Serial.printf("Time: %d | State: %d | AGL: %.2f | Apogee: %.2f\n", 
                      millis(), static_cast<int>(current_state), agl, apogee);
    }
}

// ---------------------
// State Logic Functions
// ---------------------

void handlePadIdle(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
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


void handleBoost(double agl, bool altitudeUpdate) {
    if (millis() - stateStart_ms > BOOST_TIME_MS) {
        transitionState(State::COAST);
    }
}


void handleCoast(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
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


void handleDrogueDeployment(double agl, bool altitudeUpdate) {
    if (!stateInit) {
        digitalWrite(DROGUE_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(DROGUE_PIN, LOW);
        transitionState(State::DROGUE_DESCENT);
    }
}


void handleDrogueDescent(double agl, bool altitudeUpdate) {
    if (altitudeUpdate) {
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


void handleMainDeployment(double agl, bool altitudeUpdate) {
    if (!stateInit) {
        digitalWrite(MAIN_PIN, HIGH);
        stateStart_ms = millis();
        stateInit = true;

    } else if (millis() - stateStart_ms >= FIRE_TIME_MS) {
        digitalWrite(MAIN_PIN, LOW);
        transitionState(State::MAIN_DESCENT);
    }
}


void handleMainDescent(double agl, bool altitudeUpdate) {
    static double lastAgl = 0.0;
    static unsigned long lastCheck_ms = 0;

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


void handleLanded(double agl, bool altitudeUpdate) {
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

    updateStateLED();
}


// -----------------
// SD Card Functions
// -----------------

void setupSD() {
    SPI.setRX(20);  // MISO
    SPI.setTX(19);  // MOSI
    SPI.setSCK(18); // SCK
    SPI.begin();    // Boot the SPI bus

    // Set SdFat to use SPI1 at 16 MHz
    SdSpiConfig sdConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI);

    if (!SD.begin(sdConfig)) {
        Serial.println("--SD CARD FAILED TO MOUNT--");
        fatalError(2);
        return;
    }

    if (SIMULATION_MODE) {
        flightLog = SD.open("SIM_OUT.csv", O_WRITE | O_CREAT | O_TRUNC);
        if (flightLog) {
            flightLog.println("Time(ms),State,AGL_Alt(m),Raw_Pressure(Pa),Apogee(m)");
            flightLog.sync(); // force save to the card
            sdActive = true;
            Serial.println("SIMULATION MODE ACTIVE: File Loaded.");
        } else {
            Serial.println("ERROR: COULD NOT CREATE SIM_OUT.CSV");
        }
    } else {
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

}


void updateStateLED() {
    uint32_t color = 0;

    switch (current_state) {
        case State::PAD_IDLE:          color = pixel.Color(0, 0, 255);     break; // Blue
        case State::BOOST:             color = pixel.Color(0, 255, 0);     break; // Green
        case State::COAST:             color = pixel.Color(255, 255, 0);   break; // Yellow
        case State::DROGUE_DEPLOYMENT: color = pixel.Color(255, 0, 255);   break; // Purple
        case State::DROGUE_DESCENT:    color = pixel.Color(0, 255, 255);   break; // Cyan
        case State::MAIN_DEPLOYMENT:   color = pixel.Color(255, 255, 255); break; // White
        case State::MAIN_DESCENT:      color = pixel.Color(255, 128, 0);   break; // Orange
        case State::LANDED:            color = pixel.Color(255, 0, 0);     break; // Red
    }

    pixel.setPixelColor(0, color);
    pixel.show();
}


void fatalError(int num) {
    digitalWrite(DROGUE_PIN, LOW);
    digitalWrite(MAIN_PIN, LOW);

    while (true) {
        for (int i = 0; i < num; i++) {
            pixel.setPixelColor(0, pixel.Color(255, 0, 0));
            pixel.show();
            delay(300);
            
            pixel.clear();
            pixel.show();
            delay(300);
        }
        
        delay(1500); 
    }
}