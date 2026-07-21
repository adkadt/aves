#include "ImuManager.h"

ImuManager::ImuManager() : bno08x_(BNO085_RST) {
    // Initialize struct to zero
    current_data_ = {0};
}

bool ImuManager::Begin() {
    // Initialize BNO08x on default I2C address (0x4A)
    if (!bno08x_.begin_I2C(BNO08x_I2CADDR_DEFAULT, &Wire)) {
        return false;
    }

    // Enable reports: Accelerometer, Gyroscope, Rotation Vector
    // 5000us = 5ms = 200Hz update rate
    if (!bno08x_.enableReport(SH2_ACCELEROMETER)) return false;
    if (!bno08x_.enableReport(SH2_GYROSCOPE_CALIBRATED)) return false;
    if (!bno08x_.enableReport(SH2_GAME_ROTATION_VECTOR)) return false;

    return true;
}

bool ImuManager::Update() {
    // Check if sensor reset occurred
    if (bno08x_.wasReset()) {
        bno08x_.enableReport(SH2_ACCELEROMETER);
        bno08x_.enableReport(SH2_GYROSCOPE_CALIBRATED);
        bno08x_.enableReport(SH2_GAME_ROTATION_VECTOR);
    }

    // Process a sensor event if available
    if (bno08x_.getSensorEvent(&sensor_value_)) {
        switch (sensor_value_.sensorId) {
            case SH2_ACCELEROMETER:
                current_data_.accel_x = sensor_value_.un.accelerometer.x;
                current_data_.accel_y = sensor_value_.un.accelerometer.y;
                current_data_.accel_z = sensor_value_.un.accelerometer.z;
                break;
            case SH2_GYROSCOPE_CALIBRATED:
                current_data_.gyro_x = sensor_value_.un.gyroscope.x;
                current_data_.gyro_y = sensor_value_.un.gyroscope.y;
                current_data_.gyro_z = sensor_value_.un.gyroscope.z;
                break;
            case SH2_GAME_ROTATION_VECTOR:
                current_data_.quat_i = sensor_value_.un.gameRotationVector.i;
                current_data_.quat_j = sensor_value_.un.gameRotationVector.j;
                current_data_.quat_k = sensor_value_.un.gameRotationVector.k;
                current_data_.quat_real = sensor_value_.un.gameRotationVector.real;
                current_data_.accuracy = sensor_value_.status;
                break;
        }
        return true; // Data was updated
    }
    return false;
}

imuData ImuManager::GetImuData() {
    return current_data_;
}

float ImuManager::GetRoll() {
    float t0 = 2.0f * (current_data_.quat_real * current_data_.quat_i + current_data_.quat_j * current_data_.quat_k);
    float t1 = 1.0f - 2.0f * (current_data_.quat_i * current_data_.quat_i + current_data_.quat_j * current_data_.quat_j);
    return atan2(t0, t1) * 180.0f / PI;
}

float ImuManager::GetPitch() {
    float t2 = +2.0f * (current_data_.quat_real * current_data_.quat_j - current_data_.quat_k * current_data_.quat_i);
    // Clamp value to avoid NaN errors
    if (t2 > 1.0f) t2 = 1.0f;
    if (t2 < -1.0f) t2 = -1.0f;
    return asin(t2) * 180.0f / PI;
}

float ImuManager::GetYaw() {
    float t3 = +2.0f * (current_data_.quat_real * current_data_.quat_k + current_data_.quat_i * current_data_.quat_j);
    float t4 = +1.0f - 2.0f * (current_data_.quat_j * current_data_.quat_j + current_data_.quat_k * current_data_.quat_k);
    return atan2(t3, t4) * 180.0f / PI;
}

float ImuManager::GetTilt() {
    // Calculates angle between sensor Z-axis and vertical (gravity)
    float t = 1.0f - 2.0f * (current_data_.quat_i * current_data_.quat_i + current_data_.quat_j * current_data_.quat_j);
    // Clamp to handle potential floating point errors
    if (t > 1.0f) t = 1.0f;
    if (t < -1.0f) t = -1.0f;
    return acos(t) * 180.0f / PI;
}