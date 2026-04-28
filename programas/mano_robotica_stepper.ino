/*
 * Mano robótica con control EMG y motor paso a paso
 * 
 * Control de pinza de 3 dedos mediante sensor MyoWare v1
 * y motor paso a paso (unipolar o bipolar, secuencia 4 pasos).
 * 
 * Al arrancar ofrece:
 *   c -> calibrar EMG (5 mediciones reposo + 5 contracción)
 *   s -> usar valores guardados de EMG
 * 
 * Conexiones:
 *   MyoWare SIG -> A0
 *   MyoWare VCC -> 5V Arduino
 *   MyoWare GND -> GND Arduino
 *   Motor paso a paso:
 *     IN1 -> pin 8
 *     IN2 -> pin 9
 *     IN3 -> pin 10
 *     IN4 -> pin 11
 *     Alimentación externa para el motor (GND común con Arduino)
 * 
 * NOTA: Los pasos máximos para cerrar completamente la pinza
 *       se definen en la constante PASOS_MAXIMOS (ajustar según calibración).
 */

#include <Arduino.h>

// ==================== PINES MOTOR PASO A PASO ====================
const int IN1 = 8;
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

const int DELAY_PASO = 5;      // ms entre pasos (velocidad de movimiento)
int pasoActual = 0;
int posicion = 0;
int direccion = 1;
unsigned long ultimoPaso = 0;
int objetivo = 0;

// Valor fijo de pasos para cerrar completamente la pinza
// AJUSTAR ESTE NÚMERO SEGÚN CALIBRACIÓN (obtenido con el programa cálculo pasos servo)
const int PASOS_MAXIMOS = 1200;   // <-- CAMBIAR VALOR AQUÍ

// ==================== EMG ====================
const int EMG_PIN = A0;

// Valores por defecto (se pueden sobrescribir con calibración)
int REPOSO      = 146;
int CONTRACCION = 642;
int UMBRAL_BAJO = 0;
int UMBRAL_ALTO = 0;

const int LECTURAS_REPOSO = 5;     // lecturas consecutivas para confirmar reposo
const int N_CALIBRACION   = 5;     // mediciones por fase de calibración

int estado = 0;                     // 0 espera reposo inicial, 1 espera contracción, etc.
unsigned long tiempoInicio = 0;
int contadorReposo = 0;

// ==================== FUNCIONES DEL MOTOR ====================
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

void pararMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ==================== FUNCIONES DE LECTURA EMG ====================
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

void contarAtras(int segundos, const char* mensaje) {
  Serial.print(mensaje);
  Serial.flush();
  for (int i = segundos; i > 0; i--) {
    Serial.print(i); Serial.print("... ");
    Serial.flush();
    delay(1000);
  }
  Serial.println();
}

char esperarTecla() {
  delay(200);
  while (Serial.available()) Serial.read();
  while (!Serial.available());
  char c = Serial.read();
  delay(100);
  while (Serial.available()) Serial.read();
  return c;
}

