/*
Data.h

The purpose of this file is a library for any function 
related to data such as GPS values and Roll/Pitch/Yaw.

*/

#ifndef Data_h
#define Data_h

// Define VSENSE and ASENSE Input pins
#define VSENSE_SYSTEM A0
#define ASENSE_SYSTEM A1
#define VSENSE_PANEL A2
#define ASENSE_PANEL A3

// IMU and GPS libraries
#include "MPU9250.h"
#include "TinyGPSPlus.h"
#include "SoftwareSerial.h"

// The TinyGPSPlus object
TinyGPSPlus gps;
// Pin variables for GPS
static const int RXPin = 17, TXPin = 16;
// Baud rate for GPS
static const uint32_t GPSBaud = 9600;
// The serial connection to the GPS device
SoftwareSerial ss(TXPin, RXPin);
// GPS variables
float Latitude, Longitude;
float roverSpeed;

// Setup IMU connection
MPU9250 IMU(Wire,0x68);
// IMU variables
int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float pitch, roll, yaw;
float Yh, Xh;
float compassHeading;
int cardinalDirection;

// Resistor Values of VSENSE module
float R1 = 30000.0;
float R2 = 7500.0; 
// Float for Reference Voltage (Arduino GPIO voltage)
float ref_voltage = 5;
// Adc_value variable
int Vadc_value = 0;
int Aadc_value = 0;
float adc_voltage = 0;
float adc_current = 0;
// Vsense and Asense variables:
float panelVoltage = 0, panelCurrent = 0;
float AVGpanelVoltage = 0, AVGpanelCurrent = 0;
float systemVoltage = 0, systemCurrent = 0;
float AVGsystemVoltage = 0, AVGsystemCurrent = 0;
float panelPower = 0, systemPower = 0;
float voltagebias = 0;

// SOC variables
int SOC;

// Misc variables
int SensorIterator = 0;

void initData() {
  // Initialize IMU
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

  // Initialize GPS
  ss.begin(GPSBaud);  
}

// Function to check our object detection sensors as well as the tensorflow model
int checkObjectDetection() {
  // If object in front 
    // Check what it is using the tensorflow model
    // If it isn't avoidable or is actually something blocking us
    // return -1
  // Else
    // Check to make sure there actually isn't anything instead of solely relying on the ultrasonic sensors using the tensorflow model
    // if it is something and we cant avoid it
      // return -1
    // else
      // return 0
}

// Function to return SOC of battery as an int (accuracy of about +-5%)
// SOC IS BASED ON THE BATTERY SPECS. IF THIS IS UNKNOWN TO YOU, PLEASE REFERENCE THE OPEN CIRCUIT VOLTAGE VS SOC% CHART FOR THE BATTERY
int checkSOC() {
  AVGsystemVoltage = 0;
  // Use 100 Iterations to try to make the SOC as accurate as possible
  for (SensorIterator = 0; SensorIterator < 100; ++SensorIterator) {
    Vadc_value = analogRead(VSENSE_SYSTEM);

    // Determine voltage at ADC input
    adc_voltage = (Vadc_value * ref_voltage) / 1023.0;

    systemVoltage = adc_voltage / (R2/(R1+R2));

    // Use the iteration to get an average voltage at the end
    AVGsystemVoltage += systemVoltage;
  }
  AVGsystemVoltage = AVGsystemVoltage / SensorIterator;

  if (AVGsystemVoltage > 13.36) {
    SOC = 100;
  }
  else if (AVGsystemVoltage > 13.35 + voltagebias) {
    SOC = 95;
  }
  else if (AVGsystemVoltage > 13.34 + voltagebias) {
    SOC = 90;
  }
  else if (AVGsystemVoltage > 13.32 + voltagebias) {
    SOC = 85;
  }
  else if (AVGsystemVoltage > 13.3 + voltagebias) {
    SOC = 80;
  }
  else if (AVGsystemVoltage > 13.28 + voltagebias) {
    SOC = 75;
  }
  else if (AVGsystemVoltage > 13.26 + voltagebias) {
    SOC = 70;
  }
  else if (AVGsystemVoltage > 13.24 + voltagebias) {
    SOC = 65;
  }
  else if (AVGsystemVoltage > 13.22 + voltagebias) {
    SOC = 60;
  }
  else if (AVGsystemVoltage > 13.2 + voltagebias) {
    SOC = 55;
  }
  else if (AVGsystemVoltage > 13.18 + voltagebias) {
    SOC = 50;
  }
  else if (AVGsystemVoltage > 13.14 + voltagebias) {
    SOC = 45;
  }
  else if (AVGsystemVoltage > 13.1 + voltagebias) {
    SOC = 40;
  }
  // Currently seeing 12.25
  else if (AVGsystemVoltage > 13.05 + voltagebias) {
    SOC = 35;
  }
  else if (AVGsystemVoltage > 13 + voltagebias) {
    SOC = 30;
  }
  else if (AVGsystemVoltage > 12.97 + voltagebias) {
    SOC = 25;
  }
  else if (AVGsystemVoltage > 12.87 + voltagebias) {
    SOC = 20;
  }
  else if (AVGsystemVoltage > 12.75 + voltagebias) {
    SOC = 15;
  }
  else if (AVGsystemVoltage > 12.6 + voltagebias) {
    SOC = 10;
  }

  return SOC;
}

