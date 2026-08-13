#include "hardware/gps.hpp"
#include "config.hpp"
#include <HardwareSerial.h>
#include <TinyGPS++.h>

#include "system/system.hpp"

namespace {
    // GPS Status
    Gps::Status gpsStatus;
    Gps::Status previousStatus;

    // driver state
    HardwareSerial gpsSerial(1);
    TinyGPSPlus gps;
    Gps::Data gpsData{};
    
    uint32_t lastUpdate = 0;
    bool hasReceivedData = false;
    bool filterInitialized = false;

    // helpers
    void updateStatus();
    void updatePosition();
    void updateMotion();
    void updateTime();
    void updateMetrics();

    bool hasFix();
    bool isConnected();
    bool hasLocation();

    void applyEma(Gps::Position& filtered, const Gps::Position& measurement);
}

void Gps::begin() {
    gpsStatus = Gps::Status::DISCONNECTED;

    gpsSerial.begin(
        Config::GPS::BAUD_RATE, 
        SERIAL_8N1, 
        Pins::GPS_TX,
            Pins::GPS_RX
    );
}

void Gps::update() {
    while (gpsSerial.available() > 0) {
        if (!gps.encode(gpsSerial.read()))
            continue;

        updateMotion();
        updatePosition();
        updateMetrics();
        updateTime();
        
        // bookkeeping
        lastUpdate = millis();
        hasReceivedData = true;
    }

    updateStatus();
}

Gps::Status Gps::getStatus() {
    return gpsStatus;
}

const Gps::Data& Gps::getData() {
    return gpsData;
}

double Gps::getLatitude() {
    return gpsData.position.latitude;
}

double Gps::getLongitude() {
    return gpsData.position.longitude;
}

float Gps::getAltitude() {
    return gpsData.position.altitude;
}

float Gps::getSpeed() {
    return gpsData.speed;
}

float Gps::getCourse() {
    return gpsData.course;
}

uint8_t Gps::getSatellites() {
    return gpsData.satellites;
}

float Gps::getHdop() {
    return gpsData.hdop;
}

DateTime Gps::getUtcTime() {
    return gpsData.utcTime;
}

namespace {
    void updateStatus() {
        Gps::Status previousStatus = gpsStatus;

        if (isConnected() && hasLocation()) {
            gpsStatus = Gps::Status::LOCKED;
        } else if (isConnected()) {
            gpsStatus = Gps::Status::SEARCHING;
        } else {
            gpsStatus = Gps::Status::DISCONNECTED;
        }

        if (previousStatus != gpsStatus) {
            if (gpsStatus == Gps::Status::LOCKED) {
                System::setEvent(System::Event::GPS_LOCKED);
                Serial.println("GPS locked");
            } else if (previousStatus == Gps::Status::LOCKED) {
                System::setEvent(System::Event::GPS_LOST);
            }

            if (gpsStatus == Gps::Status::DISCONNECTED) {
                System::setError(System::Error::GPS_DISCONNECTED);
                System::setEvent(System::Event::GPS_TIMEOUT);
            } else if (previousStatus == Gps::Status::DISCONNECTED) {
                System::clearError(System::Error::GPS_DISCONNECTED);
            }
        }
    }

    bool isConnected() {
        return gps.charsProcessed() > 0 && (millis() - lastUpdate < Config::GPS::TIMEOUT_MS);
    }

    bool hasFix() {
        return hasReceivedData && (millis() - lastUpdate < Config::GPS::TIMEOUT_MS);
    }
    
    bool hasLocation() {
        return gps.location.isValid() && gps.location.age() < Config::GPS::TIMEOUT_MS;
    }

    void updatePosition() {
        if (!gps.location.isUpdated())
            return;

        gpsData.rawPosition.latitude = gps.location.lat();
        gpsData.rawPosition.longitude = gps.location.lng();
        gpsData.rawPosition.altitude = gps.altitude.meters();

        if (!filterInitialized) {
            gpsData.position = gpsData.rawPosition;
            filterInitialized = true;
        } else {
            applyEma(
                gpsData.position, 
                gpsData.rawPosition
            );
        }
    }

    void updateMotion() {
        if (gps.speed.isValid())
            gpsData.speed = gps.speed.kmph();

        if (gps.course.isValid())
            gpsData.course = gps.course.deg();
    }

    void updateMetrics() {
        if (gps.satellites.isValid())
            gpsData.satellites = gps.satellites.value();

        if (gps.hdop.isValid())
            gpsData.hdop = gps.hdop.hdop();
    }

    void updateTime() {
        // UTC Time
        if (gps.time.isUpdated()) {
            gpsData.utcTime.hour = gps.time.hour();
            gpsData.utcTime.minute = gps.time.minute();
            gpsData.utcTime.second = gps.time.second();
        }

        if (gps.date.isUpdated()) {
            gpsData.utcTime.day = gps.date.day();
            gpsData.utcTime.month = gps.date.month();
            gpsData.utcTime.year = gps.date.year();
        }
    }



    void applyEma(
        Gps::Position& filtered, 
        const Gps::Position& measurement
    ) {
        const double alpha = Config::GPS::POSITION_ALPHA;
        const double beta = 1.0 - alpha;
        filtered.latitude = beta * filtered.latitude + alpha * measurement.latitude;
        filtered.longitude = beta * filtered.longitude + alpha * measurement.longitude;
        filtered.altitude = beta * filtered.altitude + alpha * measurement.altitude;

    }
}