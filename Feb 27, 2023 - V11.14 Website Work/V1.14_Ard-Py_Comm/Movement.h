/*
Movement.h

The purpose of this file is a library for any function 
related to movement using our DC/Servo Motors.

*/

#ifndef Movement_h
#define Movement_h

#include "Data.h"

// Motor Pins
static const int Motor1PWM = 11, Motor1Forward = 15, Motor1Reverse = 14;
static const int Motor2PWM = 10, Motor2Forward = 8, Motor2Reverse = 9;
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

// Sleep Pin Arrays
int InitSleepArray[] = {13, 18, 19};
int FunctionSleepArray[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15};

// Movement Function Flags
bool movingForward = false, movingReverse = false;
int tempForwardPWM = 0, tempReversePWM = 0;

// Sleep Flag
bool isSleeping;

// Autonomous Solar Panel Variables
int currPanelAngle = 0;
float optimalPower = 0;
float optimalAngle = 0;
float readPower = 0;

// MoveTo Function Variables
double LKSLongitude, LKSLatitude; // Last Known Signal Long/Lat
double endTargetX = 0, endTargetY = 0;
double subTargetX = 0, subTargetY = 0;
int currentCardinal = 1;// Allows us to denote N/E/S/W with the midpoints (NE,SE) using 1-8
int targetCardinal[128];
int North = 1, NorthEast = 2, East = 3, SouthEast = 4, South = 5, SouthWest = 6, West = 7, NorthWest = 8;
bool mapExists = true;

// A* Variables
double mapLong[128];
double mapLat[128];
double heuristicVal[128];
double fastestLong[128];
double fastestLat[128];
int beenTo[128];
int checkArray[8];
int totalPoints;
int currentIndex;
int bestChoice;
bool fastestMapComplete = false;

// Turning Variables
bool overflowFlag, rightTurnOverflow, leftTurnOverflow;
bool fromZero;
bool newTurn = false;
float targetAngle = 0, lowerTargetAngle = 0, upperTargetAngle = 0;
float angleDifference = 0, currAngle = 0, prevAngle = 0;
float errorAngle = 2.0;
float tempOverflow = 0;
float turnSpeed = 0;
int turnIterator;

// Misc Variables
float PWMinput = 0;
int brakeVar = 0;
int i = 0;
int j = 0;

// Initialize function
void initMovement() {
  isSleeping = false; 

  // Default pins 2-15 to OUTPUT
  for (i = 2; i <=15; ++i) {
    pinMode(i, OUTPUT);
  }
  for (i = 0; i <3; ++i) {
    pinMode(InitSleepArray[i], OUTPUT);
    digitalWrite(InitSleepArray[i], LOW);
  }
  for (i = 30; i < 54; ++i) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  
  // Default all PWMs to 0
  for (i = 0; i < 4; ++i) {
    analogWrite(PWM[i],0);
  }
  
  // Defaults Servo to ~90 degrees which sits our solar panel flat
  for (i = 65; i < 70; i += 1) {
      analogWrite(PanelServo, i);
      delay(50);
  }
  currPanelAngle = i;

  // Initialize Last Known Signal Long/Lat
  LKSLongitude = 0;
  LKSLatitude = 0;
}

// Autonomous solar panel movement (Checks every 5 degrees between 60-120 and locates the best charging angle)
void AutonomousSolarPanel() {
  // Reset panel to 30 degrees
  for (i = currPanelAngle; i > 30; i -= 1) {
    analogWrite(PanelServo, i);
    delay(40);
  }
  currPanelAngle = 30;
  
  optimalPower = readPanelPower();
  optimalAngle = currPanelAngle;
  // Poll angles between 30-105 at 5 degree increments for the most optimal charging rate
  for (i = currPanelAngle; i < 105; i += 5) {
    analogWrite(PanelServo, i);
    // fiddle w this number
    delay(100);
    readPower = readPanelPower();
    if (readPower > optimalPower) {
      optimalAngle = i;
      optimalPower = readPower;
    }
  }
  currPanelAngle = i;

  // Move panel to recorded optimal angle
  for (i = currPanelAngle; i > optimalAngle; i -= 1) {
    analogWrite(PanelServo, i);
    delay(40);
  }
  currPanelAngle = i;
}

