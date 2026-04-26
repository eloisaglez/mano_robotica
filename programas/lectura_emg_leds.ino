/*
 * Lectura del sensor MyoWare v1 con indicacion por LEDs
 * 
 * Programa para verificar el funcionamiento del sensor
 * SIN necesidad de ordenador. Los LEDs indican el estado.
 * 
 * LEDs:
 *   LED verde  -> encendido cuando el musculo esta relajado
 *   LED rojo   -> encendido cuando el musculo esta contraido
 *   Ambos parpadean rapido -> sensor desconectado o saturado
 * 
 * Tambien envia datos por Monitor Serie si hay ordenador conectado.
 * 
 * Conexiones:
 *   MyoWare SIG -> A0
 *   MyoWare VCC -> 5V Arduino
 *   MyoWare GND -> GND Arduino
 *   LED verde   -> Pin 3 (con resistencia 220 ohm a GND)
 *   LED rojo    -> Pin 4 (con resistencia 220 ohm a GND)
 * 
 * Esquema LEDs:
 *   Pin 3 ---[220 ohm]--- LED verde (+) --- GND
 *   Pin 4 ---[220 ohm]--- LED rojo  (+) --- GND
 * 
 * Umbrales:
 *   Se usan valores por defecto que se pueden cambiar.
 *   Para obtener valores exactos usar calibracion_emg.ino primero.
 */

const int EMG_PIN   = A0;
const int LED_VERDE = 3;
const int LED_ROJO  = 4;

// ── Valores de calibracion — cambiar por los tuyos ───
int REPOSO      = 146;
int CONTRACCION = 642;

// ── Umbrales calculados ──────────────────────────────
int UMBRAL_BAJO = 0;
int UMBRAL_ALTO = 0;

int leerEMG() {
  long suma = 0;
  for (int i = 0; i < 10; i++) { suma += analogRead(EMG_PIN); delay(2); }
  return suma / 10;
}

void parpadearError() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROJO, HIGH);
    delay(100);
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);
    delay(100);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  delay(1000);

  Serial.println("=== MyoWare v1 con LEDs ===\n");

  // Calcular umbrales
  int diferencia = CONTRACCION - REPOSO;
  UMBRAL_BAJO = REPOSO + 0.08 * diferencia;
  UMBRAL_ALTO = REPOSO + 0.60 * diferencia;

  Serial.print("Reposo:        "); Serial.println(REPOSO);
  Serial.print("Contraccion:   "); Serial.println(CONTRACCION);
  Serial.print("Umbral abrir:  "); Serial.println(UMBRAL_BAJO);
  Serial.print("Umbral cerrar: "); Serial.println(UMBRAL_ALTO);
  Serial.println();

  // Chequeo sensor
  long suma = 0;
  for (int i = 0; i < 20; i++) { suma += analogRead(EMG_PIN); delay(10); }
  int valor = suma / 20;

  if (valor < 15) {
    Serial.println("ERROR: Sensor apagado -> Revisa VCC 5V y GND");
    while (true) parpadearError();
  } else if (valor > 900) {
    Serial.println("ERROR: Saturado -> Desconecta cargador o baja GAIN");
    while (true) parpadearError();
  }

  Serial.print("Sensor OK: "); Serial.println(valor);
  Serial.println("Listo — observa los LEDs\n");

  // Indicar que esta listo
  digitalWrite(LED_VERDE, HIGH);
  delay(500);
  digitalWrite(LED_VERDE, LOW);
  delay(200);
  digitalWrite(LED_VERDE, HIGH);
  delay(500);
  digitalWrite(LED_VERDE, LOW);
  delay(500);
}

void loop() {
  int emg = leerEMG();

  // ── Sensor desconectado ───────────────────────────
  if (emg < 15) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);
    parpadearError();
    Serial.println("AVISO: Sensor desconectado");
    return;
  }

  // ── Sensor saturado ───────────────────────────────
  if (emg > 900) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);
    parpadearError();
    Serial.println("AVISO: Sensor saturado");
    return;
  }

  // ── Indicacion por LEDs ───────────────────────────
  if (emg > UMBRAL_ALTO) {
    // Contraido
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, HIGH);
  } else if (emg < UMBRAL_BAJO) {
    // Relajado
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_ROJO, LOW);
  } else {
    // Zona intermedia (histeresis) — ambos apagados
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_ROJO, LOW);
  }

  Serial.print("EMG: "); Serial.print(emg);
  if (emg > UMBRAL_ALTO) Serial.println("  CONTRAIDO");
  else if (emg < UMBRAL_BAJO) Serial.println("  RELAJADO");
  else Serial.println("  ---");

  delay(20);
}
