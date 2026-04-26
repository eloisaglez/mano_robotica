# MANO ROBÓTICA CON CONTROL EMG

**Documentación del Proyecto — Versión 2**
*IES Diego Velázquez — Tecnología e Ingeniería*

---

## 1. Descripción del Proyecto

Mano robótica de 3 dedos en configuración de pinza controlada por señales electromiográficas (EMG) del músculo del antebrazo o bíceps. El usuario contrae el músculo para cerrar la pinza y la relaja para abrirla.

El programa incluye calibración automática al arrancar con opción de usar valores guardados para pruebas rápidas.

### Guía rápida — ¿Qué programa uso?

| Situación | Programa | Necesitas |
|---|---|---|
| Comprobar que el sensor funciona | `lectura_emg` | Ordenador |
| Comprobar sensor sin ordenador | `lectura_emg_leds` | 2 LEDs + 2 resistencias 220Ω |
| Calibrar el sensor | `calibracion_emg` | Ordenador |
| Buscar punto muerto del servo | `punto_muerto_servo` | Ordenador |
| Probar servo manualmente (c/a) | `test_servo` | Ordenador |
| Probar servo automáticamente | `test_servo_auto` | — |
| Usar la mano con ordenador | `mano_robotica` | Ordenador |
| Usar la mano sin ordenador | `mano_autonoma` | Calibrar antes y copiar valores |
| Usar la mano con botón | `mano_boton` | Pulsador en pin 2 |

---

## 2. Hardware Necesario

### 2.1 Componentes obligatorios

| Componente | Referencia | Cantidad | Notas |
|---|---|---|---|
| Microcontrolador | Arduino UNO R4 WiFi | 1 | Requiere librería Servo actualizada |
| Sensor EMG | MyoWare v1 (AT-04-001) | 1 | Salida ENV por pin SIG |
| Electrodos desechables | Electrodos de gel adhesivo | 3 | Un solo uso — cambiar cada sesión |
| Estructura mano | Impresión 3D o madera | 1 | 3 dedos en configuración pinza |
| Hilo de nailon | 0.5-1 mm diámetro | 1 m | Para transmisión de movimiento |
| Cable USB | USB-A a USB-C | 1 | Para programar y alimentar |
| Pilas AA | 1.5V cada una | 4 | Para alimentar el servo por separado (6V) |

### 2.2 Motor — Opción A: Servo rotación continua DS04-NFC

Motor probado en este proyecto. Controla velocidad y dirección por microsegundos. Requiere calibración del punto muerto cada vez que cambia la fuente de alimentación.

| Parámetro | Valor USB (5V) | Valor Pilas (6V) |
|---|---|---|
| Punto muerto | ~1520 µs | ~1540 µs |
| Cerrar (max velocidad) | write(180) o 2000 µs | 2000 µs |
| Abrir (max velocidad) | write(0) o 1000 µs | 1000 µs |
| Tiempo cierre/apertura | 1000-1500 ms | 1000-1500 ms |
| Control proporcional | No | No |
| Torque | 5.5 kg/cm | 5.5 kg/cm |
| Consumo máximo | 1000 mA | 1000 mA |

**IMPORTANTE:** El punto muerto cambia con el voltaje de alimentación. Siempre recalibrar al cambiar de USB a pilas o viceversa.

### 2.3 Motor — Opción B: Servo estándar SG90

Motor más sencillo de programar — control por posición, sin problemas de punto muerto. Permite control proporcional.

| Parámetro | Valor |
|---|---|
| Abierto | write(180) |
| Cerrado | write(0) |
| Control proporcional | Sí — posición proporcional al EMG |
| Torque | 1.8 kg/cm |
| Alimentación | 5V desde Arduino |
| Limitación | Engranajes frágiles, se rompen con facilidad |

### 2.4 Motor — Opción C: Motor paso a paso 28BYJ-48 + ULN2003

Motor con mayor torque y parada exacta sin punto muerto. Requiere driver ULN2003.

| Parámetro | Valor |
|---|---|
| Driver necesario | ULN2003 (incluido en el kit) |
| Alimentación | 5V |
| Torque | Mayor que SG90 |
| Librería Arduino | Stepper.h (incluida en IDE) |
| Ventaja | Para exacto, sin energía residual |
| Desventaja | Más lento que un servo |

---

## 3. Conexiones

### 3.1 MyoWare v1