// Change the error margin of the IMU with the new input one
void changeErrorMargin (float newErrorMargin) {
  errorAngle = newErrorMargin;  
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
  if (movingForward == true) {
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
  movingForward = true;
}

// Moves the solar panel servo to a called angle 
void MovePanel(int angle) {
  // Determine if called angle is out of the reachable bounds
  // Set to 105 if the angle called is greater than 105
  if (angle > 105) {
    angle = 105;
  }
  // Set to 30 if the angle called is less than 30
  if (angle < 30) {
    angle = 30;
  }

  // Case where the called angle is greater than the current angle
  if (currPanelAngle < angle) {
    for (i = currPanelAngle; i < angle; i += 1) {
      analogWrite(PanelServo, i);
      delay(50);
    }
  }
  // Case where the called angle is smaller than the current angle
  else if (currPanelAngle > angle) {
    for (i = currPanelAngle; i > angle; i -= 1) {
      analogWrite(PanelServo, i);
      delay(50);
    }
  }
  // Update the current Angle
  currPanelAngle = angle;
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
    movingForward = false;
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
  tempReversePWM = i;
  movingReverse = true;
}

// Send GPS coords/RPY/etc. The "~" is to split the data when it's sent back to the RPi
void sendData() {
  Serial.print(IMUValues() + GPSValues() + "~" + String(currPanelAngle) + "~" + systemVA() + "~" + panelVA() + "~" + String(checkSOC()) + "~");
}

// Slow down and stop 
void Stop(int num) { 
  movingForward = false;
  movingReverse = false;
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

// Turn Right x amount of degrees
void TurnRight(float angle) {
  Serial.println("Turning Right");
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
  movingForward = false;
  movingReverse = false;

  // Flag for the while loop when turning to determine whether or not we are starting our speed at 0
  fromZero = true;
  overflowFlag = false;

  // Pull compass heading from IMU and create a target angle as well as the upper/lower limit with the error
  newTurn = true;
  currAngle = getCompassHeading(newTurn);
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
      // Serial.println(currAngle);
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
        turnSpeed = ((angleDifference/angle) * 50) + 25;   
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
        currAngle = getCompassHeading(newTurn);
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], i);
          }
          delay(2);
        }
        currAngle = getCompassHeading(newTurn);
      }
      // Update variables and recapture the current compass reading
      prevAngle = currAngle;
      brakeVar = turnSpeed;
      //currAngle = getCompassHeading(newTurn);
    }
  }
  // No overflow
  else {
    while((currAngle < lowerTargetAngle) || (currAngle > upperTargetAngle)) {
      //Serial.println(currAngle);
      // Calculate remaining distance from target angle
      angleDifference = targetAngle - currAngle;
      angleDifference = abs(angleDifference); // This just accounts for overshoot or left turns 
  
      // Set speed based on percent distance remaining      
      if (angleDifference/angle >= 1) {
        turnSpeed = 50;
      }
      else {
        turnSpeed = ((angleDifference/angle) * 50) + 25;   
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
        currAngle = getCompassHeading(newTurn);
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], turnSpeed);
          }
          delay(2);
        }
        currAngle = getCompassHeading(newTurn);
      }
      // Update variables and recapture the current compass reading
      brakeVar = turnSpeed;
      //currAngle = getCompassHeading(newTurn);
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
  movingForward = false;
  movingReverse = false;

  // Flag for the while loop when turning to determine whether or not we are starting our speed at 0
  fromZero = true;
  overflowFlag = false;

  // Pull compass heading from IMU and create a target angle as well as the upper/lower limit with the error  
  newTurn = true;
  currAngle = getCompassHeading(newTurn);  
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
        turnSpeed = ((angleDifference/angle) * 50) + 25;   
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
        currAngle = getCompassHeading(newTurn);
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], i);
          }
          delay(2);
        }
        currAngle = getCompassHeading(newTurn);
      }
      // Update variables and recapture the current compass reading
      prevAngle = currAngle;
      brakeVar = turnSpeed;
      //currAngle = getCompassHeading(newTurn);        
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
        turnSpeed = ((angleDifference/angle) * 50) + 25;   
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
        currAngle = getCompassHeading(newTurn);
      }
      // Otherwise, we will slow down from the previous speed
      else {
        for (i = brakeVar; i > turnSpeed; --i) {
          for (j = 0; j < 4; ++j) {
            analogWrite(PWM[j], turnSpeed);
          }
          delay(2);
        }
        currAngle = getCompassHeading(newTurn);
      }
      // Update variables and recapture the current compass reading
      brakeVar = turnSpeed;
      //currAngle = getCompassHeading(newTurn);  
    }    
  }
  
  for (i = 0; i < 2; ++i) {
    digitalWrite(RightForward[i], LOW);
    digitalWrite(LeftReverse[i], LOW);
  }
}

