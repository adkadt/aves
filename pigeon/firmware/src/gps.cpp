#include "gps.hpp"
#include "config.hpp"
#include <HardwareSerial.h>
#include <TinyGPS++.h>

namespace {
    HardwareSerial gpsSerial(1);
    TinyGPSPlus gps;
    
    Gps::Data gpsData{};

    uint32_t lastSentenceReceived = 0;
    uint32_t lastLocationUpdate = 0;
    bool hasReceivedData = false;
}

void Gps::begin() {
    gpsSerial.begin(
        Config::GPS_BAUD_RATE, 
        SERIAL_8N1, 
        Pins::GPS_RX, 
        Pins::GPS_TX
    );
}

void Gps::update() {
    while (gpsSerial.available() > 0) {
        // Serial.print((char)gpsSerial.read());
        if (gps.encode(gpsSerial.read())) {
            // Position
            if (gps.location.isValid()) {
                gpsData.position.latitude = gps.location.lat();
                gpsData.position.longitude = gps.location.lng();
                gpsData.position.altitude = gps.altitude.meters();
                lastLocationUpdate = millis();
            }

            if (gps.speed.isValid())
                gpsData.speed = gps.speed.kmph();
            if (gps.course.isValid())
                gpsData.course = gps.course.deg();

            if (gps.satellites.isValid())
                gpsData.satellites = gps.satellites.value();
            if (gps.hdop.isValid())
                gpsData.hdop = gps.hdop.hdop();

            // UTC Time
            if (gps.time.isValid()) {
                gpsData.utcTime.hour = gps.time.hour();
                gpsData.utcTime.minute = gps.time.minute();
                gpsData.utcTime.second = gps.time.second();

                gpsData.utcTime.day = gps.date.day();
                gpsData.utcTime.month = gps.date.month();
                gpsData.utcTime.year = gps.date.year();
            }
        
            // bookkeeping
            lastSentenceReceived = millis();
            hasReceivedData = true;
        }
    }
}

bool Gps::hasFix() {
    return gps.location.isValid();
}

bool Gps::isConnected() {
    return hasReceivedData && (millis() - lastSentenceReceived < Config::GPS_TIMEOUT_MS);
}

bool Gps::hasLocation() {
    return gps.location.isValid() && gps.location.age() < Config::GPS_TIMEOUT_MS;
}


const Gps::Data& Gps::getData() {
    return gpsData;
}