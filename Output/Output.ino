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

void recieveData() {
  for (int index : DataIndex) {
    HC05.write(index);
    HC05.flush();
    if (HC05.available()) {
      int data = HC05.read();
      safeData(index, data);
    }
  }
}

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
      readData.readWinkelData.roll = data;
      break;
  }
}

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
  // //printData();
  // float t2 = millis();
  // float td = t2 - t1;
  // Serial.print("td: ");
  // Serial.println(td);
  // delay(20);
}
