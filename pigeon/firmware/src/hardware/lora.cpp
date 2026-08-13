#include "hardware/lora.hpp"
#include "config.h"

#include <RadioLib.h>
#include <cstring> // Required for memcpy

#include "system/system.hpp"


namespace {
    // Radio
    LoRa::Status loraStatus;

    Module module(
        Pins::LORA_CS,
        Pins::LORA_DIO0,
        Pins::LORA_RST
    );

    SX1276 radio(&module);

    uint32_t lastReceiveTime = 0;
    uint32_t lastTransmitTime = 0;

    enum class Operation {
        RECEIVE,
        TRANSMIT
    };

    Operation currentOperation = Operation::RECEIVE;

    volatile bool radioInterrupt = false;
    void onRadioInterrupt();

    // receiving
    bool startReceive();
    bool packetAvailable = false;
    LoRa::Packet receivedPacket{};

    // transmitting
    uint8_t nextSequenceNumber = 0;
    bool transmissionPending = false;

    // helpers
    bool encodePacket(
        const LoRa::Packet& packet,
        uint8_t* buffer,
        uint8_t& length
    );

    bool decodePacket(
        LoRa::Packet& packet,
        const uint8_t* buffer,
        uint8_t length
    );
}

void LoRa::begin() {
    int16_t state;
    state = radio.begin(
        Config::LoRa::FREQUENCY,
        Config::LoRa::BANDWIDTH,
        Config::LoRa::SPREADING_FACTOR,
        Config::LoRa::CODING_RATE,
        Config::LoRa::SYNC_WORD,
        Config::LoRa::TX_POWER,
        Config::LoRa::PREAMBLE_LENGTH,
        Config::LoRa::GAIN
    );
    
    // show failed init
    if (state != RADIOLIB_ERR_NONE) {
        loraStatus = LoRa::Status::DISCONNECTED;
        System::setError(System::Error::LORA_DISCONNECTED);
        
        Serial.print("LoRa init failed: ");
        Serial.println(state);
        return;
    }

    // set radio inturupt
    radio.setPacketReceivedAction(onRadioInterrupt);
    radio.setPacketSentAction(onRadioInterrupt);
    
    if (!startReceive())
    return;
    
    loraStatus = LoRa::Status::READY;
    Serial.println("LoRa initialized");
}

void LoRa::update() {
    if (radioInterrupt) {
        radioInterrupt = false;
        
        if (currentOperation == Operation::TRANSMIT) {
            Serial.println("LoRa TX complete");
        
            // packetTransmitted = false;
            transmissionPending = false;
        
            lastTransmitTime = millis();
            System::setEvent(System::Event::LORA_TRANSMIT_FINISHED);
            startReceive();
        } else {
            Serial.println("LoRa RX complete");
            
            // get size of packet
            size_t length = radio.getPacketLength();
            if (length > Config::LoRa::MAX_PACKET_LENGTH) {
                System::setEvent(System::Event::LORA_PACKET_TOO_LARGE);
                Serial.println("LoRa packet too long");
                startReceive();
                return;
            }
                
            // receive packet
            uint8_t buffer[Config::LoRa::MAX_PACKET_LENGTH];
            if (radio.readData(buffer, length) != RADIOLIB_ERR_NONE) {
                Serial.println("LoRa packet read failed");
                System::setEvent(System::Event::LORA_RECEIVE_FAILED);
                startReceive();
                return;
            }
        
            if (decodePacket(receivedPacket, buffer, length)) {
                receivedPacket.rssi = radio.getRSSI();
                receivedPacket.snr = radio.getSNR();
        
                packetAvailable = true;
                lastReceiveTime = millis();
                System::setEvent(System::Event::LORA_RECEIVED);
            } else {
                System::setEvent(System::Event::LORA_PACKET_INVALID);
                Serial.println("LoRa packet decode failed");
            }
            
            // restart receive
            startReceive();
        }
    }
}

LoRa::Status LoRa::getStatus() {
    return loraStatus;
}

// return available packet
bool LoRa::receive(Packet& packet) {
    if (!packetAvailable)
        return false;

    Serial.println("LoRa packet received");

    memcpy(&packet, &receivedPacket, sizeof(Packet));
    packetAvailable = false;
    return true;
}

