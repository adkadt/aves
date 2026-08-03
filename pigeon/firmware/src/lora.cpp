#include "lora.hpp"
#include "config.h"

#include <RadioLib.h>

namespace {
    SX1262 radio = new Module(
        Pins::LORA_SCK, 
        Pins::LORA_MISO, 
        Pins::LORA_MOSI, 
        Pins::LORA_CS
    );

    int state = 0;
}

void LoRa::begin() {
    state = radio.begin();
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
}

bool LoRa::receive(Packet& packet) {
    return false;
}