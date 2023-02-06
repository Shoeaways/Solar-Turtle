/*
  Main file for the communication between our Arduino to Raspbery Pi
  Rev 1.13 Changes
  - Update Movement function
    - Remove serial prints
  (Current Rev)
  (Future Rev)
  - Review IMU calibration
    - Check the bounds of the IMU
    - Determine what the compass heading points toward (what degree is N/E/S/W)
  - Create a Reinit function (Check for user command "reinitialize" which then sets the initFlag to true in order to reinit)
  - Set a low power mode/sleep mode for when the rover needs to charge
  - Create a SOC/SOH function
  - 
*/
 
#include "Movement.h"

// String variables for the serial message
String message = "";
String command;
String tempString; 
int initPanel;
int spaceIndex;
int num;
bool initFlag = true;

// Start serial on Arduino power up
void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read the command from the serial port
  readSerialPort();
  message.toLowerCase();   

  // Initialization check
  if (initFlag == true) {
    initMovement();
    initData();
    initFlag = false;
  }
  else {
    // Stop command stops the robots movement
    if (message == "quick stop") {
      Stop(3);
    }
    else if (message == "stop") {
      Stop(2);
    }
    else if (message == "slow stop") {
      Stop(1);
    }
    // Data command sends important values such as GPS coordinates or current Roll/Pitch/Yaw
    else if (message == "data") {
      sendData();
    } 
    // If the command is none of the above, it will contain a number, space, then the command (XXX COMMAND)
    else {
      // Find the index of the space (spaceIndex) and split message into two strings, number and command
      spaceIndex = message.indexOf(" ");
      tempString = message.substring(0, spaceIndex);
      num = tempString.toInt();
      command = message.substring((spaceIndex + 1), message.length());
      
      if (command == "forward") {
        MoveForward(num);    
      } 
      else if (command == "reverse") {
        MoveReverse(num);
      }
      else if (command == "left") {
        TurnLeft(num);
      }
      else if (command == "right") {
        TurnRight(num);
      }
      else if (command == "panel") {
        MovePanel(num);
      }
      else if (command == "error") {
        changeErrorMargin(num);
      }

      num = 0;
      command = "";
    }
  }
  delay(250);
}

// Read serial port
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
