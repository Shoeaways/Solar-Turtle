/*
Data.h

The purpose of this file is a library for any function 
related to data such as GPS values and Roll/Pitch/Yaw.
*/

// Need to Implement code for Voltage and Current Sensors in Data.h file

#ifndef Data_h
#define Data_h

#include "MPU9250.h"

// Setup IMU connection
MPU9250 IMU(Wire,0x68);

int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float pitch, roll, yaw;
float Yh, Xh;

void initData() {
  status = IMU.begin();
  if (status < 0) {
    // Serial.println("IMU initialization unsuccessful");
    // Serial.println("Check IMU wiring or try cycling power");
    // Serial.print("Status: ");
    // Serial.println(status);
    // while(1) {}
  }
  // setting the accelerometer full scale range to +/-2G 
  IMU.setAccelRange(MPU9250::ACCEL_RANGE_2G);
  // setting the gyroscope full scale range to +/-250 deg/s
  IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
  // setting DLPF bandwidth to 5 Hz
  IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_5HZ);
  // setting SRD to 19 for a 50 Hz update rate
  IMU.setSrd(19);
}

void updateValues() {
  if (status >= 0){
    // Read values from sensors
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
   }
  else {
    //Serial.println("The IMU device is not enabled retrying connection...");
    status = IMU.begin();
    if (status < 0) {
      //Serial.println("IMU initialization unsuccessful...");
    }
    else {
      // setting the accelerometer full scale range to +/-2G 
      IMU.setAccelRange(MPU9250::ACCEL_RANGE_2G);
      // setting the gyroscope full scale range to +/-250 deg/s
      IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
      // setting DLPF bandwidth to 5 Hz
      IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_5HZ);
      // setting SRD to 19 for a 50 Hz update rate
      IMU.setSrd(19);
      //Serial.print("IMU connected, retry command.")
    }
  }
}

// Function to send Roll/Pitch/Yaw values
String IMUValues() {
  updateValues();
  // The "~" is to split the data when it's sent back to the RPi
  // Check if Long/Lat
  return("~" + String(roll) + "~" + String(pitch) + "~" + String(yaw) + "~");
}

// Function to send just the Yaw value (compass heading)
float YawValue() {
  updateValues();
  return(yaw);
}

#endif
