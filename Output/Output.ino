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

static const uint8_t DataIndex[] = { 14, 15, 16, 17, 20, 21, 88, 89 };

/* brauche Werte nicht speichern -> wechseln zu schnell & kann direkt als Argument weitergeben
struct Data {
  struct FlexData {
    int Flex0;
    int Flex1;
    int Flex2;
    int Flex3;
    int Flex4;
    int Potentiometer;
  } readFlexData;

  struct WinkelData {
    int pitch;  //X-Achse
    int roll;   //Y-Achse
  } readWinkelData;

} readData; 
*/

void recieveData() {
  for (int index : DataIndex) {
    HC05.write(index);
    HC05.flush();
    if (HC05.available()) {
      int data = HC05.read();
      // safeData(index, data); speichern obsolet
      move(index, data);
    }
  }
}

// Ansprechung von Servoobjekten

struct Finger {
  Servo Servo0;
  Servo Servo1;
  Servo Servo2;  // Pin zum Öffnen|schließen der Hand
};

struct Hand {
  Finger FingerArray[] = {
    Hand.Finger0,  // pinky
    Hand.Finger1,  // Ring
    Hand.Finger2,  // middle
    Hand.Finger3,  // pointer
    Hand.Finger4   // thumb
  };

  struct WinkelData {
    Servo pitch;  //X-Achse
    Servo roll;   //Y-Achse
  } Drehung;

} Hand;

void moveFinger(int FingerIndex, int Data) {
  // TO-DO: muss noch getestet werden, wie stark die Finger angesteuert werden
  Hand.FingerArray[FingerIndex].Servo0.write(data);
  if (data > 90) {
    Hand.FingerArray[FingerIndex].Servo1.write(data);
  }
}

void oeffne(int data) {
  Hand.FingerArray[0].Servo2.write(data); 
  Hand.FingerArray[1].Servo2.write(data);
  Hand.FingerArray[2].Servo2.write(180-data);
  Hand.FingerArray[3].Servo2.write(180-data);
  // Hand.FingerArray[4].Servo2.write(data);
}

void move(int index, int data) {
  /* Indexliste:
              A0 | 14 -> Flex 1
              A1 | 15 -> Flex 2
              A2 | 16 -> Flex 3
              A3 | 17 -> Flex 4
              A6 | 20 -> Flex 5
              A7 | 21 -> Handöffnung
              X  | 88 -> pitch
              Y  | 89 -> roll
              sonstiges -> 0 = sende erneut
          */

  switch (index) {
    case 14:
      moveFinger(0, data);
      break;
    case 15:
      moveFinger(1, data);
      break;
    case 16:
      moveFinger(2, data);
      break;
    case 17:
      moveFinger(3, data);
      break;
    case 20:
      moveFinger(4, data);
      break;
    case 21:
      oeffne(data);  // doch eigene methode
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
/* speichern obsolet
void safeData(int index, int data) {
  switch (index) {
    case 14:
      readData.readFlexData.Flex0 = data;
      break;
    case 15:
      readData.readFlexData.Flex1 = data;
      break;
    case 16:
      readData.readFlexData.Flex2 = data;
      break;
    case 17:
      readData.readFlexData.Flex3 = data;
      break;
    case 20:
      readData.readFlexData.Flex4 = data;
      break;
    case 21:
      readData.readFlexData.Potentiometer = data;
      break;
    case 88:
      if (data > 165) {
        data = data - 255;
      }
      readData.readWinkelData.pitch = data;
      break;
    case 89:
      data = map(data, 0, 255, -180, 180);
      readData.readWinkelData.roll = data;
      break;
  }
}
*/

/* braucht speicherung zum Ausgeben
void printData() {
  Serial.println("---------------------------------------------------------------------------------------------");
  Serial.print("Flex0: ");
  Serial.print(readData.readFlexData.Flex0);
  Serial.print("\t");

  Serial.print("Flex1: ");
  Serial.print(readData.readFlexData.Flex1);
  Serial.print("\t");

  Serial.print("Flex2: ");
  Serial.print(readData.readFlexData.Flex2);
  Serial.print("\t");

  Serial.print("Flex3: ");
  Serial.print(readData.readFlexData.Flex3);
  Serial.print("\t");

  Serial.print("Flex4: ");
  Serial.print(readData.readFlexData.Flex4);
  Serial.print("\t");

  Serial.println();
  Serial.print("Potentiometer: ");
  Serial.print(readData.readFlexData.Potentiometer);
  Serial.print("\t\t");

  Serial.print("pitch: ");
  Serial.print(readData.readWinkelData.pitch);
  Serial.print("\t");

  Serial.print("roll: ");
  Serial.print(readData.readWinkelData.roll);

  Serial.println();
  Serial.println("---------------------------------------------------------------------------------------------");
}
*/
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
  // printData();
  // float t2 = millis();
  // float td = t2 - t1;
  // Serial.print("td: ");
  // Serial.println(td);
  delay(100);
}