void esperarEnter() {
  delay(200);
  while (Serial.available()) Serial.read();
  while (!Serial.available());
  delay(100);
  while (Serial.available()) Serial.read();
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

// ==================== CALIBRACIÓN EMG ====================
bool calibrarEMG() {
  int valoresReposo[N_CALIBRACION];
  int valoresContraccion[N_CALIBRACION];

  long suma = 0;
  for (int i = 0; i < 20; i++) { suma += analogRead(EMG_PIN); delay(10); }
  int valor = suma / 20;
  if (valor < 15) {
    Serial.println("ERROR: Sensor apagado -> Revisa VCC 5V y GND");
    return false;
  } else if (valor > 900) {
    Serial.println("ERROR: Saturado -> Desconecta cargador o baja GAIN");
    return false;
  }
  Serial.print("Sensor OK: "); Serial.println(valor);
  Serial.println();

  for (int i = 0; i < N_CALIBRACION; i++) {
    contarAtras(3, "RELAJA: ");
    valoresReposo[i] = medirFase();
    Serial.print("  Reposo "); Serial.print(i+1); Serial.print("/");
    Serial.print(N_CALIBRACION); Serial.print(": ");
    Serial.println(valoresReposo[i]);
    delay(500);
  }
  Serial.println();

  for (int i = 0; i < N_CALIBRACION; i++) {
    Serial.print("Contrac "); Serial.print(i+1); Serial.print("/");
    Serial.print(N_CALIBRACION);
    Serial.println(" — CONTRAE y pulsa ENTER:");
    esperarEnter();
    valoresContraccion[i] = medirFase();
    Serial.print("  Valor: "); Serial.println(valoresContraccion[i]);
    delay(500);
  }

  REPOSO = mediaRobusta(valoresReposo, N_CALIBRACION);
  CONTRACCION = mediaRobusta(valoresContraccion, N_CALIBRACION);
  int diferencia = CONTRACCION - REPOSO;

  Serial.println("\n--- CALIBRACIÓN EMG ---");
  Serial.print("Reposo:      "); Serial.println(REPOSO);
  Serial.print("Contracción: "); Serial.println(CONTRACCION);
  Serial.print("Diferencia:  "); Serial.println(diferencia);

  if (diferencia < 50) {
    Serial.println("ERROR: Diferencia nula -> cargador, electrodos o sensor");
    return false;
  } else if (diferencia < 150) {
    Serial.println("AVISO: Diferencia justa -> recoloca sensor o cambia electrodos");
  } else if (diferencia < 250) {
    Serial.println("OK: Aceptable");
  } else {
    Serial.println("OK: Calibración correcta");
  }
  return true;
}

void calcularUmbrales() {
  int diferencia = CONTRACCION - REPOSO;
  UMBRAL_BAJO = REPOSO + 0.08 * diferencia;
  UMBRAL_ALTO = REPOSO + 0.60 * diferencia;
  Serial.print("Umbral abrir (relajación):  "); Serial.println(UMBRAL_BAJO);
  Serial.print("Umbral cerrar (contracción): "); Serial.println(UMBRAL_ALTO);
  Serial.println();
}

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pararMotor();

  delay(1000);
  Serial.println("=== Mano robótica con motor paso a paso (EMG) ===\n");
  Serial.print("Pasos máximos configurados: ");
  Serial.println(PASOS_MAXIMOS);
  Serial.println();

  Serial.print("Valores EMG guardados: reposo="); Serial.print(REPOSO);
  Serial.print("  contracción="); Serial.println(CONTRACCION);
  Serial.println("c -> calibrar EMG");
  Serial.println("s -> usar valores guardados");

  char opcion = esperarTecla();

  if (opcion == 'c') {
    Serial.println("\nIniciando calibración EMG...\n");
    while (!calibrarEMG()) {
      Serial.println("\nCalibración fallida — reintentando en 3 segundos...\n");
      delay(3000);
    }
  } else {
    Serial.println("\nUsando valores guardados");
  }

  calcularUmbrales();

  Serial.print("Arrancando control EMG en: ");
  for (int i = 5; i > 0; i--) {
    Serial.print(i); Serial.print("... ");
    Serial.flush();
    delay(1000);
  }
  Serial.println("\nRelaja el músculo...");
}

// ==================== LOOP PRINCIPAL ====================
void loop() {
  // Movimiento continuo del motor paso a paso (si no ha llegado al objetivo)
  if (posicion != objetivo && millis() - ultimoPaso >= DELAY_PASO) {
    ultimoPaso = millis();
    if (objetivo > posicion) direccion = 1;
    else direccion = -1;
    moverUnPaso();
  }

  int emg = leerEMG();

  // Sensor desconectado
  if (emg < 15) {
    pararMotor();
    estado = 0;
    contadorReposo = 0;
    Serial.println("AVISO: Sensor desconectado — revisa cables");
    delay(500);
    while (leerEMG() < 15) delay(200);
    Serial.println("Sensor reconectado — relaja el músculo...");
    return;
  }

  // Sensor saturado
  if (emg > 900) {
    pararMotor();
    estado = 0;
    contadorReposo = 0;
    Serial.println("AVISO: Sensor saturado — desconecta cargador o baja GAIN");
    delay(500);
    return;
  }

  // Máquina de estados EMG
  switch (estado) {
    case 0:  // espera reposo inicial
      if (emg < UMBRAL_BAJO) {
        contadorReposo++;
        if (contadorReposo >= LECTURAS_REPOSO) {
          estado = 1;
          contadorReposo = 0;
          Serial.println("Listo — contrae para cerrar");
        }
      } else {
        contadorReposo = 0;
      }
      break;

    case 1:  // espera contracción (cerrar)
      if (emg > UMBRAL_ALTO) {
        objetivo = PASOS_MAXIMOS;
        tiempoInicio = millis();
        estado = 2;
        contadorReposo = 0;
        Serial.println("CERRANDO...");
      }
      break;

    case 2:  // espera a que termine el cierre
      if (posicion == objetivo) {
        pararMotor();
        tiempoInicio = millis();
        estado = 3;
        contadorReposo = 0;
        Serial.println("CERRADA — relaja para abrir");
      }
      break;

    case 3:  // espera relajación con tiempo mínimo y lecturas consecutivas
      if (millis() - tiempoInicio < 2000) break;
      if (emg < UMBRAL_BAJO) {
        contadorReposo++;
        if (contadorReposo >= LECTURAS_REPOSO) {
          objetivo = 0;
          tiempoInicio = millis();
          estado = 4;
          contadorReposo = 0;
          Serial.println("ABRIENDO...");
        }
      } else {
        contadorReposo = 0;
      }
      break;

    case 4:  // espera a que termine la apertura
      if (posicion == 0) {
        pararMotor();
        estado = 1;
        contadorReposo = 0;
        Serial.println("ABIERTA — contrae para cerrar");
      }
      break;
  }

  // Monitorización opcional
  Serial.print("EMG: "); Serial.print(emg);
  Serial.print("  Estado: "); Serial.println(estado);
  delay(20);
}