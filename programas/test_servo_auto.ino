/*
 * Test automatico del servo DS04-NFC
 * 
 * Programa para verificar rapidamente que el servo funciona
 * correctamente tras montar el cableado.
 * 
 * Secuencia:
 *   1. Espera 2 segundos (parado)
 *   2. Cierra durante 1.5 segundos
 *   3. Para 2 segundos
 *   4. Abre durante 1.5 segundos
 *   5. Para y fin
 * 
 * Antes de usar este programa, ejecutar punto_muerto_servo.ino
 * para encontrar el valor de SERVO_PARADO_US de tu servo.
 * 
 * Conexiones:
 *   Servo senal  -> Pin 9
 *   Servo VCC    -> Pilas externas 6V (o 5V Arduino si es SG90)
 *   Servo GND    -> GND comun
 */

#include <Servo.h>

Servo servo;
const int SERVO_PARADO_US = 1540;  // ajustar al punto muerto de tu servo
const int SERVO_CERRAR_US = 2000;
const int SERVO_ABRIR_US  = 1000;
const int TIEMPO_MS       = 1500;  // duracion de cada movimiento

void setup() {
  Serial.begin(115200);
  servo.attach(9);
  servo.writeMicroseconds(SERVO_PARADO_US);
  delay(2000);

  Serial.println("Cerrando...");
  servo.writeMicroseconds(SERVO_CERRAR_US);
  delay(TIEMPO_MS);

  servo.writeMicroseconds(SERVO_PARADO_US);
  delay(2000);

  Serial.println("Abriendo...");
  servo.writeMicroseconds(SERVO_ABRIR_US);
  delay(TIEMPO_MS);

  servo.writeMicroseconds(SERVO_PARADO_US);
  Serial.println("Fin");
  while(true);
}

void loop() {}
