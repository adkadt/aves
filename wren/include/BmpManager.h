#ifndef BMP_MANAGER_H
#define BMP_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BMP5xx.h"
#include <optional>

#define SEALEVELPRESSURE_HPA (1013.25)

struct BmpData {
    double temperature;
    double pressure;
    double altitude;
};

class BmpManager {    
    private:
        Adafruit_BMP5xx bmp_;

    public:
        bool Begin();
        bool SetMode(const bmp5xx_powermode_t power_mode, const std::optional<bmp5xx_odr_t> odr = BMP5XX_ODR_240_HZ);
        bool SetTempOSR(const bmp5xx_oversampling_t osr);
        bool SetPressureOSR(const bmp5xx_oversampling_t osr);
        bool setIIRCoeff(const bmp5xx_iir_filter_t coef);
        bool Update();
        BmpData GetBmpData();
};

#endif