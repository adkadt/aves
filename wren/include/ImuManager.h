#ifndef IMU_MANAGER_H
#define IMU_MANAGER_H

#include <Arduino.h>
#include <Adafruit_BNO08x.h>

// Define pins for BNO085
#define BNO085_RST 17

struct imuData {
    float accel_x, accel_y, accel_z;
    float gyro_x, gyro_y, gyro_z;
    float quat_i, quat_j, quat_k, quat_real;
    uint8_t accuracy;
};

class ImuManager {
    private:
        Adafruit_BNO08x bno08x_;
        sh2_SensorValue_t sensor_value_;
        imuData current_data_;

    public:
        ImuManager();
        bool Begin();
        bool Update();
        imuData GetImuData();
        float GetRoll();
        float GetPitch();
        float GetYaw();
        float GetTilt();
};

#endif