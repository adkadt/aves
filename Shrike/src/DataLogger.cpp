#include "DataLogger.h"

DataLogger::DataLogger(uint8_t cs_pin, unsigned long write_interval_ms) 
    : m_csPin(cs_pin), m_writeIntervalMs(write_interval_ms) {}

bool DataLogger::begin(bool simulation_mode) {
    SPI.setRX(20);  // MISO
    SPI.setTX(19);  // MOSI
    SPI.setSCK(18); // SCK
    SPI.begin();    // Boot the SPI bus

    SdSpiConfig sdConfig(m_csPin, DEDICATED_SPI, SD_SCK_MHZ(16), &SPI);

    if (!SD.begin(sdConfig)) {
        return false;
    }

    if (simulation_mode) {
        flightLog = SD.open("SIM_OUT.csv", O_WRITE | O_CREAT | O_TRUNC);
        if (flightLog) {
            flightLog.println("Time(ms),State,AGL_Alt(m),Raw_Pressure(Pa),Apogee(m)");
            flightLog.sync();
            m_sdActive = true;
            Serial.println("SIMULATION MODE ACTIVE: File Loaded.");
        } else {
            Serial.println("ERROR: COULD NOT CREATE SIM_OUT.CSV");
            return false;
        }
    } else {
        char filename[15];
        strcpy(filename, "FLIGHT00.CSV");
        for (uint8_t i = 0; i < 100; i++) {
            filename[6] = '0' + i / 10;
            filename[7] = '0' + i % 10;
            if (!SD.exists(filename)) {
                break;
            }
        }
    
        flightLog = SD.open(filename, O_WRITE | O_CREAT);
        if (flightLog) {
            flightLog.println("Time(ms),State,AGL_Alt(m),Raw_Pressure(Pa),Apogee(m)");
            flightLog.sync();
            m_sdActive = true;
            Serial.printf("SD Card Active. Logging to: %s\n", filename);
        } else {
            Serial.printf("ERROR: COULD NOT CREATE %s\n", filename);
            return false;
        }
    }
    return true;
}

void DataLogger::logData(unsigned long time_ms, int state, double agl, double raw_pressure, double apogee) {
    if (!m_sdActive) return;
    flightLog.printf("%lu,%d,%.2f,%.2f,%.2f\n", time_ms, state, agl, raw_pressure, apogee);
}

void DataLogger::sync() {
    if (m_sdActive && (millis() - m_lastSdWrite >= m_writeIntervalMs)) {
        flightLog.sync();
        m_lastSdWrite = millis();
    }
}

void DataLogger::close() {
    if (m_sdActive) flightLog.close();
    m_sdActive = false;
}

bool DataLogger::isActive() const {
    return m_sdActive;
}