// Function will update IMU and GPS values
void updateIMUValues() {
  if (status >= 0){
    // Read values from IMU and GPS sensors
    IMU.readSensor();
    
    // Fill variables with sensor values
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
    compassHeading = yaw;
  }
  // If the IMU didn't begin then retry (Need to integrate this with the GPS as well)
  else {
    //Serial.println("The IMU device is not enabled retrying connection...");
    status = IMU.begin();

    // setting the accelerometer full scale range to +/-2G 
    IMU.setAccelRange(MPU9250::ACCEL_RANGE_2G);
    // setting the gyroscope full scale range to +/-250 deg/s
    IMU.setGyroRange(MPU9250::GYRO_RANGE_250DPS);
    // setting DLPF bandwidth to 5 Hz
    IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_92HZ);
    // setting SRD to 19 for a 50 Hz update rate
    IMU.setSrd(19);
    //Serial.print("IMU connected, retry command.")
  }
}

// Function will update GPS values
void updateGPSValues() {
  gps.encode(ss.read());

  // GPS Values
  Latitude = gps.location.lat();
  Longitude = gps.location.lng();
  roverSpeed = gps.speed.mps(); 
}

// Function will update the Voltage/Current/Power of the Solar Panel
void updatePanelVA() {
  // Read the Analog Input
  AVGpanelVoltage = 0;
  AVGpanelCurrent = 0;
  for (SensorIterator = 0; SensorIterator < 100; ++SensorIterator) {
    Vadc_value = analogRead(VSENSE_PANEL);
    Aadc_value = analogRead(ASENSE_PANEL);
   
    // Determine voltage at ADC input
    adc_voltage  = (Vadc_value * ref_voltage) / 1023.0; 
    adc_current = (Aadc_value * ref_voltage) / 1023.0;

    panelVoltage = adc_voltage / (R2/(R1+R2));

    // Use the iteration to get an average voltage at the end
    AVGpanelVoltage += panelVoltage;
    AVGpanelCurrent += adc_current;
  }

  AVGpanelVoltage = AVGpanelVoltage / SensorIterator;
  AVGpanelCurrent = ((AVGpanelCurrent / SensorIterator) - (ref_voltage / 2)) / 0.066;
  AVGpanelCurrent = abs(AVGpanelCurrent);
  panelPower = AVGpanelVoltage * AVGpanelCurrent;
}

// Function will update the Voltage/Current/Power of the System
void updateSystemVA() {
  AVGsystemVoltage = 0;
  AVGsystemCurrent = 0;
  // Read the Analog Input
  for (SensorIterator = 0; SensorIterator < 10; ++SensorIterator) {
    Vadc_value = analogRead(VSENSE_SYSTEM);
    Aadc_value = analogRead(ASENSE_SYSTEM);

    // Determine voltage at ADC input
    adc_voltage = (Vadc_value * ref_voltage) / 1023.0;
    adc_current = (Aadc_value * ref_voltage) / 1023.0;

    systemVoltage = adc_voltage / (R2/(R1+R2));

    // Use the iteration to get an average voltage at the end
    AVGsystemVoltage += systemVoltage;
    AVGsystemCurrent += adc_current;
  }

  AVGsystemVoltage = AVGsystemVoltage / SensorIterator;
  AVGsystemCurrent = ((AVGsystemCurrent / SensorIterator) - (ref_voltage / 2)) / 0.066;
  AVGsystemCurrent = abs(AVGsystemCurrent);
  systemPower = AVGsystemVoltage * AVGsystemCurrent;
}

// Function returns current cardinal direction
float getCardinal() {
  updateIMUValues();
  updateGPSValues();
  return(cardinalDirection);
}

// Function returns compass heading
float getCompassHeading() {
  updateIMUValues();
  return(compassHeading);
}

// Function returns longitude
float getLongitude() {
  updateGPSValues();
  return Longitude;
}

// Function returns latitude
float getLatitude() {
  updateGPSValues();
  return Latitude;
}

// Function to send Long/Lat values
String GPSValues() {
  updateGPSValues();
  // The "~" is to split the data when it's sent back to the RPi
  return("~" + String(Longitude) + "~" + String(Latitude) + "~" + String(roverSpeed));
}

// Function to send IMU values
String IMUValues() {
  updateIMUValues();
  // The "~" is to split the data when it's sent back to the RPi
  return("~" + String(roll) + "~" + String(pitch) + "~" + String(yaw));
}

// Function to return panel Voltage, Current, and Power as a string
String panelVA() {
  updatePanelVA();
  // Return the average voltage/current/power of the system  
  return(String(AVGpanelVoltage) + "~" + String(AVGpanelCurrent) + "~" + String(panelPower));
}

// Function sends Panel Power back as a float
float readPanelPower() {
  updatePanelVA();
  return(panelPower);
}

// Function to return system Voltage, Current, and Power as a string
String systemVA() {
  updateSystemVA();
  // Return the average voltage/current/power of the system  
  return(String(AVGsystemVoltage) + "~" + String(AVGsystemCurrent) + "~" + String(systemPower));
}

#endif
