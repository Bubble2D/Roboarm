#include <SoftwareSerial.h>

SoftwareSerial hc06(10, 11);
String sendMsg = "Ping an UNO";
String receiveMsg = "";

void setup() {

  Serial.begin(9600);
  Serial.println("Los geht's");
  hc06.begin(38400);  //38400 für hc-05
  hc06.print(sendMsg);
}
void loop() {  // run over and over
  if (hc06.available()) {
    receiveMsg = hc06.readString();
    Serial.println(receiveMsg);
    delay(20);
    hc06.print(sendMsg);
  }

  if (Serial.available()) {
    hc06.print(Serial.readString());
  }
}