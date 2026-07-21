#include <Arduino.h>
#include <optional>
#include "BmpManager.h"
#include "Altimeter.h"
#include "DataLogger.h"
#include "FlightController.h"
#include <Adafruit_NeoPixel.h>

#define SIMULATION_MODE_ENABLED false

constexpr uint8_t DROGUE_PIN = 13; // ! Set to actual pin when not testing
constexpr uint8_t MAIN_PIN = 13; // ! Set to actual pin when not testing

constexpr uint8_t SD_CS_PIN = 23;
constexpr unsigned long SD_WRITE_INTERVAL_MS = 500;

constexpr uint8_t NEOPIXEL_PIN = 17;
constexpr uint8_t NUMPIXELS = 1;

constexpr uint8_t BUZZER_PIN = 5;


#if SIMULATION_MODE_ENABLED
    #warning "SIMULATION_MODE_ENABLED is enabled - disable before flight!"
    #include <cstdlib>
    double injectNoise(double altitude, double stddev_m = 0.4) {
        // Box-Muller transform for Gaussian noise
        double u1 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double u2 = (rand() + 1.0) / (RAND_MAX + 1.0);
        double gaussian = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        return altitude + gaussian * stddev_m;
    }
#endif
// --------------
// System Objects
// --------------
BmpManager bmp;
Altimeter altimeter;
DataLogger logger(SD_CS_PIN, SD_WRITE_INTERVAL_MS);
FlightController flightController(altimeter, logger, DROGUE_PIN, MAIN_PIN);

Adafruit_NeoPixel pixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ---------------------
// Function Declarations
// ---------------------
void updateStateLED(State state);
void fatalError(int num);
void printTelemetry(double agl, State state, double apogee);

void setup() {
    pinMode(DROGUE_PIN, OUTPUT);
    pinMode(MAIN_PIN, OUTPUT);
    
    digitalWrite(DROGUE_PIN, LOW);
    digitalWrite(MAIN_PIN, LOW);

    pinMode(BUZZER_PIN, OUTPUT);

    pixel.begin();
    pixel.setBrightness(50);
    pixel.clear();
    pixel.show();

    Serial.begin(115200);    // USB Serial for Monitor
    
    if (SIMULATION_MODE_ENABLED) {
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

    if (!logger.begin(SIMULATION_MODE_ENABLED)) {
        Serial.println("--SD CARD FAILED TO MOUNT--");
        fatalError(2);
    }

    updateStateLED(flightController.getState());

    // Success chime
    tone(BUZZER_PIN, 523);
    delay(150);
    tone(BUZZER_PIN, 659);
    delay(150);
    tone(BUZZER_PIN, 784);
    delay(150);
    tone(BUZZER_PIN, 1046);
    delay(400);
    noTone(BUZZER_PIN);
}

void loop() {
    bool altitudeUpdate = false;
    double rawSensorValue = 0.0;

    if (SIMULATION_MODE_ENABLED) {
        if (Serial.available()) {
            rawSensorValue = Serial.parseFloat();
            Serial.readStringUntil('\n'); // Clear the rest of the buffer (including newline)
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

    double agl = altimeter.getAltitudeAGL();
    State current_state = flightController.getState();

    if (altitudeUpdate && logger.isActive()) {
        logger.logData(millis(), static_cast<int>(current_state), agl, rawSensorValue, flightController.getApogee());
    }

    State new_state = flightController.update(agl, altitudeUpdate);
    if (new_state != current_state) {
        updateStateLED(new_state);
        tone(BUZZER_PIN, 2000, 200);
    }

    logger.sync();
    printTelemetry(agl, new_state, flightController.getApogee());

    // chirp while idle on the pad
    if (new_state == State::PAD_IDLE || new_state == State::LANDED) {
        static unsigned long last_heartbeat = 0;
        static int beeps_remaining = 0;
        static unsigned long last_beep_time = 0;

        // Trigger the start of a beep sequence every 2.5 seconds
        if (millis() - last_heartbeat >= 2500) {
            beeps_remaining = (new_state == State::LANDED) ? 3 : 1;
            last_heartbeat = millis();
        }

        // Process the active beep sequence
        if (beeps_remaining > 0 && millis() - last_beep_time >= 150) {
            tone(BUZZER_PIN, 3000, 50);
            last_beep_time = millis();
            beeps_remaining--;
        }
    }
}


// -----------------
// SD Card Functions
// -----------------

void updateStateLED(State state) {
    uint32_t color = 0;

    switch (state) {
        case State::PAD_IDLE:          color = pixel.Color(0, 255, 0);     break; // Green - Ready
        case State::BOOST:             color = pixel.Color(255, 255, 255); break; // White - Motor firing
        case State::COAST:             color = pixel.Color(0, 0, 255);     break; // Blue - Sky
        case State::DROGUE_DEPLOYMENT: color = pixel.Color(255, 255, 0);   break; // Yellow - First Event
        case State::DROGUE_DESCENT:    color = pixel.Color(255, 128, 0);   break; // Orange - Falling
        case State::MAIN_DEPLOYMENT:   color = pixel.Color(255, 0, 255);   break; // Magenta - Second Event
        case State::MAIN_DESCENT:      color = pixel.Color(0, 255, 255);   break; // Cyan - Falling
        case State::LANDED:            color = pixel.Color(255, 0, 0);     break; // Red - Done
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
            tone(BUZZER_PIN, 1000);
            delay(300);
            
            pixel.clear();
            pixel.show();
            noTone(BUZZER_PIN);
            delay(300);
        }
        
        delay(1500); 
    }
}

void printTelemetry(double agl, State state, double apogee) {
    if (SIMULATION_MODE_ENABLED) return;
    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 250) { // Print 4 times a second
        last_ping = millis();
        Serial.printf("Time: %d | State: %d | AGL: %.2f | Apogee: %.2f\n", 
                      millis(), static_cast<int>(state), agl, apogee);
    }
}