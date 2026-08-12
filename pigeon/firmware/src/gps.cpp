#include "gps.hpp"
#include "config.hpp"
#include <HardwareSerial.h>
#include <TinyGPS++.h>

#include "system.hpp"

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

    void applyEma(Gps::Position& filtered, const Gps::Position& measurement, double alpha);
    double calculatePositionAlpha(float speed);
}

void Gps::begin() {
    gpsStatus = Gps::Status::DISCONNECTED;

    gpsSerial.begin(
        Config::GPS_BAUD_RATE, 
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
        return gps.charsProcessed() > 0 && (millis() - lastUpdate < Config::GPS_TIMEOUT_MS);
    }

    bool hasFix() {
        return hasReceivedData && (millis() - lastUpdate < Config::GPS_TIMEOUT_MS);
    }
    
    bool hasLocation() {
        return gps.location.isValid() && gps.location.age() < Config::GPS_TIMEOUT_MS;
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
                gpsData.rawPosition, 
                calculatePositionAlpha(gpsData.speed)
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
        const Gps::Position& measurement, 
        double alpha
    ) {
        const double beta = 1.0 - alpha;
        filtered.latitude = beta * filtered.latitude + alpha * measurement.latitude;
        filtered.longitude = beta * filtered.longitude + alpha * measurement.longitude;
        filtered.altitude = beta * filtered.altitude + alpha * measurement.altitude;

    }

    double calculatePositionAlpha(float speed) {
        float t = speed / Config::GPS_MAX_SPEED;
        t = std::min(std::max(t, 0.0f), 1.0f);
        return Config::GPS_MIN_ALPHA + t * (Config::GPS_MAX_ALPHA - Config::GPS_MIN_ALPHA);
    }
}