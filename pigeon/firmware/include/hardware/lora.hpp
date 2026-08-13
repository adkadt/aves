#pragma once

#include <cstdint>
#include "config.hpp"

namespace LoRa {

    enum class Status {
        DISCONNECTED,
        READY
    };
    
    enum class PacketType : uint8_t {
        HEARTBEAT,
        GPS_TELEMETRY,
        COMMAND,
        RESPONSE,
        EVENT,
        ERROR
    };

    struct Packet {
        uint8_t destination;
        uint8_t source;
        PacketType type;
        
        uint8_t sequenceNumber;
        uint8_t length;
        uint8_t payload[Config::LoRa::MAX_PAYLOAD_LENGTH];

        float rssi;
        float snr;
    };

    // default functions
    void begin();
    void update();

    // Status
    Status getStatus();
    
    // Reveiving
    bool receive(Packet& packet);
    
    // Transmitting
    bool transmit(const Packet& packet);

    // Packet Contruction
    Packet createPacket(
        uint8_t destination, 
        uint8_t source, 
        PacketType type, 
        const uint8_t* payload,
        uint8_t length
    );

    // Radio Activity Data
    float getRSSI();
    float getSNR();
    uint32_t getLastReceiveTime();
    uint32_t getLastTransmitTime();

    const char* packetTypeName(LoRa::PacketType type);
}