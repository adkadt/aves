#include "GpsManager.h"

// constructor sets serial port and serial baud rate to gps
GpsManager::GpsManager(HardwareSerial& serial_port, uint32_t baud) 
    : serial_port_(&serial_port),
      baud_(baud)
{ }

// starts gps serial port
void GpsManager::Begin() {
    // start initial contact with gps
    serial_port_->begin(9600);
    delay(100);
    
    // set 115200 baud rate on gps module
    serial_port_->println("$PMTK251,115200*1F"); 
    delay(100);
    
    serial_port_->end();
    serial_port_->begin(baud_);
    delay(100);

    // Set Aeronautical Mode (< 4G)
    serial_port_->println("$PMTK869,1,1*35");
    delay(100);

    // set 10Hz Update rate
    serial_port_->println("$PMTK220,100*2F");
}

// updates tinygps with the latest data
bool GpsManager::Update() {
    bool updated_status = false;
    // loops until serial port is cleared
    while (serial_port_->available() > 0) {
        // reads data from serial port and encodes it in tinygps
        if (gps_.encode(serial_port_->read())) {
            updated_status = gps_.satellites.isUpdated();
        }
    }
    return updated_status;
}

// gets the location data from tinygps
locationData GpsManager::GetLocationData() {
    locationData data;
    data.lat = gps_.location.lat();
    data.lng = gps_.location.lng();
    data.sats = gps_.satellites.value();
    data.alt = gps_.altitude.feet();
    data.is_valid = gps_.location.isValid();
    return data;
}

// will add time information

// can add other functions for other info