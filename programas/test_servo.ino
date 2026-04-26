/*
 * Test del servo DS04-NFC
 * 
 * Programa para probar el servo independientemente del sensor EMG.
 * Acepta comandos por Monitor Serie:
 *   c -> cierra durante TIEMPO_MS y para
 *   a -> abre durante TIEMPO_MS y para
 * 
 * Conexiones:
 *   Servo senal  -> Pin 9
 *   Servo VCC    -> Pilas externas 6V (o 5V Arduino si es SG90)
 *   Servo GND    -> GND comun
 */

#include <Servo.h>

Servo servoMano;
const int SERVO_PIN       = 9;
const int SERVO_PARADO_US = 1540;  // ajustar al punto muerto de tu servo
const int SERVO_CERRAR_US = 2000;
const int SERVO_ABRIR_US  = 1000;
const int TIEMPO_MS       = 1500;

void pararServo() {
  servoMano.writeMicroseconds(SERVO_PARADO_US);
}

void mover(int direccion_us) {
  servoMano.writeMicroseconds(direccion_us);
  delay(TIEMPO_MS);
  pararServo();
}

void setup() {
  Serial.begin(115200);
  servoMano.attach(SERVO_PIN);
  pararServo();
  delay(1000);

  Serial.println("=== Test DS04-NFC ===");
  Serial.println("c -> cerrar 1.5s y para");
  Serial.println("a -> abrir 1.5s y para");
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'c') {
      Serial.println("CERRANDO...");
      mover(SERVO_CERRAR_US);
      Serial.println("PARADO");
    } else if (cmd == 'a') {
      Serial.println("ABRIENDO...");
      mover(SERVO_ABRIR_US);
      Serial.println("PARADO");
    }
  }
}
