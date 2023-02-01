#include "MPU9250.h"

MPU9250 IMU(Wire,0x68);

int status;
float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float magX, magY, magZ;
float pitch, roll, yaw;
float Yh, Xh;

const int Motor1PWM = 11;
const int Motor1Forward = 14;
const int Motor1Reverse = 15;
const int Motor2PWM = 10;
const int Motor2Forward = 9;
const int Motor2Reverse = 8;
const int Motor3PWM = 5;
const int Motor3Forward = 6;
const int Motor3Reverse = 7;
const int Motor4PWM = 4;
const int Motor4Forward = 2;
const int Motor4Reverse = 3;

const int PanelServo = 12;

const int LeftForward[] = {Motor1Forward, Motor2Forward};
const int LeftReverse[] = {Motor1Reverse, Motor2Reverse};
const int RightForward[] = {Motor3Forward, Motor4Forward};
const int RightReverse[] = {Motor3Reverse, Motor4Reverse};
const int Forward[] = {Motor1Forward, Motor2Forward, Motor3Forward, Motor4Forward};
const int Reverse[] = {Motor1Reverse, Motor2Reverse, Motor3Reverse, Motor4Reverse};
const int PWM[] = {Motor4PWM, Motor3PWM, Motor2PWM, Motor1PWM};
float PWMinput = 0;
int brakeVar = 0;
int currAngle = 0;
int i = 0;
int j = 0;

String nom = "Solar Turtle is ";
String message;
String msg;
String tempString; 
int num;

void setup() {
  Serial.begin(115200);
  
  for (i = 2; i <=15; ++i) {
    pinMode(i, OUTPUT);
  }
  for (i = 0; i < 4; ++i) {
    analogWrite(PWM[i],0);
  }
  
  status = IMU.begin();
  if (status < 0) {
    Serial.println("IMU initialization unsuccessful");
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
  }
  
  for (j = 140; j <= 180; ++j) {
      analogWrite(PanelServo, j);
      delay(25);
  }
  currAngle = 180;
}

void loop() {
  readSerialPort();

  if (message == "data") {
    Serial.print(nom);
    sendData();
  } 
  else {
    message.toLowerCase();  
      
    if (message == "stop") {
      Serial.print(nom + "Stopping...");
      Stop();
    }
    
    tempString = message.substring(0, 3);
    num = tempString.toInt();
    msg = message.substring(4, message.length());
    
    if (msg == "forward") {
      Serial.print(nom + "Schmooving Forward ");
      MoveForward(num);    
    } 
    else if (msg == "reverse") {
      Serial.print(nom +"Reverse Reverse ");
      MoveReverse(num);
    }
    else if (msg == "left") {
      Serial.print(nom + "Turning to the Left ");
      TurnLeft();
    }
    else if (msg == "right") {
      Serial.print(nom + "Turning to the Left ");
      TurnRight();
    }
    else if (msg == "panel") {
      Serial.print("Schmooving the Panel ");
      MovePanel(num);
    }

    num = 0;
    tempString = "";
    msg = "";
  }
  delay(500);
}

void readSerialPort() {
 message = "";
  if (Serial.available()) {
    delay(10);
    while (Serial.available() > 0) {
      message += (char)Serial.read();
    }
    Serial.flush();
  }
}

void sendData() {
  if (status >= 0){
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

    pitch = atan2 (accelY,(sqrt ((accelX * accelX) + (accelZ * accelZ))));
    roll = atan2(-accelX,(sqrt((accelY * accelY) + (accelZ * accelZ))));

    Yh = (magY * cos(roll)) - (magZ * sin(roll));
    Xh = (magX * cos(pitch))+(magY * sin(roll)*sin(pitch)) + (magZ * cos(roll) * sin(pitch));
  
    yaw =  atan2(Yh, Xh);

    // Convert from Radians to Degrees
    roll *= 180/(PI);
    pitch *= 180/(PI);
    yaw *= 180/(PI);
  
    // Alter range to be 0 to 360 instead of -180 to 180
    yaw += 180;
  
    Serial.print("pitch: "  + String(pitch) + " roll: " + String(roll) + " yaw: " + String(yaw));
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

void MoveForward(int num) {
  PWMinput = num * 2.55;
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(8);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], HIGH);
    digitalWrite(Reverse[i], LOW);
  }

  for (i = 0; i < PWMinput; ++i) { //nice
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], i);
    }
    delay(8);
  }
}

void MoveReverse(int num) {
  PWMinput = num * 2.55;
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(8);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], HIGH);
  }

  for (i = 0; i < PWMinput; ++i) { //nice
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], i);
    }
    delay(8);
  }
}

void TurnRight() {
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(8);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
  
  for (i = 0; i < 2; ++i) {
    digitalWrite(LeftForward[i], HIGH);
    digitalWrite(RightReverse[i], HIGH);
  }
  
  for (i = 0; i <= 150; ++i) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], i);
    }
    delay(8);
  }
}

void TurnLeft() {
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(8);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
  
  for (i = 0; i < 2; ++i) {
    digitalWrite(RightForward[i], HIGH);
    digitalWrite(LeftReverse[i], HIGH);
  }
  
  for (i = 0; i <= 150; ++i) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], i);
    }
    delay(8);
  }
}

void MovePanel(int angle) {
  if (currAngle < angle) {
    for (i = currAngle; i <= angle; ++i) {
      analogWrite(PanelServo, i);
      delay(25);
    }
  }
  else if (currAngle > angle) {
    for (i = currAngle; i >= angle; --i) {
      analogWrite(PanelServo, i);
      delay(25);
    }
  }
  currAngle = angle;
}

void Stop() { 
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(8);
  }

  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
}