| Pin MyoWare | Conectar a | Notas |
|---|---|---|
| VCC (+) | 5V Arduino | NO usar 3.3V — satura la señal |
| GND (-) | GND Arduino | GND común con todo el circuito |
| SIG | A0 Arduino | Salida ENV — señal procesada |
| REF (cable negro) | Electrodo de referencia | Colocar en hueso — codo o muñeca |

### 3.2 Electrodos

| Electrodo | Posición | Notas |
|---|---|---|
| MIDDLE (M) | Vientre del músculo — antebrazo interior | Alineado con fibras musculares |
| END (E) | Adyacente al M hacia el codo | M y E son intercambiables |
| REF (negro) | Parte ósea — codo o muñeca | NO sobre músculo |

### 3.3 Alimentación del servo con pilas externas

**CRÍTICO:** Alimentar el servo por separado del Arduino. El GND debe ser común.

| Cable servo | Conectar a |
|---|---|
| Rojo (VCC) | Pilas + (6V) |
| Marrón (GND) | GND común Arduino + Pilas |
| Naranja (señal) | Pin 9 Arduino |

```
Arduino GND ---+--- Servo GND (marrón)
               |
Pilas GND -----+

Pilas + (6V) ------- Servo VCC (rojo)

Arduino pin 9 ------ Servo señal (naranja)
```

### 3.4 Alimentación del servo desde Arduino (solo SG90)

Solo para el SG90 que consume poco. El DS04-NFC necesita alimentación externa.

| Cable servo | Conectar a |
|---|---|
| Rojo (VCC) | 5V Arduino |
| Marrón (GND) | GND Arduino |
| Naranja (señal) | Pin 9 Arduino |

---

## 4. Calibración del Sensor EMG

### 4.1 Preparación

- Limpiar la piel con alcohol y secar bien
- Afeitar la zona si hay vello
- Pegar el electrodo de referencia en la parte ósea del codo
- Pegar los electrodos M y E en el vientre del antebrazo interior
- Presionar cada electrodo 10 segundos para activar el gel
- **CRÍTICO:** Desconectar el cargador del portátil — ruido de red 50 Hz satura la señal
- Verificar que los dos LEDs del MyoWare están encendidos
- Apoyar el antebrazo en la mesa para medir reposo

### 4.2 Calibración integrada en el programa

El programa ofrece dos opciones al arrancar:

- Pulsar `c` — calibrar con 5 mediciones de reposo (cuenta atrás) y 5 de contracción (ENTER)
- Pulsar `s` — usar valores guardados por defecto para pruebas rápidas

La calibración descarta el valor máximo y mínimo de cada fase (media robusta) para eliminar lecturas atípicas.

### 4.3 Valores típicos (alimentación 5V, ADC 0-1023)

| Estado | Rango típico | Notas |
|---|---|---|
| Reposo | 50 - 200 | Depende del músculo y persona |
| Contracción leve | 200 - 450 | |
| Contracción máxima | 500 - 750 | Con buena colocación |
| Saturación | > 900 | Revisar cargador, electrodos o ganancia |

### 4.4 Umbrales

```
UMBRAL_BAJO = REPOSO + 0.08 * (CONTRACCION - REPOSO)  // 8% — detecta relajado
UMBRAL_ALTO = REPOSO + 0.60 * (CONTRACCION - REPOSO)  // 60% — detecta contracción
```

La zona entre ambos umbrales actúa como histéresis para evitar oscilaciones.

Además se requieren 5 lecturas consecutivas por debajo del UMBRAL_BAJO para confirmar relajado — evita falsos positivos por el condensador integrador del MyoWare.

### 4.5 Calibración del punto muerto del servo DS04-NFC

El punto muerto varía con el voltaje de alimentación. Calibrar cada vez que se cambie de fuente.

- Subir programa de punto muerto con `writeMicroseconds(1500)`
- Si el servo gira, cambiar de 5 en 5: 1500, 1510, 1520...
- El valor donde el servo se queda quieto es `SERVO_PARADO_US`
- Rango habitual: 1450 a 1550 µs
- Con USB (5V) suele ser ~1520 µs
- Con pilas (6V) suele ser ~1540 µs

### 4.6 Problemas comunes de calibración

