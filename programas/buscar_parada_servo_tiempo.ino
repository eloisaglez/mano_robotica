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
 * 4. El valor donde el servo se quede quieto es tu SERVO_PARADO_US
 * 5. Usa ese valor en el programa de la mano robotica
 *
 * Ejemplo
 * 1520 µs → parado (valor encontrado)
 *  > 1520 µs → gira en un sentido
 *  < 1520 µs → gira en el otro sentido
 * Si le mandas 1530µs en lugar de 1520µs, le estás diciendo "gira despacio" sin querer 
 */

#include <Servo.h>
Servo servo;

void setup() {
  servo.attach(9);
  servo.writeMicroseconds(1530);  // cambia este valor hasta que pare
}

void loop() {}