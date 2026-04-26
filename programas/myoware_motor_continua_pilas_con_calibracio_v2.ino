#include <Servo.h>

const int EMG_PIN    = A0;
const int SERVO_PIN  = 9;

// ── Valores por defecto — se usan si no calibras ─────
int REPOSO      = 146;
int CONTRACCION = 642;
int UMBRAL_BAJO = 0;
int UMBRAL_ALTO = 0;

// ── Servo DS04-NFC con pilas 6V ───────────────────────
Servo servoMano;
const int SERVO_PARADO_US = 1530;
const int SERVO_CERRAR_US = 2000;
const int SERVO_ABRIR_US  = 2000;
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

bool calibrar() {
  int valoresReposo[N_CALIBRACION];
  int valoresContraccion[N_CALIBRACION];

  // ── Chequeo sensor ────────────────────────────────
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

  // ── Reposo ────────────────────────────────────────
  for (int i = 0; i < N_CALIBRACION; i++) {
    contarAtras(3, "RELAJA: ");
    valoresReposo[i] = medirFase();
    Serial.print("  Reposo "); Serial.print(i + 1); Serial.print("/");
    Serial.print(N_CALIBRACION); Serial.print(": ");
    Serial.println(valoresReposo[i]);
    delay(500);
  }

  Serial.println();

  // ── Contraccion con ENTER ─────────────────────────
  for (int i = 0; i < N_CALIBRACION; i++) {
    Serial.print("Contrac "); Serial.print(i + 1); Serial.print("/");
    Serial.print(N_CALIBRACION);
    Serial.println(" — CONTRAE y pulsa ENTER:");
    esperarEnter();
    valoresContraccion[i] = medirFase();
    Serial.print("  Valor: "); Serial.println(valoresContraccion[i]);
    delay(500);
  }

  // ── Calculo ───────────────────────────────────────
  REPOSO = mediaRobusta(valoresReposo, N_CALIBRACION);
  CONTRACCION = mediaRobusta(valoresContraccion, N_CALIBRACION);
  int diferencia = CONTRACCION - REPOSO;

  Serial.println("\n--- CALIBRACION ---");
  Serial.print("Reposo:      "); Serial.println(REPOSO);
  Serial.print("Contraccion: "); Serial.println(CONTRACCION);
  Serial.print("Diferencia:  "); Serial.println(diferencia);

  if (diferencia < 50) {
    Serial.println("ERROR: Diferencia nula -> cargador, electrodos o sensor");
    return false;
  } else if (diferencia < 150) {
    Serial.println("AVISO: Diferencia justa -> recoloca sensor o cambia electrodos");
  } else if (diferencia < 250) {
    Serial.println("OK: Aceptable");
  } else {
    Serial.println("OK: Calibracion correcta");
  }

  return true;
}

void calcularUmbrales() {
  int diferencia = CONTRACCION - REPOSO;
  UMBRAL_BAJO = REPOSO + 0.08 * diferencia;
  UMBRAL_ALTO = REPOSO + 0.60 * diferencia;
  Serial.print("Umbral abrir:  "); Serial.println(UMBRAL_BAJO);
  Serial.print("Umbral cerrar: "); Serial.println(UMBRAL_ALTO);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  servoMano.attach(SERVO_PIN);
  pararServo();
  delay(1000);

  Serial.println("=== Mano robotica ===\n");

  // ── Preguntar si calibrar ─────────────────────────
  Serial.print("Valores guardados: reposo="); Serial.print(REPOSO);
  Serial.print("  contraccion="); Serial.println(CONTRACCION);
  Serial.println("c → calibrar");
  Serial.println("s → usar valores guardados");

  char opcion = esperarTecla();

  if (opcion == 'c') {
    Serial.println("\nIniciando calibracion...\n");
    while (!calibrar()) {
      Serial.println("\nCalibracion fallida — reintentando en 3 segundos...\n");
      delay(3000);
    }
  } else {
    Serial.println("\nUsando valores guardados");
  }

  calcularUmbrales();

  // ── Cuenta atras para prepararse ──────────────────
  Serial.print("Arrancando en: ");
  for (int i = 5; i > 0; i--) {
    Serial.print(i); Serial.print("... ");
    Serial.flush();
    delay(1000);
  }
  Serial.println("\nRelaja el musculo...");
}

void loop() {
  int emg = leerEMG();

  // ── Sensor desconectado → para todo y detach ──────
  if (emg < 15) {
    pararServo();
    delay(100);
    servoMano.detach();
    estado = 0;
    contadorReposo = 0;
    Serial.println("AVISO: Sensor desconectado — revisa cables");
    delay(500);
    while (leerEMG() < 15) delay(200);
    servoMano.attach(SERVO_PIN);
    pararServo();
    Serial.println("Sensor reconectado — relaja el musculo...");
    return;
  }

  // ── Sensor saturado → avisa ───────────────────────
  if (emg > 900) {
    pararServo();
    estado = 0;
    contadorReposo = 0;
    Serial.println("AVISO: Sensor saturado — desconecta cargador o baja GAIN");
    delay(500);
    return;
  }
  // ─────────────────────────────────────────────────

  switch (estado) {

    case 0:
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

    case 1:
      if (emg > UMBRAL_ALTO) {
        servoMano.writeMicroseconds(SERVO_CERRAR_US);
        tiempoInicio = millis();
        estado = 2;
        contadorReposo = 0;
        Serial.println("CERRANDO...");
      }
      break;

    case 2:
      if (millis() - tiempoInicio > TIEMPO_MS) {
        pararServo();
        tiempoInicio = millis();
        estado = 3;
        contadorReposo = 0;
        Serial.println("CERRADA — relaja para abrir");
      }
      break;

    case 3:
      if (millis() - tiempoInicio < ESPERA_CERRADA) break;
      if (emg < UMBRAL_BAJO) {
        contadorReposo++;
        if (contadorReposo >= LECTURAS_REPOSO) {
          servoMano.writeMicroseconds(SERVO_ABRIR_US);
          tiempoInicio = millis();
          estado = 4;
          contadorReposo = 0;
          Serial.println("ABRIENDO...");
        }
      } else {
        contadorReposo = 0;
      }
      break;

    case 4:
      if (millis() - tiempoInicio > TIEMPO_MS) {
        pararServo();
        estado = 1;
        contadorReposo = 0;
        Serial.println("ABIERTA — contrae para cerrar");
      }
      break;
  }

  Serial.print("EMG: "); Serial.print(emg);
  Serial.print("  Estado: "); Serial.println(estado);
  delay(20);
}