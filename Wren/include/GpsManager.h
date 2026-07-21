#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>

struct locationData {
    double lat;
    double lng;
    double alt;
    uint32_t sats;
    bool is_valid;
};

class GpsManager {    
    public:
    GpsManager(HardwareSerial& serial_port, uint32_t baud = 9600);
    void Begin();
    bool Update();
    locationData GetLocationData();

    private:
        TinyGPSPlus gps_;
        HardwareSerial* serial_port_;
        uint32_t baud_;
};

#endif