bool LoRa::transmit(const Packet& packet) {
    if (transmissionPending)
        return false;
        
    uint8_t buffer[Config::LoRa::MAX_PACKET_LENGTH];
    uint8_t length;
    
    if (!encodePacket(packet, buffer, length)) {
        Serial.println("LoRa packet encode failed");
        return false;
    }
    
    int16_t state = radio.startTransmit(buffer, length);

    if (state != RADIOLIB_ERR_NONE) {
        System::setEvent(System::Event::LORA_TRANSMIT_FAILED);
        Serial.print("LoRa transmit failed: ");
        Serial.println(state);
        return false;
    }

    currentOperation = Operation::TRANSMIT;
    transmissionPending = true;
        
    System::setEvent(System::Event::LORA_TRANSMIT_STARTED);
    Serial.println("LoRa packet transmitting");

    return true;
}

LoRa::Packet LoRa::createPacket(
    uint8_t destination, 
    uint8_t source, 
    PacketType type, 
    const uint8_t* payload,
    uint8_t length
) {
    Packet packet{};

    packet.destination = destination;
    packet.source = source;
    packet.type = type;

    packet.sequenceNumber = nextSequenceNumber++;
    
    if (length > Config::LoRa::MAX_PAYLOAD_LENGTH) {
        length = Config::LoRa::MAX_PAYLOAD_LENGTH;
    }
    packet.length = length;

    if (length > 0) {
        memcpy(packet.payload, payload, length); // Copy payload data
    }

    return packet;
}

float LoRa::getRSSI() {
    return radio.getRSSI();
}

float LoRa::getSNR() {
    return radio.getSNR();
}

uint32_t LoRa::getLastReceiveTime() {
    return lastReceiveTime;
}

uint32_t LoRa::getLastTransmitTime() {
    return lastTransmitTime;

}

// const char* LoRa::packetTypeName(LoRa::PacketType type) {
//     switch (type) {
//         case LoRa::PacketType::HEARTBEAT: return "HEARTBEAT";
//         case LoRa::PacketType::TELEMETRY: return "TELEMETRY";
//         case LoRa::PacketType::COMMAND:   return "COMMAND";
//         case LoRa::PacketType::ERROR:     return "ERROR";
//         default:                          return "UNKNOWN";
//     }
// }

namespace {
    void onRadioInterrupt() {
        radioInterrupt = true;
    }

    bool startReceive() {
        int16_t state = radio.startReceive();

        if (state != RADIOLIB_ERR_NONE) {
            loraStatus = LoRa::Status::DISCONNECTED;
            System::setError(System::Error::LORA_DISCONNECTED);
            System::setEvent(System::Event::LORA_RECEIVE_FAILED);
            Serial.print("LoRa receive start failed: ");
            Serial.println(state);
            return false;
        }

        if (loraStatus == LoRa::Status::DISCONNECTED) {
            System::clearError(System::Error::LORA_DISCONNECTED);
        }
     
        currentOperation = Operation::RECEIVE;
        loraStatus = LoRa::Status::READY;
        
        return true;
    }

    bool encodePacket(
        const LoRa::Packet& packet,
        uint8_t* buffer,
        uint8_t& length
    ) {
        if (packet.length > Config::LoRa::MAX_PAYLOAD_LENGTH)
            return false;

        length = Config::LoRa::PACKET_HEADER_LENGTH + packet.length;

        buffer[0] = packet.destination;
        buffer[1] = packet.source;
        buffer[2] = static_cast<uint8_t>(packet.type);
        buffer[3] = packet.sequenceNumber;
        buffer[4] = packet.length;

        if (packet.length > 0) {
            memcpy(buffer + Config::LoRa::PACKET_HEADER_LENGTH, packet.payload, packet.length);
        }

        return true;
    }

    bool decodePacket(
        LoRa::Packet& packet,
        const uint8_t* buffer,
        uint8_t length
    ) {
        // check if packet is shorter than minimum header length
        if (length < Config::LoRa::PACKET_HEADER_LENGTH)
            return false;

        uint8_t payloadLength = buffer[4];

        // check if payload is bigger than payload length
        if (payloadLength > Config::LoRa::MAX_PAYLOAD_LENGTH)
            return false;

        // check if packet is equal to the expected packet length
        if (length != Config::LoRa::PACKET_HEADER_LENGTH + payloadLength)
            return false;

        packet.destination = buffer[0];
        packet.source = buffer[1];
        packet.type = static_cast<LoRa::PacketType>(buffer[2]);
        packet.sequenceNumber = buffer[3];
        packet.length = buffer[4];

        if (payloadLength > 0) {
            memcpy(packet.payload, buffer + Config::LoRa::PACKET_HEADER_LENGTH, payloadLength);
        }

        return true;
    }
}