#include <Servo.h>

Servo servo;

void setup() {
  servo.attach(9, 900, 2100);
  servo.write(93);  // punto muerto calculado
}

void loop() {}