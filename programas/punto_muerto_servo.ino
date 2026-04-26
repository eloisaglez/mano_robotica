/*
 * Buscar el punto muerto del servo de rotacion continua DS04-NFC
 * 
 * El punto muerto es el ancho de pulso PWM en microsegundos
 * que hace que el servo se quede completamente parado.
 * 
 * Instrucciones:
 * 1. Sube este programa
 * 2. Si el servo gira, cambia el valor de writeMicroseconds
 * 3. Prueba de 5 en 5: 1500, 1510, 1520, 1530, 1540...
 * 4. Rango habitual: 1450 a 1550 us
 * 5. Con USB (5V) suele ser ~1520 us
 * 6. Con pilas (6V) suele ser ~1540 us
 * 7. El valor donde el servo se quede quieto es tu SERVO_PARADO_US
 * 8. Usa ese valor en el programa de la mano robotica
 */

#include <Servo.h>
Servo servo;

void setup() {
  servo.attach(9);
  servo.writeMicroseconds(1540);  // cambia este valor hasta que pare
}

void loop() {}
