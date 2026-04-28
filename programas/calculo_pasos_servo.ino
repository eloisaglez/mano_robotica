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