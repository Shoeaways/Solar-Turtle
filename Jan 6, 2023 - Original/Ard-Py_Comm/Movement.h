/*
Movement.h

The purpose of this file is a library for any function 
related to movement using our DC/Servo Motors.
*/

#ifndef Movement_h
#define Movement_h

#include "Data.h"

//Motor Pins
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

//Servo Pins
const int PanelServo = 12;

//Motor Pin Arrays
const int LeftForward[] = {Motor1Forward, Motor2Forward};
const int LeftReverse[] = {Motor1Reverse, Motor2Reverse};
const int RightForward[] = {Motor3Forward, Motor4Forward};
const int RightReverse[] = {Motor3Reverse, Motor4Reverse};
const int Forward[] = {Motor1Forward, Motor2Forward, Motor3Forward, Motor4Forward};
const int Reverse[] = {Motor1Reverse, Motor2Reverse, Motor3Reverse, Motor4Reverse};
const int PWM[] = {Motor4PWM, Motor3PWM, Motor2PWM, Motor1PWM};

//Variables
//String data;
float PWMinput = 0;
int brakeVar = 0;
int currAngle = 0;
int i = 0;
int j = 0;

//Initialize function
void initMovement() {
  //Default pins 2-15 to output
  for (i = 2; i <=15; ++i) {
    pinMode(i, OUTPUT);
  }
  
  //Default all PWMs to 0
  for (i = 0; i < 4; ++i) {
    analogWrite(PWM[i],0);
  }
  
  //Defaults Servo to ~90 degrees which sits our solar panel flat
  for (j = 140; j <= 180; ++j) {
      analogWrite(PanelServo, j);
      delay(25);
  }
  currAngle = 180;
}

//Send GPS coords/RPY/etc.
void sendData() {
  Serial.print(sendValues() + " " + String(currAngle));
}

//Move forward at a given speed (num is a 0-100 speed input)
void MoveForward(int num) {
  PWMinput = num * 2.55;
  //Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

//Move backward at a given speed (num is a 0-100 speed input)
void MoveReverse(int num) {
  PWMinput = num * 2.55;
  //Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

//Turn Right at a default speed
void TurnRight() {
  //Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

//Turn Left at a default speed
void TurnLeft() {
  //Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

//Moves the solar panel servo to a called angle 
void MovePanel(int angle) {
  //Case where the called angle is greater than the current angle
  if (currAngle < angle) {
    for (i = currAngle; i <= angle; ++i) {
      analogWrite(PanelServo, i);
      delay(25);
    }
  }
  //Case where the called angle is smaller than the current angle
  else if (currAngle > angle) {
    for (i = currAngle; i >= angle; --i) {
      analogWrite(PanelServo, i);
      delay(25);
    }
  }
  //Update the current Angle
  currAngle = angle;
}

//Slow down and stop 
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

#endif
