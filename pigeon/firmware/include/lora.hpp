#ifndef LORA_HPP
#define LORA_HPP

#include <cstdint>
#include "config.hpp"

namespace LoRa {

    struct Packet {
        uint8_t destination;
        uint8_t source;
        uint8_t type;
        uint8_t payload[Config::LORA_MAX_PAYLOAD_LENGTH];
        int16_t rssi;
        int8_t snr;
    };

    enum class PacketType : uint8_t {
        HEARTBEAT,
        GPS,
        TELEMETRY,
        COMMAND,
        ACK,
        DEBUG,
        ERROR
    };

    void begin();
    void update();

    bool connected();

    bool available();
    bool send(const Packet& packet);
    bool receive(Packet& packet);

    Packet LoRa::createPacket(
        uint8_t destination, 
        uint8_t source, 
        PacketType type, 
        const uint8_t* payload,
        uint8_t length
    );

}

#endif // LORA_HPP