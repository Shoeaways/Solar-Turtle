String message;

void setup() {
  Serial.begin(9600);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(11, HIGH);
  digitalWrite(12, LOW); 
}

// the loop function runs over and over again forever
void loop() {
  readSerialPort();

  Serial.print(message);

  if (message == "blink") {
    digitalWrite(11, HIGH);
    digitalWrite(12, HIGH); 
  }
  else if (message == "stop") {   
    digitalWrite(11, LOW);
    digitalWrite(12, LOW); 
  }

  delay(250);
}

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
