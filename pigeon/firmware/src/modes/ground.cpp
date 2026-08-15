#include "modes/ground.hpp"

#include "hardware/lora.hpp"
#include "protocol/payloads.h"

#include <Arduino.h>

namespace {

    struct RemoteState {
        uint8_t deviceId;

        bool connected = false;

        uint32_t lastPacketTime;
        uint32_t lastHeartbeatTime;
        uint32_t lastGpsTelemetryTime;

        // System
        uint8_t mode;
        uint8_t state;
        uint8_t batteryStatus;
        float batteryVoltage;
        uint8_t gpsStatus;

        uint32_t errors;
        uint32_t warnings;

        // GPS
        double latitude;
        double longitude;
        double altitude;
        double maxAltitude;
        double minAltitude;

        double speed;
        double course;

        uint8_t satellites;

        // Radio
        float rssi;
        float snr;
    };

    RemoteState remoteState{};

    void receivePackets();
    void processPacket(const LoRa::Packet& packet);
    void processHeartbeat(const LoRa::Packet& packet);
    void processGpsTelemetry(const LoRa::Packet& packet);
    void checkConnection();
    
    void printRemoteState();
    void printGpsState();
}


void Mode::Ground::begin() {
    remoteState = {};
    System::setState(System::State::REMOTE_WAITING);
    Serial.println("Ground mode started");
}


void Mode::Ground::update() {
    receivePackets();
    checkConnection();
}


// private implementation
namespace {
    void receivePackets() {
        LoRa::Packet packet{};
        while (LoRa::receive(packet))
            processPacket(packet);
    }

    void processPacket(const LoRa::Packet& packet) {
        uint32_t now = millis();
        remoteState.deviceId = packet.source;
        remoteState.lastPacketTime = now;
        remoteState.rssi = packet.rssi;
        remoteState.snr = packet.snr;

        switch (packet.type) {
            case LoRa::PacketType::HEARTBEAT:
                processHeartbeat(packet);
                break;
            
            case LoRa::PacketType::GPS_TELEMETRY:
                processGpsTelemetry(packet);
                break;

            default:
                break;
        }
    }

    void processHeartbeat(const LoRa::Packet& packet) {
        if (packet.length != sizeof(Payload::Heartbeat))
            return;

        remoteState.connected = true;

        Payload::Heartbeat payload{};
        uint32_t now = millis();

        memcpy(&payload, packet.payload, sizeof(payload));
        
        remoteState.lastHeartbeatTime = now;

        remoteState.mode = payload.mode;
        remoteState.state = payload.state;
        remoteState.batteryStatus = payload.batteryStatus;
        remoteState.batteryVoltage = static_cast<float>(payload.batteryVoltage) / 100.0f;
        remoteState.gpsStatus = payload.gpsStatus;
        remoteState.errors = payload.errors;
        remoteState.warnings = payload.warnings;   

        printRemoteState();
    }

    void processGpsTelemetry(const LoRa::Packet& packet) {
        if (packet.length != sizeof(Payload::GpsTelemetry))
            return;

        remoteState.connected = true;

        Payload::GpsTelemetry payload{};
        uint32_t now = millis();

        memcpy(&payload, packet.payload, sizeof(payload));

        remoteState.lastGpsTelemetryTime = now;

        remoteState.latitude = payload.latitude / 100000.0;
        remoteState.longitude = payload.longitude / 100000.0;
        remoteState.altitude = payload.altitude / 100000.0;

        remoteState.maxAltitude = payload.maxAltitude / 100000.0;
        remoteState.minAltitude = payload.minAltitude / 100000.0;

        remoteState.speed = payload.speed / 100000.0;
        remoteState.course = payload.course / 100000.0;
        
        remoteState.satellites = payload.satellites;

        printGpsState();
    }

    void checkConnection() {
        if (!remoteState.connected)
            return;
        
        if (millis() - remoteState.lastHeartbeatTime > Config::Ground::CONNECTION_TIMEOUT) {
            remoteState.connected = false;
            System::setState(System::State::REMOTE_WAITING);
            return;
        }

        System::setState(System::State::REMOTE_CONNECTED);
    }

    void printRemoteState() {
        Serial.println();
        Serial.println("========================================");
        Serial.println("           REMOTE STATE");
        Serial.println("========================================");

        Serial.print("Device ID:          ");
        Serial.println(remoteState.deviceId);

        Serial.print("Connected:           ");
        Serial.println(remoteState.connected ? "YES" : "NO");

        Serial.println();

        Serial.println("SYSTEM");
        Serial.print("Mode:                ");
        Serial.println(remoteState.mode);

        Serial.print("State:               ");
        Serial.println(remoteState.state);

        Serial.print("Battery Status:      ");
        Serial.println(remoteState.batteryStatus);

        Serial.print("Battery Voltage:     ");
        Serial.print(remoteState.batteryVoltage, 2);
        Serial.println(" V");

        Serial.print("GPS Status:          ");
        Serial.println(remoteState.gpsStatus);

        Serial.print("Errors:              0x");
        Serial.println(remoteState.errors, HEX);

        Serial.print("Warnings:            0x");
        Serial.println(remoteState.warnings, HEX);

        Serial.println();

        Serial.println("TIMING");

        Serial.print("Last Packet:         ");
        Serial.print(millis() - remoteState.lastPacketTime);
        Serial.println(" ms ago");

        Serial.print("Last Heartbeat:      ");
        Serial.print(millis() - remoteState.lastHeartbeatTime);
        Serial.println(" ms ago");

        Serial.print("Last GPS Telemetry:  ");
        Serial.print(millis() - remoteState.lastGpsTelemetryTime);
        Serial.println(" ms ago");

        Serial.println();

        Serial.println("RADIO");

        Serial.print("RSSI:                ");
        Serial.print(remoteState.rssi, 2);
        Serial.println(" dBm");

        Serial.print("SNR:                 ");
        Serial.print(remoteState.snr, 2);
        Serial.println(" dB");

        Serial.println("========================================");
    }

    void printGpsState() {
        Serial.println();
        Serial.println("----------------------------------------");
        Serial.println("             REMOTE GPS");
        Serial.println("----------------------------------------");

        Serial.print("Latitude:            ");
        Serial.println(remoteState.latitude, 5);

        Serial.print("Longitude:           ");
        Serial.println(remoteState.longitude, 5);

        Serial.print("Altitude:            ");
        Serial.print(remoteState.altitude, 2);
        Serial.println(" m");

        Serial.print("Max Altitude:        ");
        Serial.print(remoteState.maxAltitude, 2);
        Serial.println(" m");

        Serial.print("Min Altitude:        ");
        Serial.print(remoteState.minAltitude, 2);
        Serial.println(" m");

        Serial.print("Speed:               ");
        Serial.print(remoteState.speed, 2);
        Serial.println(" km/h");

        Serial.print("Course:              ");
        Serial.print(remoteState.course, 2);
        Serial.println(" deg");

        Serial.print("Satellites:          ");
        Serial.println(remoteState.satellites);

        Serial.print("Last GPS Telemetry:  ");
        Serial.print(millis() - remoteState.lastGpsTelemetryTime);
        Serial.println(" ms ago");

        Serial.println("----------------------------------------");
    }
}