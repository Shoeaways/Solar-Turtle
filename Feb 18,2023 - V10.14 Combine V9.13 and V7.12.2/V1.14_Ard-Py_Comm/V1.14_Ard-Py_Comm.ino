/*
  Main file for the communication between our Arduino to Raspbery Pi
  Rev 1.14 Changes
  - Poll Power percent remaining every x minutes (Grab Test data)
    - Code SOC check for power percent (Added to data for now)
    - Can make sort of dynamic by including load and usage time of load for power remaining
  - Code Sleep mode and exit sleep mode
    - Raises isSleep flag and that doesn't allow the movement function to iterate
    - Also iterates SoC check and Solar Panel functions
    

  (Current Rev)
  - Code autonomous movement function
    - Create map function
      - Create submap function
    - Moveto function
      - Poll GPS connection
      - LKS function in the case GPS is not recieving signal
  - Object detection function
    - Ultrasonic sensors function
    - Connecting to image object detection information from the website
  - Review IMU calibration (Use quarternions)
    - Check the bounds of the IMU (240/290/270/290)
    - Determine what the compass heading points toward (what degree is N/E/S/W)

  (Future Rev) 
  - When the Rover lost comms, flag it and find a safe spot - Should then put rover in a wait mode for comms for a short period
    - If comms are not regained, return to last known position with comms
      - This means we would need a log recording if we still have comms and the coordinates that it was at 
  - Make a display for the map on the website

  (Notes)
  - In order to be some sort of insurance we can send DD/MM/YYYY and HR/MN/SC to the rover so that the rover does not idle indefinitely and has some sort of timeout to EVERYTHING
    - This will only work if there is internet connection

  (Clear issues we will run into)
  - If GPS and LKS are both (0,0) this requires human interaction
  - If the rover goes all possible locations AROUND the target but can never reach the target due to GPS loss or unmaneuverable land, it will go forever
    - This means we have to assume the final location is reachable

  - Need to account for if the robot stops under something (Can check with Panel V but there's no way to distinguish night and cover)
      - We can use a time function from HTML or something to return the current time to the Arduino

  - Losing internet connection also poses a huge threat and there is not really a fix for it with the current set up
    - We would need a way to detect connection loss as well as return to the last position with internet
*/

#include "Movement.h"

// String variables for the serial message
String nom = "Solar Turtle is ";
String message = "";
String command;
String tempString; 
int spaceIndex;
int num;
int chargePercent;
bool initFlag = true;

// Power automation flags
bool isSleep = false, isReady = false;
int checkPanelIterator = 0;
int checkSOCIterator = 0;

// Movement automation flags
bool isAutomated = true;
float currentLongitude = 0, currentLatitude = 0;
float targetLongitude, targetLatitude; 
int MoveStatus = 0;

// Start serial with 9600 baud rate on Arduino power up
void setup() {
  Serial.begin(9600);
}

void loop() {
  // Read the command from the serial port
  readSerialPort();
  message.toLowerCase();   

  if (isSleep == true) {
    if (message == "") {
      // Panel Iterator will allow us to delay how often the solar panel runs automation (240 iterations = ~1 minute)
      if (checkPanelIterator > 3600) {
        checkPanelIterator = 0;
        AutonomousSolarPanel();
      }
      else {
        ++checkPanelIterator;
      }

      // SOC Iterator will allow us to delay how often the SOC check runs (240 iterations = ~1 minute)
      if (checkSOCIterator > 1200) {
        checkSOCIterator = 0;
        chargePercent = checkSOC();

        // If SOC is > 90% exit sleep and resume commands
        if (chargePercent >= 90) {
          // Unraise sleep flag and execute exit sleep command
          // isSleep = false;
          // isReady = false;
          // exitSleep();
        }
        // If SOC is > 50% set ready to move but don't exit sleep
        else if(chargePercent >= 50) {
          // Raise ready flag
          // isReady = true;
        }
      }
      else {
        ++checkSOCIterator;
      }
    }
    else {
      if (isReady == true) {
        if (message == "data") {
          sendData();
        }
        // Allow resume command to run if it's called
        else if (message == "wake") {
          isSleep = false;
          isReady = false;
          exitSleep();
        }
      }
      else {
        if (message == "data") {
          sendData();
        }
      }
    }
  }
  else {
    // Initialization check
    if (initFlag == true) {
      checkPanelIterator = 0;
      initMovement();
      initData();
      initFlag = false;
    }
    if (isAutomated == true) {
      if (message == "") {
        // Panel Iterator will allow us to delay how often the solar panel runs automation
        // Realistically we would poll this every ~30-45 minutes instead of every ~30 seconds
        if (checkPanelIterator > 7200) {
          checkPanelIterator = 0;
          AutonomousSolarPanel();
        }
        else {
          ++checkPanelIterator;
        }

        // SOC Iterator will allow us to delay how often the SOC check runs
        // Realistically we would poll this every 3-5 minutes (can change based on our load usage) instead of every ~30 seconds
        if (checkSOCIterator > 1200) {
          checkSOCIterator = 0;
          chargePercent = checkSOC();

          // If SOC is < ~25% (12.9V @ Open Circuit)
          // if(chargePercent <= 25) {
            // Raise Sleep Mode flag and execute sleep function - Means all functions cannot be called except for data to save power
            // isSleep = true;
            // enterSleep();
              // Finish all short functions
              // Pause and remember all long functions
          // }
        }
        else {
          ++checkSOCIterator;
        }

        // Check if an object is in front of the rover here to ensure the rover doesn't begin by going into an object
        if (checkObjectDetection() < 0) {

        }

        // Move towards target longitude and latitude function with mode 0 (as normal)
        MoveStatus = MoveTo(currentLongitude, currentLatitude, targetLongitude, targetLatitude, 0);
        if (MoveStatus < 0) {
          if (MoveStatus == -1) {
            MoveTo(currentLongitude, currentLatitude, targetLongitude, targetLatitude, 1);
          }
          else if (MoveStatus == -2) {
            // Output to user that GPS signal is not available, please move rover
          }
        }
        else {
          // This means everything is working as intended
          // Report anything needed to be reported
        }
      }
      else {
        // Only valid commands during automated mode is to set it to manual mode and long/lat coords
        if (message == "manual") {
          // Switch to manual mode but also continue current function (Maybe store the current job and remember)
        }
        else {
          spaceIndex = message.indexOf(" ");
          tempString = message.substring(0, spaceIndex);
          targetLongitude = tempString.toFloat();
          tempString = message.substring((spaceIndex + 1), message.length());
          targetLatitude = tempString.toFloat();
        }
      }
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
          chargePercent = checkSOC();

          // If SOC is < ~25% (12.9V @ Open Circuit)
          // if(chargePercent <= 25) {
            // Raise Sleep Mode flag and execute sleep function - Means all functions cannot be called except for data to save power
            // isSleep = true;
            // enterSleep();
              // Finish all short functions
              // Pause and remember all long functions
          // }
        }
        else {
          ++checkSOCIterator;
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
