#include <Arduino.h>
#include "config.hpp"
#include <SPI.h>

#include "led.hpp"
#include "gps.hpp"
#include "lora.hpp"

void setup() {
    Serial.begin(Config::BAUD_RATE);
    Led::begin();
    Gps::begin();
    Led::setState(Led::State::GPS_SEARCHING);

    SPI.begin(Pins::LORA_SCK, Pins::LORA_MISO, Pins::LORA_MOSI);
    LoRa::begin();
}

void loop() {
    Gps::update();
    Led::update();

    static uint32_t lastPrint = 0;

    if (millis() - lastPrint >= 1000)
    {
        lastPrint = millis();

        Serial.println();

        Serial.print("Connected: ");
        Serial.println(Gps::isConnected() ? "Yes" : "No");

        Serial.print("Fix: ");
        Serial.println(Gps::hasFix() ? "Yes" : "No");

        Serial.print("Location: ");
        Serial.println(Gps::hasLocation() ? "Yes" : "No");

        if (!Gps::hasLocation())
            Led::setState(Led::State::GPS_SEARCHING);
        else
            Led::setState(Led::State::GPS_LOCK);

        if (Gps::isConnected())
        {
            const Gps::Data& data = Gps::getData();

            Serial.print("Latitude: ");
            Serial.println(data.position.latitude, 6);

            Serial.print("Longitude: ");
            Serial.println(data.position.longitude, 6);

            Serial.print("Altitude: ");
            Serial.print(data.position.altitude);
            Serial.println(" m");

            Serial.print("Speed: ");
            Serial.print(data.speed);
            Serial.println(" km/h");

            Serial.print("Course: ");
            Serial.print(data.course);
            Serial.println(" deg");

            Serial.print("Satellites: ");
            Serial.println(data.satellites);

            Serial.print("HDOP: ");
            Serial.println(data.hdop);

            Serial.printf(
                "UTC: %02u:%02u:%02u %02u/%02u/%04u\n",
                data.utcTime.hour,
                data.utcTime.minute,
                data.utcTime.second,
                data.utcTime.day,
                data.utcTime.month,
                data.utcTime.year
            );
        }
    }
    
}