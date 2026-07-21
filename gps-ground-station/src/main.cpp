#include <Arduino.h>
#include <optional>
#include "RadioManager.h"

RadioManager radio;
bool radio_active = false;

void setup() {
    Serial.begin(115200);    // USB Serial for Monitor
    while (!Serial) delay(10);      // Wait for Serial Monitor to open [REMOVE FOR FLIGHT]

    if (!radio.Begin()) {
        Serial.println("--LORA FAILED TO START--");
    } else {
        radio_active = true;
    }

    Serial.println("Teensy 4.1 Ground Station");
}

void loop() {
    if (radio_active) {
        String msg = radio.Receive();
        if (msg.length() > 0) {
            Serial.printf("RX: %s\n", msg.c_str());
        }
    }
}