// Function enters sleep mode, pausing all movement functions
void enterSleep() {
  Stop(2);
  // Set Motor Pins to low
  for (i = 0; i < 12; ++i) {
    digitalWrite(FunctionSleepArray[i], LOW);
  }
}

// Function exits sleep mode, continuining any previous functions
void exitSleep() {
  // Reset all used pins back to their previous setting
  for (i = 0; i < 12; ++i) {
    digitalWrite(FunctionSleepArray[i], HIGH);
  }
}

// NOTES FOR CREATING THIS FUNCTION:
// Length of 1° latitude ~= 87,870m
// Length of 1° longitude ~= 111,045m
// Determine what direction it is so the rover begins in the right direction
// Populate map function to target (For A*)
// Need to make it so that beenTo = 1 just increases the heuristic by a small margin
void createMap(double currentX, double currentY, double targetX, double targetY) {
  int temp;

  Serial.println(currentX, 8);
  Serial.println(currentY, 8);
  Serial.println(targetX, 8);
  Serial.println(targetY, 8);

  double xdifference = targetX - currentX;
  double ydifference = targetY - currentY;
  xdifference /= 10.0;
  ydifference /= 10.0;

  Serial.println(xdifference, 8);
  Serial.println(ydifference, 8);

  // Populates the arrays with the map, with the first item in both arrays as the start point and the last item in both arrays as the end point
  // Map consists of 11 x 11, each point spaced evenly in the X and Y direction
  for (i = 0; i < 11; ++i) {
    for (j = 0; j < 11; ++j) {
      temp = ((i * 11) + j);
      mapLong[temp] = (currentX + (xdifference * j));
      mapLat[temp] = (currentY + (ydifference * i));
      heuristicVal[temp] = 10000.0 * sqrt((((mapLat[temp] - targetY) * 8.787) * ((mapLat[temp] - targetY) * 8.787) + ((mapLong[temp] - targetX) * 11.1045) * ((mapLong[temp] - targetX) * 11.1045)));
      beenTo[temp] = 0;
    }
  }

  for (i = 0; i < 11; ++i) {
    for (j = 0; j < 11; ++j) {
      temp = ((i * 11) + j);
      Serial.print(heuristicVal[temp], 8);
      Serial.print(" ");
      //Serial.print(mapLong[temp], 8);
      //Serial.print(",");
      //Serial.print(mapLat[temp], 8);
      //Serial.print(" ");
    }
    Serial.println();
  }

  // Uncomment these to create artifical heuristic changes
  //heuristicVal[1] += 999999;
  //heuristicVal[12] += 999999;
  //heuristicVal[23] += 999999;
  //heuristicVal[34] += 999999;
  //heuristicVal[45] += 999999;
  //heuristicVal[56] += 999999;
  //heuristicVal[67] += 999999;
  //heuristicVal[78] += 999999;
  //heuristicVal[89] += 999999;
  //heuristicVal[100] += 999999;

  /*
  The map created looks something like this
  110 111 112 113 114 115 116 117 118 119 120
  99  100 101 102 103 104 105 106 107 108 109
  88  89  90  91  92  93  94  95  96  97  98
  77  78  79  80  81  82  83  84  85  86  87
  66  67  68  69  70  71  72  73  74  75  76
  55  56  57  58  59  60  61  62  63  64  65
  44  45  46  47  48  49  50  51  52  53  54
  33  34  35  36  37  38  39  40  41  42  43
  22  23  24  25  26  27  28  29  30  31  32
  11  12  13  14  15  16  17  18  19  20  21
  0   1   2   3   4   5   6   7   8   9   10
  */

  // Determine the shortest path, which should be a diagonal line when we first determine it.
  // It will change when objects are detected and filled into the map
  // In our case the items inside the arrays at indexs: 0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120
  
  currentIndex = 0;
  fastestLat[currentIndex] = currentY;
  fastestLong[currentIndex] = currentX;
  beenTo[currentIndex] = 1;
  heuristicVal[currentIndex] += 9999;
  totalPoints = 1;
  
  while(fastestMapComplete == false) {
    if (currentIndex == 120) {
      fastestMapComplete = true;
    }
    else if (currentIndex == 108 || currentIndex == 109 || currentIndex == 119) {\
      if (currentIndex == 108) {
        bestChoice = 12;
      }
      else if (currentIndex == 109) {
        bestChoice = 11;
      }
      else if (currentIndex == 119) {
        bestChoice = 1;
      }
    }
    else if (currentIndex == 0) {
      // Check +1, +11, +12
      checkArray[0] = 12; checkArray[1] = 11; checkArray[2] = 1; 
      bestChoice = 12;
      for (i = 1; i < 3; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 110) {
      // Check -11, -10, +1
      checkArray[0] = 1; checkArray[1] = -11; checkArray[2] = -10; 
      bestChoice = 1;
      for (i = 1; i < 3; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 10) {
      // Check -1, +10, +11
      checkArray[0] = 11; checkArray[1] = 10; checkArray[2] = -1; 
      bestChoice = 11;
      for (i = 1; i < 3; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 1 || currentIndex == 2 || currentIndex == 3 || currentIndex == 4 || currentIndex == 5 || currentIndex == 6 || currentIndex == 7 || currentIndex == 8 || currentIndex == 9) {
      // Check -1, +1, +10, +11, +12
      checkArray[0] = 12; checkArray[1] = 11; checkArray[2] = 10; checkArray[3] = 1; checkArray[4] = -1;
      bestChoice = 12;
      for (i = 1; i < 5; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 11 || currentIndex == 22 || currentIndex == 33 || currentIndex == 44|| currentIndex == 55 || currentIndex == 66 || currentIndex == 77 || currentIndex == 88 || currentIndex == 99) {
      // Check -11, -10, +1, +11, +12
      checkArray[0] = 12; checkArray[1] = 11; checkArray[2] = 1; checkArray[3] = -10; checkArray[4] = -11;
      bestChoice = 12;
      for (i = 1; i < 5; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 21 || currentIndex == 32 || currentIndex == 43 || currentIndex == 54 || currentIndex == 65 || currentIndex == 76 || currentIndex == 87 || currentIndex == 98) {
      // Check -12, -11, -1, +10, +11
      checkArray[0] = 11; checkArray[1] = 10; checkArray[2] = -1; checkArray[3] = -11; checkArray[4] = -12;
      bestChoice = 11;
      for (i = 1; i < 5; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else if (currentIndex == 111 || currentIndex == 112 || currentIndex == 113 || currentIndex == 114 || currentIndex == 115 || currentIndex == 116 || currentIndex == 117 || currentIndex == 118) {
      // Check -12, -11, -10, -1, +1
      checkArray[0] = 1; checkArray[1] = -1; checkArray[2] = -10; checkArray[3] = -11; checkArray[4] = -12;
      bestChoice = 1;
      for (i = 1; i < 5; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    else {
      // Check -12, -11, -10, -1, +1, +10, +11, +12
      checkArray[0] = 12; checkArray[1] = 11; checkArray[2] = 10; checkArray[3] = 1; checkArray[4] = -1; checkArray[5] = -10; checkArray[6] = -11; checkArray[7] = -12;
      bestChoice = 12;
      for (i = 1; i < 8; ++i) {
        if (heuristicVal[currentIndex + checkArray[i]] < heuristicVal[currentIndex + bestChoice]) {
          bestChoice = checkArray[i];
          if (beenTo[currentIndex + bestChoice] == 1) {
            heuristicVal[currentIndex] += 9999;
          }
        }
      }
    }
    
    switch (bestChoice) {
      case 12:
        targetCardinal[totalPoints] = NorthEast;
        break;
      case 11:
        targetCardinal[totalPoints] = North;
        break;
      case 10:
        targetCardinal[totalPoints] = NorthWest;
        break;
      case 1:
        targetCardinal[totalPoints] = East;
        break;
      case -1:
        targetCardinal[totalPoints] = West;
        break;
      case -10:
        targetCardinal[totalPoints] = SouthEast;
        break;   
      case -11:
        targetCardinal[totalPoints] = South;
        break;
      case -12:
        targetCardinal[totalPoints] = SouthWest;
        break;  
      default:
        break;
    }

    currentIndex += bestChoice;
    beenTo[currentIndex] = 1;
    heuristicVal[currentIndex] += 9999;
    fastestLat[totalPoints] = mapLat[currentIndex];
    fastestLong[totalPoints] = mapLong[currentIndex];
    ++totalPoints;
  }

  --totalPoints;
  for (i = 0; i <= currentIndex; ++i) {
    beenTo[i] = 0;
  }
  currentIndex = 1;
}

// Create sub maps inside the map function.
void createSubMap(double currentX, double currentY, double targetX, double targetY) {
  if (currentIndex < (totalPoints + 1)) {
    subTargetX = fastestLong[currentIndex];
    subTargetY = fastestLat[currentIndex];
  }
}

int updateLocation(double currentLong, double currentLat) {
  // 0.000001 is the tolerance/radius of the actual target coordinates we need to be within to be considered complete
  // This is value of degrees which we can calculate as feet or meters
  if ((currentLong > endTargetX - 0.000005) && (currentLong < endTargetX - 0.000005) && (currentLat > endTargetY - 0.000006) && (currentLat < endTargetY + 0.000006)) {
    // clear Maps
    // clearMaps();
    return 2; // Means final destination was reached
  }
  else if ((currentLong > subTargetX - 0.000005) && (currentLong < subTargetX - 0.000005) && (currentLat > subTargetY - 0.000006) && (currentLat < subTargetY + 0.000006)) {
    return 1; // Means sub destination was reached
  }
  else {
    return 0; // Neither final or sub destination was reached
  }
}

// Autonomous movement function
int MoveTo(double &currentLongitude, double &currentLatitude, double &targetLongitude, double &targetLatitude, int mode) {
  currentLongitude = getLongitude();
  currentLatitude = getLatitude();
  //currentCardinal = getCardinal();

  // FOR DEMO SET THE TARGET TO ~25x25 METERS AWAY
  if (targetLongitude == 0 || targetLatitude == 0) {
    targetLongitude = currentLongitude + 0.000225;
    targetLatitude = currentLatitude + 0.000285;
    currentCardinal = North;
    Serial.print(targetLongitude, 8);
    Serial.print(" ");
    Serial.println(targetLatitude, 8);
  }

  // FOR ACTUAL
  /*
  if (targetLongitude == 0 || targetLatitude == 0) {
    return 0;
  }
  */

  if (mode == 0) {
    // Send -1 to alert GPS signal was unable to be found
    if (currentLongitude == 0 && currentLatitude == 0) {
      return -1;
    }
    else {
      LKSLongitude = currentLongitude;
      LKSLatitude = currentLatitude;
      // Map out destination if a current map doesn't exist
      if (mapExists == false) {
        Serial.println("Creating A* Map");
        createMap(currentLongitude, currentLatitude, targetLongitude, targetLatitude);
        mapExists = true;
        endTargetY = targetLongitude;
        endTargetX = targetLatitude;
        createSubMap(currentLongitude, currentLatitude, subTargetX, subTargetY);
      }
      
      currentLongitude = getLongitude();
      currentLatitude = getLatitude();
      Serial.print(currentLongitude,8);
      Serial.print(" ");
      Serial.println(currentLatitude,8);
      Serial.println();

      if (updateLocation(currentLongitude, currentLatitude) > 0) {
        Serial.println("Updating Location");
        // If subTarget reached, iterate to next point
        if (updateLocation(currentLongitude, currentLatitude) == 1) {
          Stop(2);
          ++currentIndex;
          createSubMap(currentLongitude, currentLatitude, subTargetX, subTargetY);
        }
        else if (updateLocation(currentLongitude, currentLatitude) == 2) {
          // Destination reached, do something to demonstrate completion like shake head or something idk
          // shakeHead();
        }
      }

      // Check if the current cardinal direction and target cardinal direction is the same

      while (currentCardinal != targetCardinal[currentIndex]) {
        // Determine which way we need to turn to get to the target Cardinal and also how far we are from it
        // Turn Left/Right function to reach target cardinal
        // Can make it better by having it overflow 1 to 8
        if (currentCardinal < targetCardinal[currentIndex]) {
          // Need to turn right
          int CardinalDifference = (targetCardinal[currentIndex] - currentCardinal) * 45;
          TurnRight(CardinalDifference);
          currentCardinal = targetCardinal[currentIndex];
          Stop(2);
          Serial.println("Target turn Reached");
        }
        else if (currentCardinal < targetCardinal[currentIndex]) {
          // Need to turn left
          int CardinalDifference = (currentCardinal - targetCardinal[currentIndex]) * 45;
          TurnLeft(CardinalDifference);
          currentCardinal = targetCardinal[currentIndex];
          Stop(2);
        }
        Serial.println("DOOR STUCK");
      }

      Serial.println("Moving Forward");
      MoveForward(20);
      return 0;

      // Check if an object is in front of the rover before moving
      if (checkObjectDetectionFront() < 0) {
        // Change the heuristic of whichever point is closer if there is an object detected
        // Point 120 is an exception, it will
        Serial.println("SOMETHING IS IN FRONT MOVE BITCH");
      }
      else {
        // Move towards destination
        Serial.println("Moving Forward");
        MoveForward(20);
        return 0;
      }
    }
  }
  // Mode 1 is run when GPS signal is lost and we are attempting to retrun to an Last Known Signal position
  else if (mode == 1) {
    if (currentLongitude == 0 && currentLatitude == 0) {
      if (LKSLongitude == 0 && LKSLatitude == 0) {
        // Send -2 if GPS signal is unavailable and LKS is (0,0) meaning there was no Last Known Signal location
        return -2;
      }
      else {
        // create a subtarget to LKS and render the current position as GPS signal loss which declares that location as no-go 
        // Move to LKS position and try a different route 
        
      }
    }
  }
  // Mode 2 is run when the LKS is spotted and we are traversing back to it
  else if (mode == 2) {
    // Create a submap and set the LKS as the target point
      // We will be in opposite cardinal direction so if the LKS is to the NE, we will be facing SW and reverse instead of moving forward
      // This helps distinct what the rover is doing on the livestream

    // while currLong/Lat == 0
    while (checkObjectDetectionBack() < 0) {
      // Do a turn ~20-35 degrees
      // iterate something to remember how much we turned
    }
    while ((checkObjectDetectionBack() >= 0) && currentLongitude == 0 && currentLatitude == 0) {
      // MoveReverse(25);
    }
    // Turn the amount you turned earlier in the opposite direction
  }
}

#endif
