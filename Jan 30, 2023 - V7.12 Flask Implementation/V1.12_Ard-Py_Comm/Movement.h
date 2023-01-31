/*
Movement.h

The purpose of this file is a library for any function 
related to movement using our DC/Servo Motors.
*/

#ifndef Movement_h
#define Movement_h

#include "Data.h"

// Motor Pins
static const int Motor1PWM = 11, Motor1Forward = 14, Motor1Reverse = 15;
static const int Motor2PWM = 10, Motor2Forward = 9, Motor2Reverse = 8;
static const int Motor3PWM = 5, Motor3Forward = 6, Motor3Reverse = 7;
static const int Motor4PWM = 4, Motor4Forward = 2 ,Motor4Reverse = 3;

// Servo Pins
static const int PanelServo = 12;

// Motor Pin Arrays
static const int LeftForward[] = {Motor1Forward, Motor2Forward};
static const int LeftReverse[] = {Motor1Reverse, Motor2Reverse};
static const int RightForward[] = {Motor3Forward, Motor4Forward};
static const int RightReverse[] = {Motor3Reverse, Motor4Reverse};
static const int Forward[] = {Motor1Forward, Motor2Forward, Motor3Forward, Motor4Forward};
static const int Reverse[] = {Motor1Reverse, Motor2Reverse, Motor3Reverse, Motor4Reverse};
static const int PWM[] = {Motor4PWM, Motor3PWM, Motor2PWM, Motor1PWM};

// Running Function Flags
bool movingFoward = false, movingReverse = false;
int tempForwardPWM = 0, tempReversePWM = 0;

// Turning Variables
bool overflowFlag, rightTurnOverflow, leftTurnOverflow;
bool fromZero;
float targetAngle = 0, lowerTargetAngle = 0, upperTargetAngle = 0;
float angleDifference = 0, currAngle = 0, prevAngle = 0;
float errorAngle = 2.0;
float tempOverflow = 0;
float turnSpeed = 0;

// Misc Variables
float PWMinput = 0;
int brakeVar = 0;
int currPanelAngle = 0;
int i = 0;
int j = 0;

// Initialize function
void initMovement() {
  // Default pins 2-15 to OUTPUT
  for (i = 2; i <=15; ++i) {
    pinMode(i, OUTPUT);
  }
  
  // Default all PWMs to 0
  for (i = 0; i < 4; ++i) {
    analogWrite(PWM[i],0);
  }
  
  // Defaults Servo to ~90 degrees which sits our solar panel flat
  for (i = 90; i >= 100; i -= 3) {
      analogWrite(PanelServo, i);
      delay(100);
  }
  currPanelAngle = 100;
}

// Send GPS coords/RPY/etc.
// The "~" is to split the data when it's sent back to the RPi
void sendData() {
  Serial.print(IMUandGPSValues() + "~" + String(currPanelAngle) + "~" + systemVA() + "~" + panelVA() + "~");
}

// Move forward at a given speed (num is a 0-100 speed input)
void MoveForward(int num) {
  if (num > 100) {
    num = 100;
    PWMinput = 255;
  }
  else {
    PWMinput = num * 2.55;
  }

  // Check if the rover is already moving forward or not
  if (movingFoward == true) {
    // Check if the previous speed is >, <, or = the new speed
    if (tempForwardPWM > PWMinput) {
      for (i = tempForwardPWM; i >= PWMinput; --i) {
        for (j = 0; j < 4; ++j) {
          analogWrite(PWM[j], i);
        }
        delay(8);
      }
    }
    else if (tempForwardPWM < PWMinput) {
      for (i = tempForwardPWM; i <= PWMinput; ++i) {
        for (j = 0; j < 4; ++j) {
          analogWrite(PWM[j], i);
        }
        delay(8);
      }
    }
  }
  // If not, stop the current movement and start moving forward from zero
  else {
    movingReverse = false;
    tempReversePWM = 0; 
    // Sets speed to 0 before starting to account for calling the function while the robot is moving in any direction other than forward
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
    for (i = 0; i <= PWMinput; ++i) {
      for (j = 0; j < 4; ++j) {
        analogWrite(PWM[j], i);
      }
      delay(8);
    }
  }
  tempForwardPWM = i;
  movingFoward = true;
}

