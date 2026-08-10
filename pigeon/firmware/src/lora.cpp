#include "lora.hpp"
#include "config.h"

#include <RadioLib.h>
#include <cstring> // Required for memcpy

namespace {
    Module module(
        Pins::LORA_CS,
        Pins::LORA_DIO0,
        Pins::LORA_RST
    );

    SX1276 radio(&module);

    int state = 0;
}

void LoRa::begin() {
    state = radio.begin();

    if (state != RADIOLIB_ERR_NONE) {
        // TODO: ADD ERROR HANDLING
    }
}

void LoRa::update() {
    
}

bool LoRa::connected() {
    return state == RADIOLIB_ERR_NONE;
}

bool LoRa::available() {
    return false;
}

bool LoRa::send(const Packet& packet) {
    return false;

    // Packet packet;
    packet.rssi = radio.getRSSI();
    packet.snr = radio.getSNR();
}

LoRa::Packet LoRa::createPacket(
    uint8_t destination, 
    uint8_t source, 
    PacketType type, 
    const uint8_t* payload,
    uint8_t length
) {
    Packet packet;
    packet.destination = destination;
    packet.source = source;
    packet.type = static_cast<uint8_t>(type); // Cast enum to underlying type
    memcpy(packet.payload, payload, length); // Copy payload data
    packet.rssi = 0;
    packet.snr = 0;

    return packet;
}