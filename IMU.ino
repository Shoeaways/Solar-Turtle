#include "MPU9250.h"

// Setup IMU connection
MPU9250 IMU(Wire,0x68);
// IMU variables
int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float pitch, roll, yaw;
float Yh, Xh;


void setup() {
  // Initialize IMU
Serial.begin(9600);
status = IMU.begin();

// setting the accelerometer full scale range to +/-2G 
IMU.setAccelRange(MPU9250::ACCEL_RANGE_2G);
// setting the gyroscope full scale range to +/-250 deg/s
IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
// setting DLPF bandwidth to 5 Hz
IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_5HZ);
// setting SRD to 19 for a 50 Hz update rate
IMU.setSrd(19); 
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
// Read values from IMU and GPS sensors
IMU.readSensor();

// Fill variables with sensor values
// IMU Values
accelX = IMU.getAccelX_mss();
accelY = IMU.getAccelY_mss();
accelZ = IMU.getAccelZ_mss();
gyroX = IMU.getGyroX_rads();
gyroY = IMU.getGyroY_rads();
gyroZ = IMU.getGyroZ_rads();
magX = IMU.getMagX_uT();
magY = IMU.getMagY_uT();
magZ = IMU.getMagZ_uT();
// // GPS Values
// Latitude = gps.location.lat();
// Longitude = gps.location.lng();
// roverSpeed = gps.speed.mps(); 

// Calculate roll, pitch, and yaw
roll = atan2(-accelX,(sqrt((accelY * accelY) + (accelZ * accelZ))));
pitch = atan2 (accelY,(sqrt ((accelX * accelX) + (accelZ * accelZ))));
Yh = (magY * cos(roll)) - (magZ * sin(roll));
Xh = (magX * cos(pitch))+(magY * sin(roll)*sin(pitch)) + (magZ * cos(roll) * sin(pitch));
yaw =  atan2(Yh, Xh);

// Convert from Radians to Degrees
roll *= 180/(PI);
pitch *= 180/(PI);
yaw *= 180/(PI);

// Alter range to be 0 to 360 instead of -180 to 180
yaw += 180;
Serial.println(yaw);
delay(200);
}
