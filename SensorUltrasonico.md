# Módulo SensorUltrasonico

Encapsula el manejo del sensor ultrasónico HC-SR04 (TRIG en GPIO25, ECHO en GPIO26) para el ESP32. Archivos:

- [include/SensorUltrasonico.h](include/SensorUltrasonico.h)
- [src/SensorUltrasonico.cpp](src/SensorUltrasonico.cpp)

## Explicación parte por parte

**`LecturaDistancia`** (struct): en vez de devolver solo un `float` (donde no sabrías distinguir "0 cm real" de "error"), la clase devuelve dos datos juntos: `distanciaCm` y `valida`. Así quien use el sensor siempre revisa `valida` antes de confiar en el número.

**Constructor `SensorUltrasonico(pinTrig, pinEcho, timeoutUs)`**: guarda qué pines usar (por defecto TRIG=25, ECHO=26) y cuánto esperar como máximo por el eco (30000 µs = 30 ms por defecto, tiempo de sobra para el rango del HC-SR04).

**`begin()`**: configura TRIG como salida y ECHO como entrada. Se llama una sola vez en `setup()`.

**`medirDistanciaCm()`** — el corazón de la clase, hace 3 pasos:

1. **Disparo**: pone TRIG en HIGH durante 10 µs (con 2 µs de LOW antes, para asegurar un flanco limpio). Esto le dice al sensor "emite el pulso de ultrasonido".
2. **Medición del eco**: `pulseIn(_pinEcho, HIGH, _timeoutUs)` se queda esperando a que ECHO suba y vuelva a bajar, y devuelve cuánto tiempo estuvo en HIGH (eso es el tiempo de ida y vuelta del sonido). El tercer parámetro es el timeout: si no llega ningún pulso en ese tiempo, `pulseIn` devuelve `0` en vez de bloquear el programa para siempre, evitando que el programa se quede esperando indefinidamente.
3. **Cálculo de distancia**: `distancia = (tiempo_us * 0.0343) / 2`. El `0.0343` es la velocidad del sonido en cm/µs (343 m/s), y se divide entre 2 porque el tiempo medido es ida **y** vuelta.

Al final se valida que la distancia esté dentro del rango físico que el HC-SR04 puede medir de forma confiable (2 cm a 400 cm, según su datasheet). Si la duración es 0 (timeout) o la distancia calculada cae fuera de ese rango, se devuelve `valida = false`. Esto es una validación de **hardware**, no de "rangos de negocio" — la lógica de qué hacer con cada rango de distancia la maneja otro módulo.

## Cómo probar que el sensor mide correctamente

Con un `main.cpp` mínimo de prueba (no hace falta tocar los LEDs):

```cpp
#include <Arduino.h>
#include "SensorUltrasonico.h"

SensorUltrasonico sensor; // usa GPIO25/GPIO26 por defecto

void setup() {
  Serial.begin(115200);
  sensor.begin();
}

void loop() {
  LecturaDistancia lectura = sensor.medirDistanciaCm();
  if (lectura.valida) {
    Serial.print("Distancia: ");
    Serial.print(lectura.distanciaCm);
    Serial.println(" cm");
  } else {
    Serial.println("Lectura invalida (sin eco o fuera de rango)");
  }
  delay(300);
}
```

Pasos de prueba:

1. Sube el código al ESP32 y abre el Monitor Serial (115200 baudios).
2. Pon un objeto plano (una mano, un libro) frente al sensor a distancias conocidas (ej. 10 cm, 30 cm, 1 m con una regla o cinta métrica) y verifica que el valor impreso coincida aproximadamente.
3. Mueve el objeto acercándolo y alejándolo: la lectura debe cambiar de forma suave y consistente, sin saltos erráticos.
4. Verifica los cables: TRIG del sensor a GPIO25, ECHO a GPIO26, VCC a 5V, GND a GND (el ECHO del HC-SR04 da salida de 5V; si el ESP32 es sensible a eso, usar un divisor de voltaje es recomendable, aunque muchas placas ESP32 lo toleran igual en pruebas cortas).

## Qué pasa cuando no hay eco o la lectura es inválida

- Si no llega eco dentro del timeout (objeto muy lejos, muy cerca, mal alineado, o cable suelto), `pulseIn` devuelve 0 y el método retorna `valida = false`, `distanciaCm = 0.0`, **sin bloquear el programa**.
- Si el eco llega pero la distancia calculada cae fuera de 2–400 cm (ruido o rebote raro), también se marca `valida = false`.
- La responsabilidad de la clase termina ahí: **informa** que la lectura no es confiable. Qué hacer con eso (ignorar el ciclo, mantener el último valor válido, apagar LEDs, mostrar un estado de "error", reintentar) le corresponde a quien use la clase (el módulo de lógica de rangos o el `main.cpp`), no al sensor en sí — así se mantiene la separación de responsabilidades.
