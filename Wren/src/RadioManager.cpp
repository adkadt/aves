#include "RadioManager.h"

RadioManager::RadioManager() : rf95_(RFM95_CS, RFM95_INT) {
}

bool RadioManager::Begin() {
    pinMode(RFM95_RST, OUTPUT);
    digitalWrite(RFM95_RST, HIGH);

    // Manual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    if (!rf95_.init()) {
        return false;
    }

    if (!rf95_.setFrequency(RF95_FREQ)) {
        return false;
    }

    // Tx power to 23 dBm (max supported by module)
    rf95_.setTxPower(13, false);

    // Setup Addressing
    rf95_.setThisAddress(FC_ADDR);
    rf95_.setHeaderTo(GS_ADDR);

    return true;
}

void RadioManager::Transmit(String data) {
    // Send length() + 1 to include the null terminator so the receiver knows when the string ends
    rf95_.send((uint8_t *)data.c_str(), data.length() + 1);
    rf95_.waitPacketSent();
}

String RadioManager::Receive() {
    if (rf95_.available()) {
        uint8_t buf[RH_RF95_MAX_MESSAGE_LEN + 1];
        uint8_t len = sizeof(buf) - 1;
        if (rf95_.recv(buf, &len)) {
            buf[len] = 0; // Force null termination for safety
            return String((char *)buf);
        }
    }
    return "";
}