#ifndef ALTIMETER_H
#define ALTIMETER_H

#include "RingBuffer.h"

#define GROUND_BUFFER_SIZE 120

class Altimeter {
    private:
        bool m_groundMode = true;
        RingBuffer<double, GROUND_BUFFER_SIZE> m_groundBuffer;
        double m_groundAltitudeM = -999;
        double m_curAltitudeAGL = 0.0;
    
    public:
        void update(double raw_altitude);
        double getAltitudeAGL();
        void setGroundMode(bool ground_mode);
        bool getGroundMode();
};

#endif