| Síntoma | Causa probable | Solución |
|---|---|---|
| Reposo y contracción iguales (~900) | Cargador conectado o 3.3V | Desconectar cargador, usar 5V |
| Diferencia < 50 | Electrodos secos o mal colocados | Cambiar electrodos, recolocar |
| Diferencia < 150 | Músculo equivocado o ganancia baja | Mover sensor, girar GAIN antihorario |
| Valores altos sin contraer | Electrodo REF sobre músculo | Mover REF a hueso |
| Valores inestables | Cable REF suelto | Revisar soldadura del cable de referencia |
| EMG = 11 constante | Sensor apagado o desconectado | Revisar VCC 5V y GND |
| Segundo LED no enciende al contraer | Electrodo REF sin contacto | Presionar REF, limpiar piel |

---

## 5. Lógica de Control

### 5.1 Máquina de estados

| Estado | Descripción | Transición |
|---|---|---|
| 0 — ESPERA | Espera 5 lecturas consecutivas en reposo | 5x EMG < UMBRAL_BAJO → Estado 1 |
| 1 — ABIERTA | Esperando contracción | EMG > UMBRAL_ALTO → Estado 2 |
| 2 — CERRANDO | Motor cerrando durante TIEMPO_MS | Tiempo cumplido → Estado 3 |
| 3 — CERRADA | Espera relajado (mín ESPERA_CERRADA ms, 5 lecturas consecutivas) | 5x EMG < UMBRAL_BAJO → Estado 4 |
| 4 — ABRIENDO | Motor abriendo durante TIEMPO_MS | Tiempo cumplido → Estado 1 |

### 5.2 Protecciones

| Protección | Comportamiento |
|---|---|
| Sensor desconectado (EMG < 15) | Para servo, detach para evitar movimiento, espera reconexión |
| Sensor saturado (EMG > 900) | Para servo, vuelve a estado 0, avisa por Monitor Serie |
| Lecturas consecutivas de reposo | Exige 5 lecturas seguidas bajo umbral antes de abrir |
| Tiempo mínimo cerrada | 2 segundos antes de escuchar relajado |
| Calibración fallida | Reintenta automáticamente cada 3 segundos |

### 5.3 Parámetros ajustables

| Parámetro | Valor típico | Descripción |
|---|---|---|
| REPOSO | 146 (defecto) | Se recalcula al calibrar |
| CONTRACCION | 642 (defecto) | Se recalcula al calibrar |
| UMBRAL_BAJO | 8% del rango | Umbral para detectar relajado |
| UMBRAL_ALTO | 60% del rango | Umbral para detectar contracción |
| TIEMPO_MS | 1500 ms | Duración cierre/apertura del motor |
| ESPERA_CERRADA | 2000 ms | Tiempo mínimo cerrada |
| LECTURAS_REPOSO | 5 | Lecturas consecutivas para confirmar reposo |
| N_CALIBRACION | 5 | Mediciones por fase de calibración |
| SERVO_PARADO_US | 1540 µs (6V) | Punto muerto del DS04-NFC |
| SERVO_CERRAR_US | 2000 µs | Máxima velocidad cerrando |
| SERVO_ABRIR_US | 1000 µs | Máxima velocidad abriendo |

---

## 6. Programas Arduino

### 6.1 Lectura simple del sensor (sin calibración)

Programa básico para ver en tiempo real los valores del sensor EMG por el Monitor Serie. Útil para verificar que el sensor funciona, comprobar la colocación de los electrodos y ajustar la ganancia.

### 6.2 Lectura del sensor con LEDs

Programa para verificar el funcionamiento del sensor SIN necesidad de ordenador. Los LEDs indican el estado:
- LED verde → músculo relajado
- LED rojo → músculo contraído
- Ambos parpadean → sensor desconectado o saturado

Conexiones adicionales:
```
Pin 3 ---[220Ω]--- LED verde (+) --- GND
Pin 4 ---[220Ω]--- LED rojo  (+) --- GND
```

### 6.3 Calibración del sensor

Programa independiente para obtener los valores de reposo y contracción. Ejecutar antes de cada sesión. Hace 5 mediciones de cada fase, descarta atípicos y muestra los valores listos para copiar al programa de la mano.

### 6.4 Programa principal — Mano robótica

Programa unificado con calibración integrada y control del servo. Al arrancar ofrece:

- `c` — calibrar con 5 mediciones de reposo (cuenta atrás) y 5 de contracción (ENTER)
- `s` — usar valores por defecto para pruebas rápidas

Incluye detección de sensor apagado, saturado y reconexión automática.

### 6.5 Buscar punto muerto del servo

Programa mínimo para encontrar el valor de parada del DS04-NFC. Subir y cambiar el valor de `writeMicroseconds` hasta que el servo pare.

