#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// Global object for the MPU6050
Adafruit_MPU6050 mpu;

// Struct to hold the IMU data globally
struct IMUData {
  float ax, ay, az; // Accelerometer values
  float gx, gy, gz; // Gyroscope values
};
IMUData imuData;

// Timer for non-blocking reads
unsigned long lastImuRead = 0;

void setupIMU() {
  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    // You could add a loop here to halt if the IMU is critical
    // while (1) { delay(10); }
  } else {
    Serial.println("MPU6050 Found!");
  }

  // Set sensor ranges
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  delay(100);
}

void readIMU() {
  // Only read every 50ms
  if (millis() - lastImuRead > 50) {
    lastImuRead = millis();
    
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Update the global struct with the latest readings
    imuData.ax = a.acceleration.x;
    imuData.ay = a.acceleration.y;
    imuData.az = a.acceleration.z;

    imuData.gx = g.gyro.x;
    imuData.gy = g.gyro.y;
    imuData.gz = g.gyro.z;
  }
}

