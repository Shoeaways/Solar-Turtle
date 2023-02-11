/*
  Main file for the communication between our Arduino to Raspbery Pi
  Rev 1.12.2 Changes
  - Added autonomous solar panel movement
  (Current Rev)
  (Future Rev)
  - Review IMU calibration
    - Check the bounds of the IMU (240/290/270/290)
    - Determine what the compass heading points toward (what degree is N/E/S/W)
  - When the Rover lost comms, flag it and find a safe spot - Should then put rover in a wait mode for comms for a short period
    - If comms are not regained, return to last known position with comms
      - This means we would need a log recording if we still have comms and the coordinates that it was at 
  - Poll Power percent remaining every x minutes
    - Code SOC check for power percent (Added to data for now)
    - Can make sort of dynamic by including load and usage time of load for power remaining
  - When rover is stopped for charging, code the solar panel power check to work alongside with the rover turning
*/

#include "Movement.h"

// String variables for the serial message
String nom = "Solar Turtle is ";
String message = "";
String command;
String tempString; 
int spaceIndex;
int num;
bool initFlag = true;

int checkPanelIterator = 0;
int checkSOCIterator = 0;

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
    checkPanelIterator = 0;
    initMovement();
    initData();
    initFlag = false;
  }
  else {
    // Autonomous solar panel (Checks while the robot is idle/waiting for command)
    if (message == "") {
      // Panel Iterator will allow us to delay how often the solar panel runs automation
      // Realistically we would poll this every ~30-45 minutes instead of every ~30 seconds
      if (checkPanelIterator > 120) {
        checkPanelIterator = 0;
        AutonomousSolarPanel();
      }
      else {
        ++checkPanelIterator;
      }

      // SOC Iterator will allow us to delay how often the SOC check runs
      // Realistically we would poll this every 3-5 minutes (can change based on our load usage) instead of every ~30 seconds
      if (checkSOCIterator > 120) {
        checkSOCIterator = 0;
        // Check SOC
        // If SOC is < X%
          // Finish all short functions
          // Pause and remember all long functions
          // Enter Sleep Mode (set isSleep to true)
            // Means all functions cannot be called except for data to save power
      }
    }
    else {
    // Stop command stops the robots movement
      if (message == "init") {
        Serial.print(nom + "reinitializing!");
        initFlag = true;
      }
      else if (message == "quick stop") {
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
  }
  // DO NOT CHANGE THIS DELAY UNLESS WE HAVE CONSIDERED THAT IT AFFECTS THE SOC AND PANEL CHECKS
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
