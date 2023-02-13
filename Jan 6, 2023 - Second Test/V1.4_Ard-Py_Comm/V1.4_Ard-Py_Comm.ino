#include "Movement.h"

//String variables for the serial message
String nom = "Solar Turtle is ";
String message;
String msg;
String tempString; 
int num;

//Initialize
void setup() {
  Serial.begin(115200);
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
    Serial.print(nom);
    sendData();
  } 
  //If the command is none of the above, it will contain a 3-digit number, space, then the command (XXX COMMAND)
  else {
      
    tempString = message.substring(0, 3);
    num = tempString.toInt();
    msg = message.substring(4, message.length());
    
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
      Serial.print(nom + "Turning to the Left ");
      TurnRight(num);
    }
    else if (msg == "panel") {
      Serial.print("Schmooving the Panel");
      MovePanel(num);
    }

    num = 0;
    tempString = "";
    msg = "";
  }
  delay(500);
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