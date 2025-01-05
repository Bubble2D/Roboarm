#include <MPU9250_WE.h>
#include <Wire.h>
#include <SoftwareSerial.h>

const int delayTime = 50;
static const uint8_t AnalogPins[] = { A0, A1, A2, A3, A4, A5, A6, A7 };  // A4 & A5 sind SDA & SCL pins || A0-A6 belegt FlexSensoren || A7 belegt durch Potentiometer

// Flex-Sensor Komponenten START
  const int AnzahlFlex = 5;

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
    int16_t ObergrenzeAlt = 880;  // abweichung auf von 1023 auf ca. 880
    int16_t UntergrenzeAlt = 0;
    int16_t ObergrenzeNeu = 180;
    int16_t UntergrenzeNeu = 0;

    FlexWert = analogRead(AnalogInputPORT);
    FlexWert = map(FlexWert, UntergrenzeAlt, ObergrenzeAlt, UntergrenzeNeu, ObergrenzeNeu);

    return FlexWert;
  }


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
    loopHandoeffnung();
  }
  // Handoeffnung start
    int Handoeffnung;
    void loopHandoeffnung() {
      Handoeffnung = analogRead(AnalogPins[7]);
      Handoeffnung = map(Handoeffnung, 0, 1023, 0, 180);
    }
  // Handoeffnung ende
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

// Bluethooth Komponente START
    const uint8_t txPORT = 10;
    const uint8_t rxPORT = 11;

    SoftwareSerial HC05(txPORT, rxPORT);

    int setupHC05() {
      HC05.begin(38400);
      String sendMsg = "Ping";
      String expectAnswer = "Pong";
      String recieveMsg = "";

      while (true) {
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
          if (recieveMsg.equals(expectAnswer)) {
            return 1;
          }
        }
        Serial.println("Fehler bei Bluetoothverbinung");
        Serial.println("Versuche erneut");
        delay(330);

        Serial.print(".");
        delay(330);
        Serial.print(".");
        delay(340);
        Serial.print(".");
        Serial.println("");
      }
    }
    static const uint8_t DataIndex[] = { 14, 15, 16, 17, 20, 21, 88, 89 };  // siehe readData
    int readData(int Index) {
      /* Plan: Empfange von Index, sende Date
              Indexliste:
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
        case 0x15:  // 21 bzw A7
          data = Handoeffnung;
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
          if (request == 89 && data < 0) {
            data = (-1*data);
          }
        HC05.write(data);
        HC05.flush();
      }
    }

// Bluethooth Komponente ENDE

/* //Prüfe Sensoren & gebe Werte aus
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
    Serial.print("\t");

    Serial.println();
    Serial.print("Potentiometer: ");
    Serial.print(readData(DataIndex[5]));
    Serial.print("\t\t");

    Serial.print("pitch: ");
    Serial.print(readData(DataIndex[6]));
    Serial.print("\t");

    Serial.print("roll: ");
    Serial.print(readData(DataIndex[7]));

    Serial.println();
    Serial.println("---------------------------------------------------------------------------------------------");
  }
*/

void setup() {
  Serial.begin(9600);
  Serial.println("Los geht's");
  // FlexSensoren initialisiert
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
  // setupHC05();
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
