/*
 * Mano robotica — Modo autonomo (sin ordenador, sin LEDs)
 * 
 * Arranca automaticamente con valores guardados.
 * No requiere Monitor Serie ni ordenador.
 * Ideal para el dia del concurso.
 * 
 * Instrucciones:
 * 1. Calibrar previamente con calibracion_emg.ino
 * 2. Copiar los valores de REPOSO y CONTRACCION aqui abajo
 * 3. Subir el programa
 * 4. Desconectar del ordenador y alimentar con pilas
 * 5. Esperar 5 segundos y la mano esta lista
 * 
 * Conexiones:
 *   MyoWare SIG -> A0
 *   MyoWare VCC -> 5V Arduino
 *   MyoWare GND -> GND Arduino
 *   Servo senal -> Pin 9
 *   Servo VCC   -> Pilas externas 6V
 *   Servo GND   -> GND comun (Arduino + Pilas)
 */

#include <Servo.h>

const int EMG_PIN   = A0;
const int SERVO_PIN = 9;

// ── Valores de calibracion — CAMBIAR POR LOS TUYOS ──
const int REPOSO      = 146;
const int CONTRACCION = 642;
// ─────────────────────────────────────────────────────

const int UMBRAL_BAJO = REPOSO + 0.08 * (CONTRACCION - REPOSO);
const int UMBRAL_ALTO = REPOSO + 0.60 * (CONTRACCION - REPOSO);

// ── Servo DS04-NFC ────────────────────────────────────
Servo servoMano;
const int SERVO_PARADO_US = 1540;  // punto muerto con pilas 6V
const int SERVO_CERRAR_US = 2000;
const int SERVO_ABRIR_US  = 1000;
const int TIEMPO_MS       = 1500;
const int ESPERA_CERRADA  = 2000;
const int LECTURAS_REPOSO = 5;

int estado = 0;
unsigned long tiempoInicio = 0;
int contadorReposo = 0;

void pararServo() {
  servoMano.writeMicroseconds(SERVO_PARADO_US);
}

int leerEMG() {
  long suma = 0;
  for (int i = 0; i < 10; i++) { suma += analogRead(EMG_PIN); delay(2); }
  return suma / 10;
}

void setup() {
  servoMano.attach(SERVO_PIN);
  pararServo();

  // Cuenta atras de 5 segundos para colocarse
  delay(5000);
}

void loop() {
  int emg = leerEMG();

  // ── Sensor desconectado → para todo ───────────────
  if (emg < 15) {
    pararServo();
    delay(100);
    servoMano.detach();
    estado = 0;
    contadorReposo = 0;
    while (leerEMG() < 15) delay(200);
    servoMano.attach(SERVO_PIN);
    pararServo();
    return;
  }

  // ── Sensor saturado → para todo ───────────────────
  if (emg > 900) {
    pararServo();
    estado = 0;
    contadorReposo = 0;
    delay(500);
    return;
  }

  // ── Maquina de estados ────────────────────────────
  switch (estado) {

    case 0:  // espera reposo inicial
      if (emg < UMBRAL_BAJO) {
        contadorReposo++;
        if (contadorReposo >= LECTURAS_REPOSO) {
          estado = 1;
          contadorReposo = 0;
        }
      } else {
        contadorReposo = 0;
      }
      break;

    case 1:  // espera contraccion
      if (emg > UMBRAL_ALTO) {
        servoMano.writeMicroseconds(SERVO_CERRAR_US);
        tiempoInicio = millis();
        estado = 2;
        contadorReposo = 0;
      }
      break;

    case 2:  // cerrando durante TIEMPO_MS
      if (millis() - tiempoInicio > TIEMPO_MS) {
        pararServo();
        tiempoInicio = millis();
        estado = 3;
        contadorReposo = 0;
      }
      break;

    case 3:  // espera relajar
      if (millis() - tiempoInicio < ESPERA_CERRADA) break;
      if (emg < UMBRAL_BAJO) {
        contadorReposo++;
        if (contadorReposo >= LECTURAS_REPOSO) {
          servoMano.writeMicroseconds(SERVO_ABRIR_US);
          tiempoInicio = millis();
          estado = 4;
          contadorReposo = 0;
        }
      } else {
        contadorReposo = 0;
      }
      break;

    case 4:  // abriendo durante TIEMPO_MS
      if (millis() - tiempoInicio > TIEMPO_MS) {
        pararServo();
        estado = 1;
        contadorReposo = 0;
      }
      break;
  }

  delay(20);
}
