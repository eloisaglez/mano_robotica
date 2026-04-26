/*
 * Mano robotica — Arranque con boton (sin ordenador, sin LEDs)
 * 
 * La mano no hace nada hasta que se pulsa el boton.
 * Al pulsar arranca con valores guardados.
 * Si se pulsa durante mas de 3 segundos entra en modo calibracion.
 * 
 * Modo normal (pulsacion corta):
 *   Pulsar boton -> espera 5s -> usa valores guardados -> listo
 * 
 * Modo calibracion (pulsacion larga > 3s):
 *   Pulsar boton 3s -> calibra reposo 5 veces (automatico)
 *                   -> calibra contraccion 5 veces (pulsar boton cada vez)
 *                   -> arranca con valores nuevos
 * 
 * Instrucciones:
 * 1. Subir el programa
 * 2. Conectar pilas
 * 3. Pulsar boton para arrancar
 * 
 * Conexiones:
 *   MyoWare SIG  -> A0
 *   MyoWare VCC  -> 5V Arduino
 *   MyoWare GND  -> GND Arduino
 *   Servo senal  -> Pin 9
 *   Servo VCC    -> Pilas externas 6V
 *   Servo GND    -> GND comun (Arduino + Pilas)
 *   Boton        -> Pin 2 y GND (INPUT_PULLUP, sin resistencia)
 */

#include <Servo.h>

const int EMG_PIN    = A0;
const int SERVO_PIN  = 9;
const int BOTON_PIN  = 2;

// ── Valores de calibracion — CAMBIAR POR LOS TUYOS ──
int REPOSO      = 146;
int CONTRACCION = 642;
// ─────────────────────────────────────────────────────

int UMBRAL_BAJO = 0;
int UMBRAL_ALTO = 0;

// ── Servo DS04-NFC ────────────────────────────────────
Servo servoMano;
const int SERVO_PARADO_US = 1540;
const int SERVO_CERRAR_US = 2000;
const int SERVO_ABRIR_US  = 1000;
const int TIEMPO_MS       = 1500;
const int ESPERA_CERRADA  = 2000;
const int LECTURAS_REPOSO = 5;
const int N_CALIBRACION   = 5;

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

int medirFase() {
  long suma = 0;
  for (int i = 0; i < 200; i++) { suma += analogRead(EMG_PIN); delay(5); }
  return suma / 200;
}

int mediaRobusta(int valores[], int n) {
  int minVal = valores[0], maxVal = valores[0];
  long suma = 0;
  for (int i = 0; i < n; i++) {
    suma += valores[i];
    if (valores[i] < minVal) minVal = valores[i];
    if (valores[i] > maxVal) maxVal = valores[i];
  }
  if (n > 2) {
    suma -= minVal;
    suma -= maxVal;
    return suma / (n - 2);
  }
  return suma / n;
}

void calcularUmbrales() {
  int diferencia = CONTRACCION - REPOSO;
  UMBRAL_BAJO = REPOSO + 0.08 * diferencia;
  UMBRAL_ALTO = REPOSO + 0.60 * diferencia;
}

bool botonPulsado() {
  return digitalRead(BOTON_PIN) == LOW;
}

void esperarBoton() {
  while (!botonPulsado()) delay(10);
  delay(50);  // debounce
  while (botonPulsado()) delay(10);
  delay(50);
}

void calibrarConBoton() {
  int valoresReposo[N_CALIBRACION];
  int valoresContraccion[N_CALIBRACION];

  // ── Reposo: 5 mediciones automaticas con pausa ────
  // Relajar el musculo — mide automaticamente cada 3 segundos
  for (int i = 0; i < N_CALIBRACION; i++) {
    delay(3000);
    valoresReposo[i] = medirFase();
  }

  // ── Contraccion: 5 mediciones con boton ───────────
  // Contraer el musculo y pulsar boton para cada medicion
  delay(2000);  // pausa entre fases

  for (int i = 0; i < N_CALIBRACION; i++) {
    esperarBoton();
    valoresContraccion[i] = medirFase();
    delay(500);
  }

  // ── Calcular valores ──────────────────────────────
  REPOSO = mediaRobusta(valoresReposo, N_CALIBRACION);
  CONTRACCION = mediaRobusta(valoresContraccion, N_CALIBRACION);
}

void setup() {
  pinMode(BOTON_PIN, INPUT_PULLUP);
  servoMano.attach(SERVO_PIN);
  pararServo();
  delay(500);

  // ── Esperar pulsacion del boton ───────────────────
  // Medir cuanto tiempo se mantiene pulsado
  while (!botonPulsado()) delay(10);

  unsigned long pulsacionInicio = millis();
  while (botonPulsado()) delay(10);
  unsigned long duracion = millis() - pulsacionInicio;

  if (duracion > 3000) {
    // ── Pulsacion larga: calibrar ───────────────────
    calibrarConBoton();
  }
  // Si pulsacion corta: usa valores guardados

  calcularUmbrales();

  // Cuenta atras de 5 segundos
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
