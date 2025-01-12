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

static const uint8_t DataIndex[] = { 14, 15, 16, 17, 20, 88, 89 };

void recieveData() {
  for (int index : DataIndex) {
    HC05.write(index);
    HC05.flush();
    if (HC05.available()) {
      int data = HC05.read();
      move(index, data);
    }
  }
}

// Ansprechung von Servoobjekten

struct Hand {
  Finger FingerArray[] = {
  Servo pinky;
  Servo ring;
  Servo middle;
  Servo pointer;
  Servo thumb;
};

struct WinkelData {
  Servo pitch;  //X-Achse
  Servo roll;   //Y-Achse
} Drehung;
}
Hand;

void moveFinger(int FingerIndex, int Data) {
   /* Indexliste:
              0 -> pinky 
              1 -> ring
              2 -> middle
              3 -> pointer
              4 -> thumb
          */ 

  // TO-DO: muss noch getestet werden, wie stark die Finger angesteuert werden
  Hand.FingerArray[FingerIndex].write(data);
}

void move(int index, int data) {
  /* Indexliste:
              A0 | 14 -> Flex 1 -> pinky
              A1 | 15 -> Flex 2 -> ring
              A2 | 16 -> Flex 3 -> middle
              A3 | 17 -> Flex 4 -> pointer
              A6 | 20 -> Flex 5 -> thumb
              X  | 88 -> pitch
              Y  | 89 -> roll
              sonstiges -> 0 = sende erneut
          */

  switch (index) {
    case 14:
      moveFinger(0, data);
      break;
    case 15:
      // müssen aufgrund von aufbau gespiegelt angesteuert werden
      data = 180 - data;
      moveFinger(1, data);
      break;
    case 16:
      // müssen aufgrund von aufbau gespiegelt angesteuert werden
      data = 180 - data;
      moveFinger(2, data);
      break;
    case 17:
      moveFinger(3, data);
      break;
    case 20:
      moveFinger(4, data);
      break;
    case 88:
      data = map(data, 0, 255, 0, 180);
      Hand.Drehung.pitch.write(data);
      break;
    case 89:
      data = map(data, 0, 255, 0, 180);
      Hand.Drehung.roll.write(data);
      break;
  }
}

// Bluethooth Komponente ENDE

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Los geht's");
  setupHC05();
}

void loop() {
  // float t1 = millis();
  recieveData();

  // float t2 = millis();
  // float td = t2 - t1;
  // Serial.print("td: ");
  // Serial.println(td);
  delay(100);
}
