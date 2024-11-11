

static const uint8_t AnalogPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };  // A4 & A5 sind SDA & SCL pins || A0-A6 belegt

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
}

// Flex-Sensor Komponenten ENDE

void setup() {
  Serial.begin(9600);
  setupFlex(AnzahlFlex);
}

void loop() {
  loopFlex();

  Serial.print(FlexArray[0]->getFlexWert());
  Serial.print("|");
  Serial.print(FlexArray[1]->getFlexWert());
  Serial.println();

  delay(200);
}
