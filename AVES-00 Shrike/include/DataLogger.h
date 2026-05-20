#ifndef DATALOGGER_H
#define DATALOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include "SdFat.h"

class DataLogger {
private:
    SdFs SD;
    FsFile flightLog;
    bool m_sdActive = false;
    uint8_t m_csPin;
    unsigned long m_writeIntervalMs;
    unsigned long m_lastSdWrite = 0;

public:
    DataLogger(uint8_t cs_pin, unsigned long write_interval_ms);
    bool begin(bool simulation_mode);
    void logData(unsigned long time_ms, int state, double agl, double raw_pressure, double apogee);
    void sync();
    void close();
    bool isActive() const;
};

#endif
