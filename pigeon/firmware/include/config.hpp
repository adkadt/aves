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
    constexpr uint32_t GPS_BAUD_RATE = 9600;
    constexpr uint32_t GPS_TIMEOUT_MS = 1000;

    constexpr double GPS_MIN_ALPHA = 0.1;
    constexpr double GPS_MAX_ALPHA = 0.8;
    constexpr float GPS_MAX_SPEED = 50.0f;

    // LoRa Config
    constexpr uint8_t LORA_MAX_PAYLOAD_LENGTH = 255;
}

#endif