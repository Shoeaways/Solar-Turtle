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
#include "MPU6050_tockn.h"
#include "TinyGPSPlus.h"

// Ultrasonic Sensor Pins & Variables
static const int FLsensorIN = 22, FLsensorOUT = 23;
static const int FRsensorIN = 24, FRsensorOUT = 25;
static const int BLsensorIN = 26, BLsensorOUT = 27;
static const int BRsensorIN = 28, BRsensorOUT = 29;
float Leftduration, Rightduration;
int Leftdistance, Rightdistance;

// The TinyGPSPlus object
TinyGPSPlus gps;
// Pin variables for GPS
static const int RXPin = 19, TXPin = 18;
// Baud rate for GPS
static const uint32_t GPSBaud = 9600;
// The serial connection to the GPS device
SoftwareSerial ss(TXPin, RXPin);
// GPS variables
float Latitude, Longitude;
float roverSpeed;

// Setup IMU connection
MPU6050 mpu6050(Wire);
//MPU9250 IMU(Wire,0x68);
// IMU variables
int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float pitch = 0, roll = 0, yaw = 0;
float Yh, Xh;
float compassHeading;
int cardinalDirection;

// Resistor Values of VSENSE module
float R1 = 30000.0;
float R2 = 7500.0; 
// Float for Reference Voltage (Arduino GPIO voltage)
float ref_voltage = 5.057;
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
float norm;
int SensorIterator = 0;
int dataPinIterator = 0;
int ObjectDetectionIterator = 0;

void initData() {
  for (dataPinIterator = 22; dataPinIterator <= 28; dataPinIterator += 2) {
    pinMode(dataPinIterator, INPUT);
    pinMode(dataPinIterator + 1, OUTPUT);
    digitalWrite(dataPinIterator, LOW);
  }

  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
  
  /*
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
  */

  // Initialize GPS
  Serial1.begin(9600);
}

// Function to check our object detection sensors as well as the tensorflow model for the front of the rover
int checkObjectDetectionFront() {
  // Sample the ultrasonic sensors 30 times (Left then Right)
  for (ObjectDetectionIterator = 0; ObjectDetectionIterator < 30; ++ObjectDetectionIterator) {
    digitalWrite(FLsensorIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(FLsensorIN, LOW);
    Leftduration = pulseIn(FLsensorOUT, HIGH);
    Leftdistance += (Leftduration * 0.034 / 2);
  }
  Leftdistance /= 30;

  for (ObjectDetectionIterator = 0; ObjectDetectionIterator < 30; ++ObjectDetectionIterator) {
    digitalWrite(FRsensorIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(FRsensorIN, LOW);
    Rightduration = pulseIn(FRsensorOUT, HIGH);
    Rightdistance += (Rightduration * 0.034 / 2);
  }
  Rightdistance /= 30;

  // If an object is between 30-200cm away, return that there is an object in front
  if ((Leftdistance < 200 && Leftdistance > 30) || (Rightdistance < 200 && Rightdistance > 30)) {
    // Check tensorflow
    // If tensorflow identifies it isn't avoidable or it is actually something blocking us
    return -1;
    // else it's likely something we can roll over or it actually wasn't anything
  }
  else {
    // Check tensorflow
    // If tensorflow identifies it isn't avoidable or it is actually something blocking us
    // return -1;
    // else it's likely something we can roll over or it actually wasn't anything
    return 0;
  }
}

// Function to check our object detection sensors for the back of the rover
int checkObjectDetectionBack() {
  // Sample the ultrasonic sensors 30 times
  for (ObjectDetectionIterator = 0; ObjectDetectionIterator < 30; ++ObjectDetectionIterator) {
    digitalWrite(BLsensorIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(BLsensorIN, LOW);
    Leftduration = pulseIn(BLsensorOUT, HIGH);
    Leftdistance += (Leftduration * 0.034 / 2);
  }
  Leftdistance /= 30;

  for (ObjectDetectionIterator = 0; ObjectDetectionIterator < 30; ++ObjectDetectionIterator) {
    digitalWrite(BRsensorIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(BRsensorIN, LOW);
    Rightduration = pulseIn(BRsensorOUT, HIGH);
    Rightdistance += (Rightduration * 0.034 / 2);
  }
  Rightdistance /= 30;

  // If an object is between 30-200cm away, return that there is an object in front
  if ((Leftdistance < 200 && Leftdistance > 30) || (Rightdistance < 200 && Rightdistance > 30)) {
    return -1;
  }
  else {
    return 0;
  }
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
  mpu6050.update();
  compassHeading = mpu6050.getAngleZ();
}

// Function will update GPS values
void updateGPSValues() {
  gps.encode(Serial1.read());

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
float getCompassHeading(bool &newTurnFlag) {
  if (newTurnFlag == true) {
    compassHeading = 0;
    newTurnFlag = false;
  }
  mpu6050.update();
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
  mpu6050.update();
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