// Move backward at a given speed (num is a 0-100 speed input)
void MoveReverse(int num) {
  if (num > 100) {
    num = 100;
    PWMinput = 255;
  }
  else {
    PWMinput = num * 2.55;  
  }

  // Check if the rover is already moving backward or not
  if (movingReverse == true) {
    // Check if the previous speed is >, <, or = the new speed
    if (tempReversePWM > PWMinput) {
      for (i = tempReversePWM; i >= PWMinput; --i) {
        for (j = 0; j < 4; ++j) {
          analogWrite(PWM[j], i);
        }
        delay(8);
      }
    }
    else if (tempReversePWM < PWMinput) {
      for (i = tempReversePWM; i <= PWMinput; ++i) {
        for (j = 0; j < 4; ++j) {
          analogWrite(PWM[j], i);
        }
        delay(8);
      }
    }
  }
  // If not, stop the current movement and start moving forward from zero
  else {
    movingFoward = false;
    tempForwardPWM = 0; 
    // Sets speed to 0 before starting to account for calling the function while the robot is already moving
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
    for (i = 0; i <= PWMinput; ++i) {
      for (j = 0; j < 4; ++j) {
        analogWrite(PWM[j], i);
      }
      delay(8);
    }
  }
  tempForwardPWM = i;
  movingFoward = true;
}

// Turn Right x amount of degrees
void TurnRight(float angle) {
  // Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

  // Flag for the while loop when turning to determine whether or not we are starting our speed at 0
  fromZero = true;
  overflowFlag = false;

  // Pull compass heading from IMU and create a target angle as well as the upper/lower limit with the error
  currAngle = YawValue();
  prevAngle = currAngle;
  targetAngle = currAngle + angle;
  upperTargetAngle = targetAngle + errorAngle;
  lowerTargetAngle = targetAngle - errorAngle;

  // Account for overflow of 360 to 0 degrees
  if (upperTargetAngle >= 360) {
    rightTurnOverflow = true;
  }

  for (i = 0; i < 2; ++i) {
    digitalWrite(LeftForward[i], HIGH);
    digitalWrite(RightReverse[i], HIGH);
  }

  // Target angle overflows past 360 degrees
  if (rightTurnOverflow == true) {
    // Increase error margins when we do have to overflow
    //lowerTargetAngle -= errorAngle;
    //upperTargetAngle += errorAngle;         
    while (((currAngle + 360) < lowerTargetAngle) || ((currAngle + 360) > upperTargetAngle)) {
      // Checks if the IMU overflowed from 360 to 0
      if (abs(prevAngle - currAngle) > 150) {
        overflowFlag = true;
      }
      if (overflowFlag == true) {
        // We can add 360 here since it will be temporary until it is recaptured at the end of the while loop
        currAngle += 360;
        overflowFlag = false;
      }     
      
      // Calculate remaining distance from target angle
      angleDifference = targetAngle - currAngle;
      angleDifference = abs(angleDifference); // This just accounts for overshoot or left turns 
  
      // Set speed based on percent distance remaining
      if (angleDifference/angle >= 1) {
        turnSpeed = 50;
      }
      else {
        turnSpeed = ((angleDifference/angle) * 50);   
      }

      // If the rover is beginning to move, we will start the PWM from i = 0
      if (fromZero) {
        if ((angleDifference/angle) > 1) {
          for (i = 0; i <= 50; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        else {
          for (i = 0; i <= turnSpeed; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        fromZero = false;
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], i);
          }
          delay(2);
        }
      }
      // Update variables and recapture the current compass reading
      prevAngle = currAngle;
      brakeVar = turnSpeed;
      currAngle = YawValue();
    }
  }
  // No overflow
  else {
    while((currAngle < lowerTargetAngle) || (currAngle > upperTargetAngle)) {
      // Calculate remaining distance from target angle
      angleDifference = targetAngle - currAngle;
      angleDifference = abs(angleDifference); // This just accounts for overshoot or left turns 
  
      // Set speed based on percent distance remaining      
      if (angleDifference/angle >= 1) {
        turnSpeed = 50;
      }
      else {
        turnSpeed = ((angleDifference/angle) * 50);   
      }
  
      // If the rover is beginning to move, we will start the PWM from i = 0
      if (fromZero) {
        if ((angleDifference/angle) >= 1) {
          for (i = 0; i <= 50; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        else {
          for (i = 0; i <= turnSpeed; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        fromZero = false;
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], turnSpeed);
          }
          delay(2);
        }
      }
      // Update variables and recapture the current compass reading
      brakeVar = turnSpeed;
      currAngle = YawValue();
    }
  }

  for (i = 0; i < 2; ++i) {
    digitalWrite(LeftForward[i], LOW);
    digitalWrite(RightReverse[i], LOW);
  }
}