```cpp
servo.writeMicroseconds(1540);  // cambiar hasta que pare
```

### 6.6 Test del servo (manual)

Programa para probar el motor independientemente del sensor. Acepta comandos por Monitor Serie:

- `c` + ENTER → cierra durante TIEMPO_MS y para
- `a` + ENTER → abre durante TIEMPO_MS y para

Usar para verificar que el servo abre y cierra correctamente antes de conectar el sensor.

### 6.7 Test del servo (automático)

Programa que ejecuta una secuencia automática de cierre y apertura para verificar rápidamente que el servo funciona tras montar el cableado.

### 6.8 Mano robótica — Modo autónomo (sin ordenador)

Programa que arranca automáticamente con valores guardados tras 5 segundos de espera. No requiere Monitor Serie, ordenador ni LEDs. Ideal para el día del concurso.

Preparación:
1. Calibrar previamente con `calibracion_emg.ino`
2. Copiar los valores de REPOSO y CONTRACCION al programa
3. Subir el programa, desconectar del ordenador y alimentar con pilas

### 6.9 Mano robótica — Arranque con botón

Programa que espera a que se pulse un botón para arrancar. Sin ordenador ni LEDs.

| Pulsación | Comportamiento |
|---|---|
| Corta (< 3s) | Arranca con valores guardados |
| Larga (> 3s) | Entra en modo calibración con botón |

En modo calibración con botón:
- Reposo: 5 mediciones automáticas cada 3 segundos (relajar el músculo)
- Contracción: 5 mediciones, pulsar botón cada vez (contraer y pulsar)

Conexión del botón:
```
Pin 2 ──── [pulsador] ──── GND    (INPUT_PULLUP, sin resistencia)
```

### 6.10 Calibración independiente (opcional)

Si se prefiere calibrar por separado sin el programa de la mano, existe un programa solo de calibración que al terminar muestra los valores para copiar al programa principal.

---

## 7. Problemas Conocidos y Soluciones

| Problema | Causa | Solución |
|---|---|---|
| Motor se mueve al conectar Arduino | Pin 9 flota durante arranque del R4 | Resistencia 10kΩ entre pin 9 y GND |
| Mano abre inmediatamente tras cerrar | EMG baja rápido (condensador MyoWare) | ESPERA_CERRADA 2-3s + 5 lecturas consecutivas |
| Librería Servo no compila | R4 usa Renesas RA4M1 | Instalar paquete Arduino UNO R4 actualizado |
| Sensor da 900+ en reposo | Cargador conectado (ruido 50 Hz) | Desconectar cargador del portátil |
| Servo no para (energía residual) | Punto muerto incorrecto | Calibrar con writeMicroseconds de 5 en 5 |
| Punto muerto cambia con pilas | Voltaje diferente desplaza el punto muerto | Recalibrar punto muerto con cada fuente |
| Motor gira todo el rato | Orden repetida cada loop | Máquina de estados — orden solo al cambiar |
| Enter no funciona en R4 | Buffer serie del R4 WiFi | Función esperarEnter con limpieza de buffer |
| Segundo LED no enciende | Umbral interno del MyoWare | No es problema — leer por analogRead |
| Electrodo REF se suelta | Cable con tensión o piel con vello | Fijar con cinta, limpiar y afeitar zona |
| Motor no abre/cierra con pilas | write(0)/write(180) no funciona a 6V | Usar writeMicroseconds(1000/2000) |
| SG90 suena pero no gira | Engranaje interno roto | Abrir carcasa, revisar o sustituir |

---

## 8. Notas Importantes

### 8.1 MyoWare v1

- Alimentación: 2.9V a 5.7V — usar siempre 5V
- El LED de status tiene umbral interno propio — no indica si el código funciona
- El condensador integrador hace que la señal baje lentamente tras contraer
- Electrodos de un solo uso — cambiar si están secos
- Potenciómetro GAIN: horario = menos ganancia, antihorario = más ganancia
- El electrodo de referencia DEBE estar sobre hueso, no sobre músculo
- Limpiar la piel con alcohol y afeitar si hay vello

### 8.2 DS04-NFC

- Es servo de VELOCIDAD, no de posición — writeMicroseconds controla velocidad
- El punto muerto varía con el voltaje: ~1520 µs a 5V, ~1540 µs a 6V
- Tiene potenciómetro interno de ajuste (bajo la carcasa y la placa)
- Consume hasta 1000 mA — alimentar con fuente externa, no desde Arduino
- El eje tiene ligera desviación — no recomendado para aplicaciones de precisión
- GND del servo, Arduino y batería DEBEN estar conectados entre sí

