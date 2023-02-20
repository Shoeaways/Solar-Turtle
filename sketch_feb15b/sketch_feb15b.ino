#include <Wire.h>
#include <math.h>
#include "MPU9250.h"

float theta, phi;
float thetaG = 0, phiG = 0;

float dt;
float millisOld;

// Setup IMU connection
MPU9250 IMU(Wire,0x68);
// IMU variables
int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float roll, pitch, yaw;

int startTime;
int endTime;
int iterator;
float distanceTraveled;
bool turnReached = false;

float norm;

void setup(){
  Serial.begin(9600);
  IMU.begin();
  // setting the accelerometer full scale range to +/-2G 
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_2G);
  // setting the gyroscope full scale range to +/-250 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
  // setting DLPF bandwidth to 5 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_5HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19); 
  millisOld = millis();
}

void loop(){
  IMU.readSensor();
  accelX = IMU.getAccelX_mss();
  accelY = IMU.getAccelY_mss();
  accelZ = IMU.getAccelZ_mss();
  gyroX = IMU.getGyroX_rads();
  gyroY = IMU.getGyroY_rads();
  gyroZ = IMU.getGyroZ_rads();
  magX = IMU.getMagX_uT();
  magY = IMU.getMagY_uT();
  magZ = IMU.getMagZ_uT(); 

  theta = atan2(accelX/9.8, accelZ/9.8) * 180 / PI;
  phi = atan2(accelY/9.8, accelZ/9.8) * 180 / PI;

  dt = (millis() - millisOld)/1000;
  millisOld = millis();

  thetaG = thetaG + gyroY * dt;
  phiG = phiG + gyroX * dt;

  Serial.println(thetaG);
  Serial.println(phiG);
  Serial.println();
  
  delay(100);
}