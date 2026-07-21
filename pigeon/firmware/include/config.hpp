#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <cstdint>

namespace Pins {
    // LED
    constexpr uint8_t LED = 21;

    // GPS
    constexpr uint8_t GPS_RX = 13;
    constexpr uint8_t GPS_TX = 12;

    // LoRa
    constexpr uint8_t LORA_SCK = 0;
    constexpr uint8_t LORA_MISO = 0;
    constexpr uint8_t LORA_MOSI = 0;
    constexpr uint8_t LORA_CS = 0;
    constexpr uint8_t LORA_RST = 0;
    constexpr uint8_t LORA_IRQ = 0; // G0, IRQ/ DIO0
}

namespace Config {
    // Serial Config
    constexpr uint32_t BAUD_RATE = 115200;
    
    // GPS Config
    constexpr uint32_t GPS_BAUD_RATE = 9600;
    constexpr uint32_t GPS_TIMEOUT_MS = 1000;
}

#endif