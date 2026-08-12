#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>
#include "system/system.hpp"

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
    constexpr System::Mode PRIMARY_MODE = System::Mode::FLIGHT;
    constexpr System::Mode SECONDARY_MODE = System::Mode::GROUND;

    // Serial Config
    constexpr uint32_t BAUD_RATE = 115200;
    
    // GPS Config
    namespace GPS {
        constexpr uint32_t BAUD_RATE = 9600;
        constexpr uint32_t TIMEOUT_MS = 3000;
    
        // EMA
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
        // Voltage divider
        constexpr float DIVIDER_TOP_RESISTOR = 100000.0f;
        constexpr float DIVIDER_BOTTOM_RESISTOR = 100000.0f;
        
        // Sampling
        constexpr uint32_t SAMPLE_INTERVAL_MS = 100; 

        // Battery Thresholds
        constexpr float MIN_USB_VOLTAGE = 4.7f;
        constexpr float LOW_VOLTAGE = 3.7f;
        constexpr float CRITICAL_VOLTAGE = 3.3f;

        // EMA
        constexpr float VOLTAGE_FILTER_ALPHA  = 0.3f;
    }
}

#endif