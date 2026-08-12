#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>

namespace Pins {
    // LED
    constexpr uint8_t LED = 21;
    
    // GPS
    constexpr uint8_t GPS_RX = 2;
    constexpr uint8_t GPS_TX = 3;
    constexpr uint8_t GPS_FIX = 4;
    constexpr uint8_t GPS_PPS = 5;   
    
    // LoRa
    constexpr uint8_t LORA_SCK = 11;
    constexpr uint8_t LORA_MISO = 10;
    constexpr uint8_t LORA_MOSI = 9;
    constexpr uint8_t LORA_CS = 8;
    constexpr uint8_t LORA_RST = 7;
    constexpr uint8_t LORA_DIO0 = 12; // G0, IRQ/ DIO0

    // Misc
    constexpr uint8_t VBAT_SENSE = 1;
    constexpr uint8_t BUZZER = 6;
    constexpr uint8_t MODE_SWITCH = 13;
}

namespace Config {
    // Serial Config
    constexpr uint32_t BAUD_RATE = 115200;
    
    // GPS Config
    namespace GPS {
        constexpr uint32_t BAUD_RATE = 9600;
        constexpr uint32_t TIMEOUT_MS = 3000;
    
        constexpr double MIN_ALPHA = 0.1;
        constexpr double MAX_ALPHA = 0.8;
        constexpr float MAX_SPEED = 50.0f;
    }

    // LoRa Config
    namespace LoRa {
        constexpr uint8_t MAX_PAYLOAD_LENGTH = 64;
        constexpr uint8_t PACKET_HEADER_LENGTH = 5; // this value is based on code [don't change]
        constexpr uint8_t MAX_PACKET_LENGTH = LoRa::MAX_PAYLOAD_LENGTH + LoRa::PACKET_HEADER_LENGTH;
    
        // LoRa Radio
        constexpr float FREQUENCY = 915.0;
        constexpr float BANDWIDTH = 125.0;
        constexpr uint8_t SPREADING_FACTOR = 9;
        constexpr uint8_t CODING_RATE = 7;
        constexpr uint8_t SYNC_WORD = 0x12;
        constexpr int8_t TX_POWER = 10;
        constexpr uint8_t PREAMBLE_LENGTH = 8;
        constexpr uint8_t GAIN = 0;
    } 

    // Battery
    namespace Battery {
        constexpr float HIGH_VOLTAGE = 4.2f;
        constexpr float MEDIUM_VOLTAGE = 4.0f;
        constexpr float LOW_VOLTAGE = 3.8f;
        constexpr float CRITICAL_VOLTAGE = 3.6f;
    }
}

#endif