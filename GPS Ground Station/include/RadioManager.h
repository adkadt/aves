#ifndef RADIO_MANAGER_H
#define RADIO_MANAGER_H

#include <Arduino.h>
#include <SPI.h>
#include <RH_RF95.h>

// Define pins for Teensy 4.1
#define RFM95_CS 10
#define RFM95_RST 9
#define RFM95_INT 2 // Change this to match your wiring (G0/DIO0 pin)

// Change to 868.0 if in Europe
#define RF95_FREQ 915.0

// Radio Addressing
#define FC_ADDR 1
#define GS_ADDR 2

class RadioManager {
    private:
        RH_RF95 rf95_;

    public:
        RadioManager();
        bool Begin();
        void Transmit(String data);
        String Receive();
};

#endif
