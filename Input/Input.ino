#include <MPU9250_WE.h>
#include <Wire.h>

const int delayTime = 200;
static const uint8_t AnalogPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };  // A4 & A5 sind SDA & SCL pins || A0-A6 belegt FlexSensoren {A7 belegt durch Potentiometer}

// Flex-Sensor Komponenten START
class Flex {
public:
  int getPORT();
  int getFlexWert();
  void setFlexWert(int NeuFlexWert);
  int FlexMessen();

  Flex(int PORT) {
    AnalogInputPORT = PORT;
    FlexWert = 0;
  }
private:
  int AnalogInputPORT;
  int16_t FlexWert;
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
  // gibt die Beugung in Winkel von 0-180 Grad wieder
  int16_t ObergrenzeAlt = 1023;
  int16_t UntergrenzeAlt = 0;
  int16_t ObergrenzeNeu = 180;
  int16_t UntergrenzeNeu = 0;

  FlexWert = analogRead(AnalogInputPORT);
  FlexWert = map(FlexWert, UntergrenzeAlt, ObergrenzeAlt, UntergrenzeNeu, ObergrenzeNeu);

  return FlexWert;
}

const int AnzahlFlex = 5;
Flex* FlexArray[AnzahlFlex];
int setupFlex(int AnzahlFlex) {
  switch (AnzahlFlex) {
    case 5:
      FlexArray[4] = new Flex(AnalogPins[6]);
    case 4:
      FlexArray[3] = new Flex(AnalogPins[3]);
    case 3:
      FlexArray[2] = new Flex(AnalogPins[2]);
    case 2:
      FlexArray[1] = new Flex(AnalogPins[1]);
    case 1:
      FlexArray[0] = new Flex(AnalogPins[0]);
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
  loopSpreitzen();
}
// Fingerspreitzen start
int spreitzung;
void loopSpreitzen() {
  spreitzung = analogRead(AnalogPins[7]);
  spreitzung = map(spreitzung, 0, 1023, 0, 180);
}
// Fingerspreitzen ende


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

  Serial.println("Position you MPU9250 flat and don't move it - calibrating");
  Serial.print(".");
  delay(330);
  Serial.print(".");
  delay(330);
  Serial.print(".");
  delay(340);

  //MPU9250.setAccOffsets(-14240.0, 18220.0, -17280.0, 15590.0, -20930.0, 12080.0);
  // oder auch folgende Methode
  MPU9250.autoOffsets();
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

void setup() {
  Serial.begin(9600);
  while (!setupFlex(AnzahlFlex)) {
    Serial.println("Fehler bei Flex-Initialisierung");
    Serial.println("Versuche erneut...");
  }
  Serial.println("Flex-Initialisierung erfolgreich");

  while (!setupMPU9250()) {
    Serial.println("Fehler bei Bewegungssensor");
    Serial.println("Versuche erneut...");
  }
  Serial.println("Bewegungssensor erfolgreich gestartet");
}

void loop() {
  loopFlex();
  loopMPU9250();

  Serial.println(analogRead(A2));

  delay(delayTime);
}
