#include "Altimeter.h"

void Altimeter::update(double raw_altitude) {
    // update agl altitude
    if (m_groundAltitudeM != -999) {
        m_curAltitudeAGL = raw_altitude - m_groundAltitudeM;
    } else {
        m_curAltitudeAGL = 0.0;
    }

    // if in ground mode, set ground altitude
    if (m_groundMode) {
        m_groundBuffer.push(raw_altitude);
        m_groundAltitudeM = m_groundBuffer.getAverage();
    }
}

double Altimeter::getAltitudeAGL() {
    return m_curAltitudeAGL;
}

void Altimeter::setGroundMode(bool ground_mode) {
    m_groundMode = ground_mode;
}

bool Altimeter::getGroundMode() {
    return m_groundMode;
}