### 8.3 Arduino UNO R4 WiFi

- El pin de señal flota durante el arranque más tiempo que el Uno clásico
- La librería Servo estándar no es compatible — instalar paquete UNO R4
- El buffer serie se comporta diferente — usar función esperarEnter con limpieza
- Funciona bien con alimentación USB solo con batería del portátil

### 8.4 Control proporcional (futuro)

Si se quiere que la mano se cierre proporcionalmente a la contracción, usar un servo estándar SG90 funcionando correctamente:

```cpp
int grados = map(emg, UMBRAL_BAJO, UMBRAL_ALTO, 180, 0);
grados = constrain(grados, 0, 180);
servo.write(grados);
```

Con el DS04-NFC o el 28BYJ-48 esto no es posible — el control es todo o nada.

---

## 9. Modo Autónomo (sin ordenador) — PENDIENTE

### 9.1 Problema

El programa actual requiere el Monitor Serie para:
- Elegir entre calibrar (c) o usar valores guardados (s)
- Pulsar ENTER durante la calibración de contracción
- Ver los valores de EMG y estado

En el concurso no habrá ordenador conectado — solo pilas alimentando Arduino y servo.

### 9.2 Solución propuesta

Modificar el programa para que funcione sin Monitor Serie, usando un pulsador y LEDs.

### 9.3 Hardware adicional necesario

| Componente | Pin | Función |
|---|---|---|
| Pulsador | Pin 2 (INPUT_PULLUP) | Iniciar calibración / confirmar contracción |
| LED verde | Pin 3 | Indica estado OK / listo |
| LED rojo | Pin 4 | Indica error / cerrando |
| Resistencias 220Ω | — | 2 unidades, una por LED |

### 9.4 Lógica propuesta

**Arranque normal (sin pulsar botón):**
```
Enciende → cuenta atrás 5s (LED verde parpadea) → usa valores guardados → listo
```

**Arranque con calibración (pulsar botón durante cuenta atrás):**
```
Enciende → cuenta atrás 5s → botón pulsado → entra en modo calibración
  LED verde fijo = relaja (3s cuenta atrás automática × 5 mediciones)
  LED rojo fijo = contrae (espera pulsación del botón × 5 mediciones)
  Ambos LEDs parpadean = calibración completada
  LED verde fijo = listo para usar
```

**Indicación de estados durante el uso:**
```
LED verde fijo      = ABIERTA — esperando contracción
LED rojo fijo       = CERRADA — esperando relajar
LED verde parpadeo  = CERRANDO o ABRIENDO
Ambos LEDs rápido   = ERROR — sensor desconectado o saturado
```

**Indicación de errores de calibración:**
```
LED rojo parpadeo lento  = Diferencia < 50 (error grave)
LED rojo parpadeo rápido = Diferencia < 150 (aviso)
```

### 9.5 Conexiones adicionales

```
Pin 2 ──── [pulsador] ──── GND     (INPUT_PULLUP, sin resistencia externa)

Pin 3 ──── [R 220Ω] ──── LED verde ──── GND

Pin 4 ──── [R 220Ω] ──── LED rojo ──── GND
```

### 9.6 Preparación para el concurso

1. La tarde anterior: conectar al ordenador, calibrar con el programa normal
2. Apuntar los valores de REPOSO y CONTRACCION
3. Modificar los valores por defecto en el código y subir
4. El día del concurso: enchufar las pilas y el programa arranca solo con los valores guardados
5. Si necesita recalibrar: pulsar el botón durante la cuenta atrás inicial

### 9.7 Bluetooth BLE (alternativa futura)

El Arduino UNO R4 WiFi tiene un módulo ESP32-S3 con Bluetooth BLE integrado. Se puede usar como Monitor Serie inalámbrico con aplicaciones como LightBlue o Wible en el móvil.

Esto permitiría calibrar y ver los valores de EMG desde el teléfono sin cable USB, manteniendo toda la funcionalidad actual del Monitor Serie.

Requisitos:
- Librería **ArduinoBLE** (oficial de Arduino) — instalar desde Administrador de bibliotecas en el IDE
- Firmware del ESP32-S3 actualizado — actualizar el paquete de placas Arduino UNO R4 a la última versión desde el Gestor de placas
- App en el móvil: LightBlue (iOS/Android) o Wible (iOS/Android)
