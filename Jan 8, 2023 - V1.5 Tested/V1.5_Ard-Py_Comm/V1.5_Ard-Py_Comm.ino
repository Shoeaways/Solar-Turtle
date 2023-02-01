/*
  Main file for the communication between our Raspberry Pi and Arduino

  Rev 1.5 Changes
  - Removed serialprint(nom) in the data function call
  - Added the indexOf function to implement a dynamic string splitter for QoL
*/

#include "Movement.h"

//String variables for the serial message
String nom = "Solar Turtle is ";
String message;
String msg;
String tempString; 
int spaceIndex;
int num;

//Initialize
void setup() {
  Serial.begin(9600);
  initMovement();
  initData();
}

void loop() {
  //Read the command from the serial port
  readSerialPort();

  message.toLowerCase();   
  
  //Stop command stops the robots movement
  if (message == "stop") {
    Serial.print(nom + "Stopping...");
    Stop();
  }

  //Data command sends important values such as GPS coordinates or current Roll/Pitch/Yaw
  if (message == "data") {
    sendData();
  } 
  //If the command is none of the above, it will contain a number, space, then the command (XXX COMMAND)
  else {
    //Find the index of the space and split message into two strings, number and command
    spaceIndex = message.indexOf(" ");
    tempString = message.substring(0, spaceIndex);
    num = tempString.toInt();
    msg = message.substring((spaceIndex + 1), message.length());
    
    if (msg == "forward") {
      Serial.print(nom + "Schmooving Forward ");
      MoveForward(num);    
    } 
    else if (msg == "reverse") {
      Serial.print(nom +"Reverse Reverse");
      MoveReverse(num);
    }
    else if (msg == "left") {
      Serial.print(nom + "Turning to the Left ");
      TurnLeft(num);
    }
    else if (msg == "right") {
      Serial.print(nom + "Turning to the Right ");
      TurnRight(num);
    }
    else if (msg == "panel") {
      Serial.print("Schmooving the Panel");
      MovePanel(num);
    }

    num = 0;
    msg = "";
  }
  delay(250);
}

//Read serial port
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
