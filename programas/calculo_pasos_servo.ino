/*
 * Calibracion de pasos del motor 28BYJ-48 + ULN2003
 * 
 * Programa interactivo para encontrar el numero exacto de pasos
 * que necesita el motor para cerrar la pinza completamente.
 * 
 * Comandos por Monitor Serie (9600 baud):
 *   c → cierra (mueve al valor estimado + ajuste)
 *   a → abre (vuelve a posicion 0)
 *   + → añade 20 pasos al ajuste
 *   - → quita 20 pasos al ajuste
 *   k → guarda el valor final de pasos maximos
 * 
 * Uso:
 *   1. Pulsa c — si no cierra del todo pulsa + varias veces
 *   2. Pulsa a — abre
 *   3. Pulsa c — cierra con el nuevo valor
 *   4. Repite hasta que cierre justo
 *   5. Pulsa k — muestra PASOS_MAXIMOS
 *   6. Copia ese valor al programa de la mano: mano_robotica_stepper.ino
 * 
 * Conexiones:
 *   Arduino pin 8  -> ULN2003 IN1
 *   Arduino pin 9  -> ULN2003 IN2
 *   Arduino pin 10 -> ULN2003 IN3
 *   Arduino pin 11 -> ULN2003 IN4
 *   Fuente 5V      -> ULN2003 VCC (NO usar 5V del Arduino)
 *   GND comun       -> ULN2003 GND + Arduino GND
 */

#include <Arduino.h>

const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

int delayPaso = 5;
int pasoActual = 0;

int posicion = 0;
int direccion = 1;

unsigned long ultimoPaso = 0;

int objetivo = 0;

// valor buscado
int pasosMaximos = 0;
int ajuste = 0;

void activarBobinas(int a, int b, int c, int d) {
  digitalWrite(IN1, a);
  digitalWrite(IN2, b);
  digitalWrite(IN3, c);
  digitalWrite(IN4, d);
}

void moverUnPaso() {
  switch (pasoActual) {
    case 0: activarBobinas(1,0,0,0); break;
    case 1: activarBobinas(0,1,0,0); break;
    case 2: activarBobinas(0,0,1,0); break;
    case 3: activarBobinas(0,0,0,1); break;
  }

  pasoActual += direccion;

  if (pasoActual > 3) pasoActual = 0;
  if (pasoActual < 0) pasoActual = 3;

  posicion += direccion;
}

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  Serial.begin(9600);

  Serial.println("CALIBRACION LISTA");
  Serial.println("c = cerrar");
  Serial.println("a = abrir");
  Serial.println("+ / - = ajustar");
  Serial.println("k = guardar valor");
}

void loop() {

  // movimiento básico
  if (posicion != objetivo && millis() - ultimoPaso >= delayPaso) {
    ultimoPaso = millis();

    if (objetivo > posicion) direccion = 1;
    else direccion = -1;

    moverUnPaso();
  }

  // comandos
  if (Serial.available()) {
    char c = Serial.read();

    if (c == 'c') {
      objetivo = 1200 + ajuste; // valor inicial estimado, ajustar al tamaño del tambor
      Serial.println("Cerrando...");
    }

    if (c == 'a') {
      objetivo = 0;
      Serial.println("Abriendo...");
    }

    if (c == '+') {
      ajuste += 20;
      Serial.print("Ajuste: ");
      Serial.println(ajuste);
    }

    if (c == '-') {
      ajuste -= 20;
      Serial.print("Ajuste: ");
      Serial.println(ajuste);
    }

    if (c == 'k') {
      pasosMaximos = 1200 + ajuste;

      Serial.println("===== GUARDADO =====");
      Serial.print("Pasos maximos = ");
      Serial.println(pasosMaximos);
    }
  }
}
