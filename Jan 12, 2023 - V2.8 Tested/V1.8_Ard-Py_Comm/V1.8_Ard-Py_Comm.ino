/*
  Main file for the communication between our Raspberry Pi and Arduino

  Rev 1.8 Changes
  - Implement sensors into data.h file (voltage and current)
  Todo
  (Current Rev)
  (Future Rev)
  - Check to see if we can call functions directly from data.h since we included Movement.h
  - For errors, start the string with Error:~ so that the Rpi can read that there was an 
    error and work accordingly
  - Account for noise in voltage/current sensors
*/

#include "Movement.h"

// String variables for the serial message
String nom = "Solar Turtle is ";
String message;
String command;
String tempString; 
int spaceIndex;
int num;

// Initialize
void setup() {
  Serial.begin(9600);
  initMovement();
  initData();
}

void loop() {
  // Read the command from the serial port
  readSerialPort();
  message.toLowerCase();   
  
  // Stop command stops the robots movement
  if (message == "stop") {
    Serial.print(nom + "Stopping...");
    Stop();
  }

  // Data command sends important values such as GPS coordinates or current Roll/Pitch/Yaw
  if (message == "data") {
    sendData();
  } 

  if (command == "voltage") {
      Serial.print("System Voltage: " + updateVAsenseSystem());
  }
  
  // If the command is none of the above, it will contain a number, space, then the command (XXX COMMAND)
  else {
    // Find the index of the space (spaceIndex) and split message into two strings, number and command
    spaceIndex = message.indexOf(" ");
    tempString = message.substring(0, spaceIndex);
    num = tempString.toInt();
    command = message.substring((spaceIndex + 1), message.length());
    
    if (command == "forward") {
      Serial.print(nom + "Schmooving Forward ");
      MoveForward(num);    
    } 
    else if (command == "reverse") {
      Serial.print(nom +"Reverse Reverse");
      MoveReverse(num);
    }
    else if (command == "left") {
      Serial.print(nom + "Turning to the Left ");
      TurnLeft(num);
    }
    else if (command == "right") {
      Serial.print(nom + "Turning to the Right ");
      TurnRight(num);
    }
    else if (command == "panel") {
      Serial.print("Schmooving the Panel");
      MovePanel(num);
    }
    num = 0;
    command = "";
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