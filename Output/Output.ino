#include <SoftwareSerial.h>
#include <Servo.h>

const int delayTime = 100;

// Bluethooth Komponente START
const uint8_t rxPORT = 10;
const uint8_t txPORT = 11;

SoftwareSerial HC05(rxPORT, txPORT);

void setupHC05() {
  HC05.begin(38400);
  String sendMsg = "Pong";
  String expectAnswer = "Ping";
  String recieveMsg = "";
  bool suche = true;

  while (suche) {
    HC05.overflow();
    if (HC05.available()) {
      recieveMsg = HC05.readString();

      if (recieveMsg.equals(expectAnswer)) {
        HC05.print(sendMsg);
        Serial.println(sendMsg);
        suche = false;
      }
    }
  }
  delay(2000);
}
// Bluethooth Komponente ENDE

static const uint8_t DataIndex[] = { 14, 15, 16, 17, 20, 21, 88, 89 };  // siehe readData

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Los geht's");
  setupHC05();
}

void loop() {
  for (int index : DataIndex) {
    HC05.write(index);
      Serial.print("Data request from: ");
      Serial.print(index);
      Serial.println(" ");
    HC05.flush();
    delay(50);
  }

  if (HC05.available()) {
    Serial.println(HC05.read());
  } 

  delay(100);
}
