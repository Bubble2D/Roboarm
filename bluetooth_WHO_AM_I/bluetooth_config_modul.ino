#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX / TX
void setup() {
  Serial.begin(9600);
  Serial.println("Los geht's");
  mySerial.begin(38400); //38400 für hc-05
}
void loop() { // run over and over
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}