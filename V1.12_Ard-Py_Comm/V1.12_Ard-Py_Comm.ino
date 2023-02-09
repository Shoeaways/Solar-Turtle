/*
  Main file for the communication between our Arduino to Raspbery Pi
  Rev 1.12 Changes
  - Update Movement function
    - If user calls the same function but at a different speed, instead of slowing down to zero and 
      restarting, directly slow down or speed up to the new called speed
    - Added 3 levels of stopping (Quick, Normal, Slow)
  (Current Rev)
  (Future Rev)
  - Review IMU calibration
    - Check the bounds of the IMU
    - Determine what the compass heading points toward (what degree is N/E/S/W)
*/
 
#include "Movement.h"

// String variables for the serial message
String nom = "Solar Turtle is ";
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
      Serial.print(nom + "Quick Stopping...");
      Stop(3);
    }
    else if (message == "stop") {
      Serial.print(nom + "Stopping...");
      Stop(2);
    }
    else if (message == "slow stop") {
      Serial.print(nom + "Slow Stopping...");
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
        Serial.print(nom + "Schmooving Forward at " + String(num) + "% speed");
        MoveForward(num);    
      } 
      else if (command == "reverse") {
        Serial.print(nom + "Reverse Reverse at " + String(num) + "% speed");
        MoveReverse(num);
      }
      else if (command == "left") {
        Serial.print(nom + "Turning to the Left " + String(num) + " degrees");
        TurnLeft(num);
      }
      else if (command == "right") {
        Serial.print(nom + "Turning to the Right " + String(num) + " degrees");
        TurnRight(num);
      }
      else if (command == "panel") {
        Serial.print("Schmooving the Panel to " + String(num) + " degrees");
        MovePanel(num);
      }
      else if (command == "error") {
        Serial.print("New error margin of " + String(num) + "applied, I hope you know what you're doing...");
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
