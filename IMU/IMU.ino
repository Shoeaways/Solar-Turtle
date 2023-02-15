#include <MPU9250.h>
#include <Wire.h>
#include <math.h>

bfs::MPU9250 mpu;

#define Kp 0.5 // Proportional gain
#define Ki 0.0 // Integral gain

float q0 = 1.0, q1 = 0.0, q2 = 0.0, q3 = 0.0; // Quaternion elements representing the estimated orientation
float exInt = 0.0, eyInt = 0.0, ezInt = 0.0; // Integral error terms for the complementary filter
float yawEst = 0.0; // Estimated yaw angle (in degrees)

void setup() {
  Wire.begin();
  Serial.begin(9600);

  mpu.setup();
  mpu.calibrateGyro();
}

void loop() {
  mpu.update();

  float ax = mpu.getAccelX();
  float ay = mpu.getAccelY();
  float az = mpu.getAccelZ();
  float gx = mpu.getGyroX();
  float gy = mpu.getGyroY();
  float gz = mpu.getGyroZ();

  // Convert accelerometer readings to pitch and roll angles
  float pitch = atan2(-ax, sqrt(ay*ay + az*az));
  float roll = atan2(ay, az);

  // Compute complementary filter
  float dt = mpu.getDeltaT() / 1000.0;
  float gyroScale = 0.0175; // Gyro scale factor (degrees per second)
  float qDot1 = 0.5 * (-q1 * gx - q2 * gy - q3 * gz);
  float qDot2 = 0.5 * (q0 * gx + q2 * gz - q3 * gy);
  float qDot3 = 0.5 * (q0 * gy - q1 * gz + q3 * gx);
  float qDot4 = 0.5 * (q0 * gz + q1 * gy - q2 * gx);
  exInt += qDot1 * dt;
  eyInt += qDot2 * dt;
  ezInt += qDot3 * dt;
  q0 += qDot4 * dt;
  q1 -= qDot1 * dt;
  q2 -= qDot2 * dt;
  q3 -= qDot3 * dt;

  // Normalize quaternion
  float norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  q0 /= norm;
  q1 /= norm;
  q2 /= norm;
  q3 /= norm;

  // Use complementary filter to combine accelerometer and gyro readings
  float pitchEst = q0*q0*pitch - 2*q0*q2*roll + 2*q0*q3*gyroScale*gy;
  float rollEst = q0*q0*roll + 2*q0*q1*gyroScale*gx + 2*q0*q2*pitch;
  pitchEst = (pitchEst * Kp + exInt * Ki) * 180.0 / M_PI;
  rollEst = (rollEst * Kp + eyInt * Ki) * 180.0 / M_PI;

  // Compute estimated yaw angle
  float yawAccel = atan2(ay, ax) * 180.0 / M_PI; // Yaw angle based on accelerometer readings
  yawEst = 0.98 * (yawEst + gyroScale * gz * dt) + 0.02 * yawAccel; // Complementary filter to combine accelerometer and gyro

  Serial.println(rollEst);
  Serial.println(pitchEst);
  Serial.println(yawEst);

  delay(250);
}
