#include "MPU9250.h"

// Setup IMU connection
MPU9250 IMU(Wire,0x68);
// IMU variables
int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;

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
}

void loop(){

  distanceTraveled = 0;
  iterator = 0;
  while (distanceTraveled <= 90 && turnReached == false) {
    IMU.readSensor();

    norm = sqrt(gyroX * gyroX + gyroY * gyroY + gyroZ * gyroZ);
    gyroZ /= norm;
    
    gyroZ = IMU.getGyroZ_rads();
    delay(10);
    if (abs(gyroZ) > 0.1) {
      distanceTraveled += gyroZ;
    }

    Serial.println(distanceTraveled);
  }
  Serial.println("Turn Reached!");
  turnReached = true;
}
