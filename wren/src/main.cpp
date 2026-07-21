#include <Arduino.h>
#include <optional>
#include "GpsManager.h"
#include "BmpManager.h"
#include "ImuManager.h"
#include "RadioManager.h"

// GPS using Serial5 pins (TX5 20, RX5 21)
#define GPS_SERIAL Serial5

enum class State: uint8_t {
    PAD_IDLE,
    BOOST,
    COAST,
    APOGEE,
    DROGUE_DEPLOYED,
    MAIN_DEPLOYED,
    LANDED
};

GpsManager gps(GPS_SERIAL, 115200);
BmpManager bmp;
ImuManager imu;
RadioManager radio;
bool radio_active = false;
State current_state = State::PAD_IDLE;

double ground_altitude_m;

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    // while (!Serial) delay(10);      // Wait for Serial Monitor to open [REMOVE FOR FLIGHT]

    // set up i2c
    Wire.begin();
    Wire.setClock(400000); // Lowered to 400kHz for BNO085 stability

    gps.Begin();    // starts gps

    if (!bmp.Begin()) {
        Serial.println("--BMP FAILED TO START--");
    }

    if (!imu.Begin()) {
        Serial.println("--IMU FAILED TO START--");
    }

    if (!radio.Begin()) {
        Serial.println("--LORA FAILED TO START--");
    } else {
        radio_active = true;
    }

    bmp.SetMode(BMP5XX_POWERMODE_CONTINUOUS, BMP5XX_ODR_240_HZ);
    bmp.SetTempOSR(BMP5XX_OVERSAMPLING_1X);
    bmp.SetPressureOSR(BMP5XX_OVERSAMPLING_1X);

    while (!bmp.Update()) {
        delay(10);
    }
    BmpData bmp_data = bmp.GetBmpData();
    ground_altitude_m = bmp_data.altitude;

    Serial.println("Teensy 4.1 GPS Initialized");
    if (radio_active) {
        delay(500); // Give power time to stabilize before high-current TX
        radio.Transmit("Wren Flight Computer Initialized");
    }
}

int last_gps_time = 0;
int last_bmp_time = -1;
double sps;

int counter = 0;
unsigned long last_print_time = 0;

void loop() {
    bool update = false;
    bool printed = false;
    char radio_buf[64]; // Buffer for radio packets
    
    // send gps data to ground station
    if (gps.Update()) {
        locationData gps_data = gps.GetLocationData();
        
        if (gps_data.is_valid) {
            snprintf(radio_buf, sizeof(radio_buf), "GPS:%.4f,%.4f,%.0f", gps_data.lat, gps_data.lng, gps_data.alt);
            if (radio_active) {
                radio.Transmit(radio_buf);
                Serial.printf("TX: %s\n", radio_buf);
            }
        }
    } else {
        if (millis() - last_print_time > 1000) {
            snprintf(radio_buf, sizeof(radio_buf), "Waiting for FIX");
            if (radio_active){
                radio.Transmit(radio_buf);
                Serial.println("TX: Waiting for FIX");
            }    
            last_print_time = millis();
        }
    }

    // gather raw altitude data
    BmpData raw_bmp_data;
    if (bmp.Update()) {
        raw_bmp_data = bmp.GetBmpData();

        snprintf(radio_buf, sizeof(radio_buf), "ALT:%.2f", raw_bmp_data.altitude - ground_altitude_m);
        if (radio_active) {
            radio.Transmit(radio_buf);
            Serial.printf("TX: %s\n", radio_buf);
        }
    }

    // enter current state
    switch (current_state) {
        case State::PAD_IDLE:
            break;
        case State::BOOST:
            break;
        case State::COAST:
            break;
        case State::APOGEE:
            break;
        case State::DROGUE_DEPLOYED:
            break;
        case State::MAIN_DEPLOYED:
            break;
        case State::LANDED:
            break;
        default:
            break;
    }
    
    // if (imu.Update()) {
    //     update = true;

    //     if (counter % 20 == 0) {
    //         Serial.printf("Tilt: %.1f | R: %.1f, P: %.1f, Y: %.1f\n", 
    //                       imu.GetTilt(), imu.GetRoll(), imu.GetPitch(), imu.GetYaw());
    //         printed = true;
    //     }
    // }
    
    if (update) {
        counter++;
    }
    
    if (printed) {
        Serial.println("==================");
    }

    static unsigned long last_ping = 0;
    if (millis() - last_ping >= 1000) {
        last_ping = millis();
        Serial.printf("Alive Ping: %d\n", millis() / 1000);
    }

}