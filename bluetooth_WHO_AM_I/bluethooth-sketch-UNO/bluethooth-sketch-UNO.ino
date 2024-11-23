#include <SoftwareSerial.h>


SoftwareSerial hc05(10, 11);
String sendMsg = "Pong an Nano";
String receiveMsg = "";

void setup() {

  Serial.begin(9600);
  Serial.println("Los geht's");
  hc05.begin(38400); //38400 für hc-05
  hc05.print(sendMsg);
}

void loop() { // run over and over
  if (hc05.available()) {
    receiveMsg = hc05.readString();
    Serial.println(receiveMsg);
    delay(1000);
    hc05.print(sendMsg);
  }
  if (Serial.available()) {
    hc05.print(Serial.readString());
  }
}