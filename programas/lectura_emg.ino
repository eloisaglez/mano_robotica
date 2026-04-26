/*
 * Lectura simple del sensor MyoWare v1 (sin calibracion)
 * 
 * Programa basico para ver en tiempo real los valores
 * del sensor EMG por el Monitor Serie.
 * Util para:
 *   - Verificar que el sensor funciona
 *   - Comprobar la colocacion de los electrodos
 *   - Observar los valores de reposo y contraccion
 *   - Ajustar la ganancia del potenciometro GAIN
 * 
 * Instrucciones:
 * 1. Conectar MyoWare VCC a 5V, GND a GND, SIG a A0
 * 2. Desconectar el cargador del portatil
 * 3. Colocar electrodos: M y E en antebrazo, REF en codo
 * 4. Subir este programa y abrir Monitor Serie a 115200
 * 5. Observar los valores en reposo y al contraer
 * 
 * Conexiones:
 *   MyoWare SIG -> A0
 *   MyoWare VCC -> 5V Arduino
 *   MyoWare GND -> GND Arduino
 */

const int EMG_PIN = A0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Lectura MyoWare v1 ===\n");

  // Chequeo sensor
  long suma = 0;
  for (int i = 0; i < 20; i++) { suma += analogRead(EMG_PIN); delay(10); }
  int valor = suma / 20;

  if (valor < 15) {
    Serial.println("ERROR: Sensor apagado -> Revisa VCC 5V y GND");
    while (true);
  } else if (valor > 900) {
    Serial.println("ERROR: Saturado -> Desconecta cargador o baja GAIN");
    while (true);
  }
  Serial.print("Sensor OK: "); Serial.println(valor);
  Serial.println("Contrae y relaja el musculo para ver los valores\n");
}

void loop() {
  long suma = 0;
  for (int i = 0; i < 10; i++) { suma += analogRead(EMG_PIN); delay(2); }
  int emg = suma / 10;

  Serial.print("EMG: "); Serial.println(emg);
  delay(50);
}
