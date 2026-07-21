#include "BmpManager.h"

bool BmpManager::Begin() {
    if (!bmp_.begin(BMP5XX_ALTERNATIVE_ADDRESS, &Wire)) {
        // do error thing here
        // return some sort of error to send to ground station or led
        // probably return string through pointer or something
        return false;
    }

    if(!bmp_.enablePressure()){
        return false;
    }

    if (!bmp_.configureInterrupt(BMP5XX_INTERRUPT_LATCHED, BMP5XX_INTERRUPT_ACTIVE_HIGH, BMP5XX_INTERRUPT_PUSH_PULL, BMP5XX_INTERRUPT_DATA_READY, true)) {
        return false;
    }

    // bmp found
    return true;
}

bool BmpManager::SetMode(const bmp5xx_powermode_t power_mode, const std::optional<bmp5xx_odr_t> odr) {
    if (!bmp_.setPowerMode(power_mode)) {
        return false;
    }

    if ((power_mode == BMP5XX_POWERMODE_NORMAL || power_mode == BMP5XX_POWERMODE_CONTINUOUS) && odr.has_value()) {
        if (!bmp_.setOutputDataRate(odr.value())) {
            return false;
        }
    }
    
    return true;
}

// probably 2X
bool BmpManager::SetTempOSR(const bmp5xx_oversampling_t osr) {
    return bmp_.setTemperatureOversampling(osr);
}

// probably 16X
bool BmpManager::SetPressureOSR(const bmp5xx_oversampling_t osr) {
    return bmp_.setPressureOversampling(osr);
}

// probably 3
bool BmpManager::setIIRCoeff(const bmp5xx_iir_filter_t coef) {
    return bmp_.setIIRFilterCoeff(coef);
}

bool BmpManager::Update() {
    if (bmp_.dataReady()) {
        return bmp_.performReading();
    }
    return false;
}

BmpData BmpManager::GetBmpData() {
    BmpData data;
    data.pressure = bmp_.readPressure();
    data.temperature = bmp_.readTemperature();
    data.altitude = bmp_.readAltitude();
    return data;
}