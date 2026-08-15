#include "modes/flight.hpp"

#include "system/system.hpp"
#include "config.hpp"

#include "protocol/payloads.h"
#include "hardware/lora.hpp"
#include "hardware/gps.hpp"
#include "hardware/battery.hpp"

#include <Arduino.h>

namespace {
    bool sendHeartbeat();
    void sendGpsTelemetry();

    uint32_t lastHeartbeatTime= millis();
    uint32_t lastGpsTelemetryTime = millis();

    double maxAltitude = 0;
    double minAltitude = INFINITY;
}

void Mode::Flight::begin() {
    sendHeartbeat();

    uint32_t now = millis();

    lastHeartbeatTime = now;
    lastGpsTelemetryTime = now;

    Serial.println("Flight mode started");
}

void Mode::Flight::update() {
    uint32_t now = millis();
    
    if (now - lastHeartbeatTime >= Config::Flight::HEARTBEAT_INTERVAL) {
        lastHeartbeatTime = now;
        sendHeartbeat();
    } else if (Gps::getStatus() == Gps::Status::LOCKED) {
        if (now - lastGpsTelemetryTime >= Config::Flight::GPS_TELEMETRY_INTERVAL) {
            lastGpsTelemetryTime = now;
            sendGpsTelemetry();
        }
        
        if (Gps::getAltitude() > maxAltitude)
            maxAltitude = Gps::getAltitude();
        if (Gps::getAltitude() < minAltitude)
            minAltitude = Gps::getAltitude();
    }

    if (maxAltitude - minAltitude > 50 && maxAltitude - minAltitude < 99999) {
        System::setState(System::State::RECOVERY);
    }
}

namespace {
    bool sendHeartbeat() {
        Payload::Heartbeat payload{};

        payload.uptime = millis();
        payload.mode = static_cast<uint8_t>(System::getMode());
        payload.state = static_cast<uint8_t>(System::getState());

        payload.batteryStatus = static_cast<uint8_t>(Battery::getStatus());
        payload.batteryVoltage = Battery::getVoltage() * 100;

        payload.gpsStatus = static_cast<uint8_t>(Gps::getStatus());

        payload.errors = System::getErrors();
        payload.warnings = System::getWarnings();

        LoRa::Packet packet = LoRa::createPacket(
            System::getPairedDeviceId(), 
            System::getDeviceId(), 
            LoRa::PacketType::HEARTBEAT, 
            reinterpret_cast<const uint8_t*>(&payload), 
            sizeof(payload)
        );

        LoRa::transmit(packet);
    }

    void sendGpsTelemetry() {
        Payload::GpsTelemetry payload{};

        payload.latitude = Gps::getLatitude() * 10000000;
        payload.longitude = Gps::getLongitude() * 10000000;
        payload.altitude = Gps::getAltitude() * 100000;

        payload.maxAltitude = maxAltitude * 100000;
        payload.minAltitude = minAltitude * 100000;

        payload.speed = Gps::getSpeed() * 100000;
        payload.course = Gps::getCourse() * 100000;

        payload.satellites = Gps::getSatellites();
        payload.status = static_cast<uint8_t>(Gps::getStatus());

        LoRa::Packet packet = LoRa::createPacket(
            System::getPairedDeviceId(), 
            System::getDeviceId(), 
            LoRa::PacketType::GPS_TELEMETRY,
            reinterpret_cast<const uint8_t*>(&payload), 
            sizeof(payload)
        );

        LoRa::transmit(packet);
    }
}