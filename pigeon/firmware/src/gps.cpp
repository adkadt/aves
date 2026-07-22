#include "gps.hpp"
#include "config.hpp"
#include <HardwareSerial.h>
#include <TinyGPS++.h>

namespace {
    // driver state
    HardwareSerial gpsSerial(1);
    TinyGPSPlus gps;
    Gps::Data gpsData{};
    
    uint32_t lastUpdate = 0;
    bool hasReceivedData = false;
    bool filterInitialized = false;

    // helpers
    void updatePosition();
    void updateMotion();
    void updateStatus();
    void updateTime();

    void applyEma(Gps::Position& filtered, const Gps::Position& measurement, double alpha);
    double calculatePositionAlpha(float speed);
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
        if (!gps.encode(gpsSerial.read()))
            continue;

        updatePosition();
        updateMotion();
        updateStatus();
        updateTime();
        
        // bookkeeping
        lastUpdate = millis();
        hasReceivedData = true;
        
    }
}

bool Gps::hasFix() {
    return gps.location.isValid();
}

bool Gps::isConnected() {
    return hasReceivedData && (millis() - lastUpdate < Config::GPS_TIMEOUT_MS);
}

bool Gps::hasLocation() {
    return gps.location.isValid() && gps.location.age() < Config::GPS_TIMEOUT_MS;
}


const Gps::Data& Gps::getData() {
    return gpsData;
}

namespace {
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

    void updateStatus() {
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