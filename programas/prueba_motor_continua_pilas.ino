#include <Servo.h>
Servo servo;

void setup() {
  Serial.begin(115200);
  servo.attach(9);
  servo.writeMicroseconds(1540);  // parado
  delay(2000);

  Serial.println("Cerrando...");
  servo.writeMicroseconds(2000);  // maxima velocidad un sentido
  delay(1500);

  servo.writeMicroseconds(1540);  // para
  delay(2000);

  Serial.println("Abriendo...");
  servo.writeMicroseconds(1000);  // maxima velocidad otro sentido
  delay(1500);

  servo.writeMicroseconds(1540);  // para
  Serial.println("Fin");
  while(true);
}

void loop() {}