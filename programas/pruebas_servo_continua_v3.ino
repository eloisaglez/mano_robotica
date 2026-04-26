#include <Servo.h>

Servo servoMano;
const int SERVO_PIN       = 9;
const int SERVO_PARADO_US = 1530;
const int SERVO_CERRAR    = 180;
const int SERVO_ABRIR     = 0;
const int TIEMPO_MS       = 1500;

void pararServo() {
  servoMano.writeMicroseconds(SERVO_PARADO_US);
}

void mover(int direccion) {
  servoMano.write(direccion);
  delay(TIEMPO_MS);
  pararServo();
}

void setup() {
  Serial.begin(115200);
  servoMano.attach(SERVO_PIN);
  pararServo();
  delay(1000);

  Serial.println("=== Test DS04-NFC ===");
  Serial.println("c → cerrar 1.5s y para");
  Serial.println("a → abrir 1.5s y para");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'c') {
      Serial.println("CERRANDO...");
      mover(SERVO_CERRAR);
      Serial.println("PARADO");
    } else if (cmd == 'a') {
      Serial.println("ABRIENDO...");
      mover(SERVO_ABRIR);
      Serial.println("PARADO");
    }
  }
}