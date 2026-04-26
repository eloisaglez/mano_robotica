/*
 * Calibracion del sensor MyoWare v1
 * 
 * Programa independiente para obtener los valores de reposo
 * y contraccion del sensor EMG.
 * 
 * Instrucciones:
 * 1. Conectar MyoWare VCC a 5V, GND a GND, SIG a A0
 * 2. Desconectar el cargador del portatil
 * 3. Colocar electrodos: M y E en antebrazo, REF en codo
 * 4. Subir este programa y abrir Monitor Serie a 115200
 * 5. Seguir las instrucciones en pantalla
 * 6. Copiar los valores finales al programa de la mano
 */

const int EMG_PIN      = A0;
const int N_LECTURAS   = 5;

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

void esperarEnter() {
  delay(200);
  while (Serial.available()) Serial.read();
  while (!Serial.available());
  delay(100);
  while (Serial.available()) Serial.read();
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

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== Calibracion MyoWare v1 ===\n");

  // ── Chequeo sensor ────────────────────────────────
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
  Serial.println();

  int valoresReposo[N_LECTURAS];
  int valoresContraccion[N_LECTURAS];

  // ── Reposo con cuenta atras ───────────────────────
  for (int i = 0; i < N_LECTURAS; i++) {
    contarAtras(3, "RELAJA: ");
    valoresReposo[i] = medirFase();
    Serial.print("  Reposo "); Serial.print(i + 1); Serial.print("/");
    Serial.print(N_LECTURAS); Serial.print(": ");
    Serial.println(valoresReposo[i]);
    delay(500);
  }

  Serial.println();

  // ── Contraccion con ENTER ─────────────────────────
  for (int i = 0; i < N_LECTURAS; i++) {
    Serial.print("Contrac "); Serial.print(i + 1); Serial.print("/");
    Serial.print(N_LECTURAS);
    Serial.println(" — CONTRAE y pulsa ENTER:");
    esperarEnter();
    valoresContraccion[i] = medirFase();
    Serial.print("  Valor: "); Serial.println(valoresContraccion[i]);
    delay(500);
  }

  // ── Resultados ────────────────────────────────────
  int reposoMedio      = mediaRobusta(valoresReposo, N_LECTURAS);
  int contraccionMedia = mediaRobusta(valoresContraccion, N_LECTURAS);
  int diferencia       = contraccionMedia - reposoMedio;

  Serial.println("\n=== RESULTADOS ===");
  for (int i = 0; i < N_LECTURAS; i++) {
    Serial.print("  Lectura "); Serial.print(i + 1);
    Serial.print(":  reposo="); Serial.print(valoresReposo[i]);
    Serial.print("  contraccion="); Serial.println(valoresContraccion[i]);
  }

  Serial.println("------------------");
  Serial.println("(descartados max y min de cada fase)");
  Serial.print("REPOSO MEDIO:      "); Serial.println(reposoMedio);
  Serial.print("CONTRACCION MEDIA: "); Serial.println(contraccionMedia);
  Serial.print("DIFERENCIA:        "); Serial.println(diferencia);

  Serial.println("------------------");
  if      (diferencia < 50)  Serial.println("ERROR: Diferencia nula -> cargador, electrodos o sensor");
  else if (diferencia < 150) Serial.println("AVISO: Diferencia justa -> recoloca sensor o cambia electrodos");
  else if (diferencia < 250) Serial.println("OK: Aceptable");
  else                       Serial.println("OK: Calibracion correcta");

  Serial.println("\nCOPIA ESTOS VALORES AL PROGRAMA DE LA MANO:");
  Serial.print("int REPOSO      = "); Serial.print(reposoMedio); Serial.println(";");
  Serial.print("int CONTRACCION = "); Serial.print(contraccionMedia); Serial.println(";");
}

void loop() {}
