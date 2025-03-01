#include <SoftwareSerial.h>
#include <Servo.h>

const int delayTime = 200;

// Bluethooth Komponente START
const uint8_t rxPORT = 10;
const uint8_t txPORT = 11;

SoftwareSerial HC05(rxPORT, txPORT);

void setupHC05() {
  HC05.begin(115200);
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

static const uint8_t DataIndex[] = {14, 15, 16, 17, 20, 88, 89};
//static const uint8_t DataIndex[] = {17};

void recieveData() {
  for (int index : DataIndex) {
    HC05.write(index);
    HC05.flush();
    if (HC05.available()) {
      int data = HC05.read();
      move(index, data);
    }
    delay(delayTime/2);
  }
}

// Ansprechung von Servoobjekten
struct Hand {
  Servo pinky;
  Servo ring;
  Servo middle;
  Servo pointer;
  Servo thumb;

  struct WinkelData {
    Servo pitch;  //X-Achse
    Servo roll_1;   //Y-Achse
    Servo roll_2;   //Y-Achse
  } Drehung;
} Hand;

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
      if (data > 180) { data = 180; }
      Hand.pinky.write(data);
      break;
    case 15:
      if (data > 180) { data = 180; }
      // müssen aufgrund von aufbau gespiegelt angesteuert werden
      data = 180 - data;
      Hand.ring.write(data);
      break;
    case 16:
      if (data > 180) { data = 180; }
      // müssen aufgrund von aufbau gespiegelt angesteuert werden
      data = 180 - data;
      Hand.middle.write(data);
      break;
    case 17:
      if (data > 180) { data = 180; }
      Hand.pointer.write(data);
      break;
    case 20:
      if (data > 180) { data = 180; }
      Hand.thumb.write(data);
      break;
    case 88:
      data = map(data, 0, 255, 0, 180);
      Hand.Drehung.pitch.write(data);
      break;
    case 89:
      data = map(data, 0, 255, 0, 180);
      Hand.Drehung.roll_2.write(data);
      Hand.Drehung.roll_1.write(180-data);
      break;
    default:
      move(index, data);
  }
}

// Bluethooth Komponente ENDE

void setupHand() {
  // Servos verbinden
  Hand.pinky.attach(2);
  Hand.ring.attach(3);
  Hand.middle.attach(4);
  Hand.pointer.attach(5);
  Hand.thumb.attach(6);
  Hand.Drehung.pitch.attach(7);
  Hand.Drehung.roll_1.attach(8);
  Hand.Drehung.roll_2.attach(9);
  // Servos auf Nullposition bringen
  Hand.pinky.write(0);
  Hand.ring.write(0);
  Hand.middle.write(0);
  Hand.pointer.write(0);
  Hand.thumb.write(0);
  Hand.Drehung.pitch.write(90);
  Hand.Drehung.roll_1.write(90);   
  Hand.Drehung.roll_2.write(90);  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Los geht's");
  setupHand();
  setupHC05();
}

void loop() {
  // float t1 = millis();
  if (!HC05.overflow()) {
    recieveData();
  }


  // float t2 = millis();
  // float td = t2 - t1;
  // Serial.print("td: ");
  // Serial.println(td);
  delay(delayTime);
}
