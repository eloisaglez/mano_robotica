#include <Servo.h>
Servo servo;

void setup() {
  Serial.begin(115200);
  servo.attach(9);
  servo.writeMicroseconds(1540);  // parado (ejecutar 1º bucar_parada_servo_tiempo para encontrar el valor de punto puerto)
  delay(2000);

  Serial.println("Cerrando...");
  servo.writeMicroseconds(2000);  // máxima velocidad un sentido
  delay(1500);

  servo.writeMicroseconds(1540);  // para
  delay(2000);

  Serial.println("Abriendo...");
  servo.writeMicroseconds(1000);  // máxima velocidad otro sentido
  delay(1500);

  servo.writeMicroseconds(1540);  // para
  Serial.println("Fin");
  while(true);
}

void loop() {}
