#include <MPU9250_WE.h>
#include <Wire.h>
#include <SoftwareSerial.h>

const int delayTime = 50;
static const uint8_t AnalogPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };  // A4 & A5 sind SDA & SCL pins || A0-A6 belegt FlexSensoren || A7 belegt durch Potentiometer
static const uint8_t LEDPins[] = { 3, 5, 6, 9, 10 };
static const uint8_t Bluetooth_LEDPIN = 4;

static const uint8_t BUTTON_PIN = 2;
bool ButtonPressed() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
    return true;
  }
  return false;
}

// Flex-Sensor Komponenten START
const int AnzahlFlex = 5;

class Flex {
public:
  int getPORT();
  int getFlexWert();
  void setFlexWert(int NeuFlexWert);
  int FlexMessen();
  void kalibrieren();
  void kalibrieren(bool SKIP);

  Flex(int PORT, int newLED_PORT) {
    AnalogInputPORT = PORT;
    FlexWert = 0;
    LED_PORT = newLED_PORT;
    pinMode(LED_PORT, OUTPUT);
    kalibrieren();
  }

private:
  int AnalogInputPORT;
  int16_t FlexWert;
  int LED_PORT;

  // gibt die Beugung in Winkel von 0-180 Grad wieder
  int16_t ObergrenzeAlt = 0;      // gelesene Grenzen zuerst invertiert
  int16_t UntergrenzeAlt = 1023;  // und in kallibrierung angepasst
  int16_t ObergrenzeNeu = 180;
  int16_t UntergrenzeNeu = 0;
};

int Flex::getPORT() {
  return AnalogInputPORT;
}
int Flex::getFlexWert() {
  return FlexWert;
}
void Flex::setFlexWert(int NeuFlexWert) {
  FlexWert = NeuFlexWert;
}
int Flex::FlexMessen() {
  FlexWert = analogRead(AnalogInputPORT);
  analogWrite(LED_PORT, FlexWert/ 4);
  FlexWert = map(FlexWert, UntergrenzeAlt, ObergrenzeAlt, UntergrenzeNeu, ObergrenzeNeu);

  return FlexWert;
}
void Flex::kalibrieren() {
  digitalWrite(LED_PORT, HIGH);
  delay(1000);
  while (!ButtonPressed()) {
    FlexWert = analogRead(AnalogInputPORT);
    analogWrite(LED_PORT, FlexWert / 4);
    if (FlexWert > ObergrenzeAlt) { ObergrenzeAlt = FlexWert; }
    if (FlexWert < UntergrenzeAlt) { UntergrenzeAlt = FlexWert; }
  }
  Serial.print("ObergrenzeAlt: ");
  Serial.print(ObergrenzeAlt);
  Serial.print("\t");
  Serial.print("UntergrenzeAlt: ");
  Serial.print(UntergrenzeAlt);
  Serial.println();
  digitalWrite(LED_PORT, LOW);
}
void Flex::kalibrieren(bool SKIP) {
  // durchschnitt von bereits kalibrierten Werten
  ObergrenzeAlt = 853;
  UntergrenzeAlt = 0;
}

Flex* FlexArray[AnzahlFlex];
int setupFlex(int AnzahlFlex) {
  switch (AnzahlFlex) {
    case 5:
      FlexArray[4] = new Flex(AnalogPins[6], LEDPins[4]);
    case 4:
      FlexArray[3] = new Flex(AnalogPins[3], LEDPins[3]);
    case 3:
      FlexArray[2] = new Flex(AnalogPins[2], LEDPins[2]);
    case 2:
      FlexArray[1] = new Flex(AnalogPins[1], LEDPins[1]);
    case 1:
      FlexArray[0] = new Flex(AnalogPins[0], LEDPins[0]);
      break;
    default:
      return 0;
      break;
  }
  return 1;
}

void loopFlex() {
  for (int i = 0; i < AnzahlFlex; i++) {
    int Messwert = FlexArray[i]->FlexMessen();
    FlexArray[i]->setFlexWert(Messwert);
  }
}
// Flex-Sensor Komponenten ENDE

// MPU9250 Komponente START
#define MPU9250_ADDR 0x68

struct Winkel {
  float pitch;  // X-Achse
  float roll;   // Y-Achse
} sendeWinkel;

MPU9250_WE MPU9250 = MPU9250_WE(MPU9250_ADDR);
int setupMPU9250() {
  Wire.begin();

  if (!MPU9250.init()) {
    Serial.println("MPU9250 does not respond");
  } else {
    Serial.println("MPU9250 is connected");
  }

  Serial.println("Position your MPU9250 flat and don't move it - calibrating");
  Serial.print(".");
  delay(330);
  Serial.print(".");
  delay(330);
  Serial.print(".");
  delay(340);

  MPU9250.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);
  // oder auch folgende Methode
  //MPU9250.autoOffsets();
  Serial.println("Done!");

  MPU9250.setSampleRateDivider(5);
  MPU9250.setAccRange(MPU9250_ACC_RANGE_2G);
  MPU9250.enableAccDLPF(true);
  MPU9250.setAccDLPF(MPU9250_DLPF_6);

  MPU9250.enableGyrDLPF();
  MPU9250.setGyrDLPF(MPU9250_DLPF_6);
  MPU9250.setGyrRange(MPU9250_GYRO_RANGE_1000);

  return 1;
}
void loopMPU9250() {
  sendeWinkel.pitch = MPU9250.getPitch();
  sendeWinkel.roll = MPU9250.getRoll();
}
// MPU9250 Komponente ENDE

