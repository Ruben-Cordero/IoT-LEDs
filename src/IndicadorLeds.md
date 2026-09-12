# IndicadorLeds

`IndicadorLeds` controla los tres LEDs que muestran un estado ya decidido por otro módulo del proyecto. Su única responsabilidad es encender el LED correspondiente y apagar los demás.

La clase no mide distancias, no usa el sensor ultrasónico y no compara rangos. El módulo que clasifica una distancia debe convertirla a un `EstadoIndicador` y entregárselo mediante `mostrar()`.

## Conexiones físicas

Cada LED se conecta de la siguiente forma:

`GPIO -> resistencia de 330 ohmios -> ánodo del LED -> cátodo -> GND`

Con este cableado, un valor `HIGH` enciende el LED y un valor `LOW` lo apaga.

| LED | GPIO |
| --- | --- |
| Rojo | 27 |
| Amarillo | 32 |
| Verde | 33 |

## Métodos

### `begin()`

Configura GPIO27, GPIO32 y GPIO33 como salidas. Al terminar, deja los tres LEDs apagados. Debe llamarse una vez desde `setup()`.

### `mostrar(EstadoIndicador estado)`

Apaga todos los LEDs y enciende solo el que corresponde al estado recibido.

| Estado | Resultado |
| --- | --- |
| `EstadoIndicador::Rojo` | LED rojo encendido fijo; amarillo y verde apagados. |
| `EstadoIndicador::Amarillo` | LED amarillo encendido fijo; rojo y verde apagados. |
| `EstadoIndicador::Verde` | LED verde encendido fijo; rojo y amarillo apagados. |
| `EstadoIndicador::Error` | Los tres LEDs apagados. |

No implementa parpadeos, temporizadores, `delay()`, `millis()` ni interrupciones.

## Uso

```cpp
#include "IndicadorLeds.h"

IndicadorLeds indicador;

void setup() {
    indicador.begin();
}

void loop() {
    // El clasificador entrega el estado ya determinado.
    indicador.mostrar(EstadoIndicador::Verde);
}
```

Por ejemplo, el clasificador puede usar `Rojo` para una distancia válida menor de 20 cm, `Amarillo` entre 20 cm y menos de 40 cm, `Verde` desde 40 cm y `Error` para una lectura inválida. Esa decisión pertenece al clasificador, no a `IndicadorLeds`.