// Turn Left at a default speed
void TurnLeft(float angle) {
  // Sets speed to 0 before starting to account for calling the function while the robot is already moving
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

  // Flag for the while loop when turning to determine whether or not we are starting our speed at 0
  fromZero = true;
  overflowFlag = false;

  // Pull compass heading from IMU and create a target angle as well as the upper/lower limit with the error  
  currAngle = YawValue();  
  prevAngle = currAngle;
  targetAngle = currAngle - angle;
  lowerTargetAngle = targetAngle - errorAngle;
  upperTargetAngle = targetAngle + errorAngle;

  // Account for overflow of 0 to 360 degrees
  if (lowerTargetAngle <= 0) {
    leftTurnOverflow = true;
  }  

  for (i = 0; i < 2; ++i) {
    digitalWrite(RightForward[i], HIGH);
    digitalWrite(LeftReverse[i], HIGH);
  }
  
  // Target angle overflows past 0 degrees
  if (leftTurnOverflow == true) {
    // Increase error margins when we do have to overflow
    //lowerTargetAngle -= errorAngle;
    //upperTargetAngle += errorAngle; 
    while ((currAngle < (lowerTargetAngle + 360)) || (currAngle > (upperTargetAngle + 360))) {
      // Checks if the IMU overflowed from 360 to 0
      if (abs(prevAngle - currAngle) > 150) {
        overflowFlag = true;
      }
      if (overflowFlag == true) {
        // We can add 360 here since it will be temporary until it is recaptured at the end of the while loop
        currAngle += 360;
        overflowFlag = false;
      }

      // Calculate remaining distance from target angle
      angleDifference = targetAngle - currAngle;
      angleDifference = abs(angleDifference); // This just accounts for overshoot or left turns
      
      // Set speed based on percent distance remaining
      if (angleDifference/angle >= 1) {
        turnSpeed = 50;
      }
      else {
        turnSpeed = ((angleDifference/angle) * 50);   
      }
                  
      // If the rover is beginning to move, we will start the PWM from i = 0
      if (fromZero) {
        if ((angleDifference/angle) > 1) {
          for (i = 0; i <= 50; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        else {
          for (i = 0; i <= turnSpeed; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        fromZero = false;
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], i);
          }
          delay(2);
        }
      }
      // Update variables and recapture the current compass reading
      prevAngle = currAngle;
      brakeVar = turnSpeed;
      currAngle = YawValue();        
    }
  }
  // No overflow
  else {
    while ((currAngle < lowerTargetAngle) || (currAngle > upperTargetAngle)) {
       // Calculate remaining distance from target angle
      angleDifference = targetAngle - currAngle;
      angleDifference = abs(angleDifference); // This just accounts for overshoot or left turns 
  
      // Set speed based on percent distance remaining      
      if (angleDifference/angle >= 1) {
        turnSpeed = 50;
      }
      else {
        turnSpeed = ((angleDifference/angle) * 50);   
      }
  
      // If the rover is beginning to move, we will start the PWM from i = 0
      if (fromZero) {
        if ((angleDifference/angle) >= 1) {
          for (i = 0; i <= 50; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        else {
          for (i = 0; i <= turnSpeed; ++i) {
            for (j = 0; j < 4; ++j) {
              analogWrite(PWM[j], i);
            }
            delay(6);
          }
        }
        fromZero = false;
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], turnSpeed);
          }
          delay(2);
        }
      }
      // Update variables and recapture the current compass reading
      brakeVar = turnSpeed;
      currAngle = YawValue();  
    }    
  }
  
  for (i = 0; i < 2; ++i) {
    digitalWrite(RightForward[i], LOW);
    digitalWrite(LeftReverse[i], LOW);
  }
}

// Moves the solar panel servo to a called angle 
void MovePanel(int angle) {
  // Determine if called angle is out of the reachable bounds
  // Set to 125 if the angle called is greater than 125
  if (angle > 125) {
    angle = 125;
  }
  // Set to 80 if the angle called is less than 80
  if (angle < 80) {
    angle = 80;
  }

  // Case where the called angle is greater than the current angle
  if (currPanelAngle < angle) {
    for (i = currAngle; i <= angle; i += 3) {
      analogWrite(PanelServo, i);
      delay(100);
    }
  }
  // Case where the called angle is smaller than the current angle
  else if (currPanelAngle > angle) {
    for (i = currPanelAngle; i >= angle; i -= 3) {
      analogWrite(PanelServo, i);
      delay(100);
    }
  }
  // Update the current Angle
  currPanelAngle = angle;
}

// Slow down and stop 
void Stop(int num) { 
  for (brakeVar = i; brakeVar >= 0; --brakeVar) {
    for (j = 0; j < 4; ++j) {
      analogWrite(PWM[j], brakeVar);
    }
    delay(12/num);
  }
  
  for (i = 0; i < 4; ++i) {
    digitalWrite(Forward[i], LOW);
    digitalWrite(Reverse[i], LOW);
  }
}

// Change the error margin of the IMU with the new input one
void changeErrorMargin (float newErrorMargin) {
  errorAngle = newErrorMargin;  
}

#endif