// Bluethooth Komponente START
const uint8_t txPORT = 11;
const uint8_t rxPORT = 12;

SoftwareSerial HC05(txPORT, rxPORT);

int setupHC05() {
  HC05.begin(115200);
  String sendMsg = "Ping";
  String expectAnswer = "Pong";
  String recieveMsg = "";

  while (true) {
    digitalWrite(Bluetooth_LEDPIN, LOW);
    // Code um Verbindung zu überprüfen
    if (!(HC05.available())) {
      // keine Nachricht empfangen
      HC05.print(sendMsg);
      Serial.println(sendMsg);
      delay(500);
    } else {
      // Nachricht empfangen
      recieveMsg = HC05.readString();
      Serial.println(recieveMsg);
      digitalWrite(Bluetooth_LEDPIN, HIGH);
      if (recieveMsg.equals(expectAnswer)) {
        return 1;
      }
    }
    digitalWrite(Bluetooth_LEDPIN, HIGH);
    Serial.println("Fehler bei Bluetoothverbinung");
    Serial.println("Versuche erneut");
    delay(250);
    digitalWrite(Bluetooth_LEDPIN, LOW);
    Serial.print(".");
    delay(250);
    digitalWrite(Bluetooth_LEDPIN, HIGH);
    Serial.print(".");
    delay(250);
    digitalWrite(Bluetooth_LEDPIN, LOW);
    Serial.print(".");
    Serial.println("");
    digitalWrite(Bluetooth_LEDPIN, HIGH);
    delay(250);
    digitalWrite(Bluetooth_LEDPIN, LOW);
  }
}
static const uint8_t DataIndex[] = { 14, 15, 16, 17, 20, 88, 89 };  // siehe readData
int readData(int Index) {
  /* Plan: Empfange von Index, sende Date
                  Indexliste:
                      A0 | 14 -> Flex 1
                      A1 | 15 -> Flex 2
                      A2 | 16 -> Flex 3
                      A3 | 17 -> Flex 4
                      A6 | 20 -> Flex 5
                      X  | 88 -> pitch
                      Y  | 89 -> roll
                      sonstiges -> 0 = sende erneut
                  */
  int data;
  switch (Index) {
    case 0xe:  // 14 bzw A0
      data = FlexArray[0]->getFlexWert();
      break;
    case 0xf:  // 15 bzw A1
      data = FlexArray[1]->getFlexWert();
      break;
    case 0x10:  // 16 bzw A2
      data = FlexArray[2]->getFlexWert();
      break;
    case 0x11:  // 17 bzw A3
      data = FlexArray[3]->getFlexWert();
      break;
    case 0x14:  // 20 bzw A6
      data = FlexArray[4]->getFlexWert();
      break;
    case 88:
      data = sendeWinkel.pitch;
      break;
    case 89:
      //data = sendeWinkel.roll;
      data = map(sendeWinkel.roll, -180, 180, 0, 255);
      break;
    default:
      data = -999;
      break;
  }
  return data;
}

void loopHC05() {
  if (HC05.available()) {
    int request = (int)HC05.read();
    int data = readData(request);
    HC05.write(data);
    HC05.flush();
  }
}
// Bluethooth Komponente ENDE

  //Prüfe Sensoren & gebe Werte aus
void pruefeSensoren() {
  Serial.println("---------------------------------------------------------------------------------------------");
  Serial.print("Flex0: ");
  Serial.print(readData(DataIndex[0]));
  Serial.print("\t");

  Serial.print("Flex1: ");
  Serial.print(readData(DataIndex[1]));
  Serial.print("\t");

  Serial.print("Flex2: ");
  Serial.print(readData(DataIndex[2]));
  Serial.print("\t");

  Serial.print("Flex3: ");
  Serial.print(readData(DataIndex[3]));
  Serial.print("\t");

  Serial.print("Flex4: ");
  Serial.print(readData(DataIndex[4]));
  Serial.print("\n");

  Serial.print("pitch: ");
  Serial.print(readData(DataIndex[5]));
  Serial.print("\t");

  Serial.print("roll: ");
  Serial.print(readData(DataIndex[6]));

  Serial.println();
  Serial.println("---------------------------------------------------------------------------------------------");
}


void setup() {
  Serial.begin(9600);
  Serial.println("Los geht's");
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  // FlexSensoren initialisiert
  Serial.println("Initialisiere Flex...");
  while (!setupFlex(AnzahlFlex)) {
    Serial.println("Fehler bei Flex-Initialisierung");
    Serial.println("Versuche erneut...");
  }
  Serial.println("Flex-Initialisierung erfolgreich");

  //Bewegungs/Neigungssensoren initialisiert
  while (!setupMPU9250()) {
    Serial.println("Fehler bei Bewegungssensor");
    Serial.println("Versuche erneut...");
  }
  Serial.println("Bewegungssensor erfolgreich gestartet");

  // Bluetoothmodul initialisiert
  pinMode(Bluetooth_LEDPIN, OUTPUT);
  setupHC05();
  Serial.println("Bluetoothverbinung erfolgreich hergestellt");
}

void loop() {
  // float t1 = millis();

  loopFlex();     //  loopFlex & loopMPU9250 brauchen 3-5 millisekunden zum ausführen
  loopMPU9250();  //  loopHC05 ~3 millisekunden zum ausführen
  loopHC05();     //  Schätzung: Abweichung in betrieb mit 10 millisekunden rechnen

  //pruefeSensoren();

  //  float t2 = millis();
  //  float td = t2-t1;
  //  if (td > 5) {Serial.print("td: "); Serial.println(td);}
}
