# ClasificadorDistancia

Módulo puro (sin hardware) que clasifica una lectura de distancia del sensor
ultrasónico en uno de tres rangos, con umbrales configurables e histéresis
para evitar cambios inestables cerca de un umbral.

## Archivos

| Archivo | Contenido |
|---|---|
| `include/Config.h` | Umbrales, margen de histéresis y límites del rango de trabajo (constantes configurables). |
| `lib/ClasificadorDistancia/ClasificadorDistancia.h` | Tipos (`LecturaSensor`, `RangoDistancia`) y la firma de `clasificarDistancia`. |
| `lib/ClasificadorDistancia/ClasificadorDistancia.cpp` | Implementación de la clasificación. |

## Por qué esta estructura

- **Función, no clase**: no hay estado interno que mantener entre llamadas; el
  llamador (quien tenga el `loop()`) guarda el último rango y lo pasa de
  vuelta en la siguiente llamada. Esto hace la función trivial de probar:
  mismo input → mismo output.
- **Separado del sensor y de los LEDs**: el clasificador no lee pines, no usa
  `Serial`, no usa `delay`. Solo recibe un `LecturaSensor` y devuelve un
  `RangoDistancia`. Así el sensor y los LEDs los desarrolla otro integrante
  sin acoplarse a esta lógica.
- **Nombres por distancia, no por color**: `CERCANO` / `MEDIO` / `LEJANO` en
  vez de `ROJO` / `AMARILLO` / `VERDE`. Qué color le corresponde a cada rango
  es una decisión del módulo de LEDs, no de este módulo.
- **Umbrales en `Config.h`, no en el código**: `include/Config.h` lo añade
  PlatformIO automáticamente al path de includes de todo el proyecto, así
  cualquier integrante puede ajustar los umbrales sin tocar
  `ClasificadorDistancia.cpp`.

## Config.h

```cpp
constexpr float UMBRAL_CERCA_MEDIO_CM = 20.0f;
constexpr float UMBRAL_MEDIO_LEJOS_CM = 40.0f;
constexpr float MARGEN_HISTERESIS_CM = 2.0f;
constexpr float DISTANCIA_MINIMA_VALIDA_CM = 2.0f;
constexpr float DISTANCIA_MAXIMA_VALIDA_CM = 200.0f;
```

## ClasificadorDistancia.h

```cpp
struct LecturaSensor {
    float distanciaCm;
    bool esValida;
};

enum class RangoDistancia {
    CERCANO,
    MEDIO,
    LEJANO,
    ERROR
};

RangoDistancia clasificarDistancia(const LecturaSensor& lectura, RangoDistancia rangoAnterior);
```

- `LecturaSensor` es exactamente el dato de entrada: distancia en cm y si es
  válida. No sabe nada del HC-SR04 ni de temporizadores.
- `RangoDistancia` es el resultado, incluyendo `ERROR` para lectura inválida
  o fuera del rango de trabajo.
- `rangoAnterior` es el resultado de la clasificación previa. Se usa
  únicamente para la histéresis. Si no hay clasificación previa (primer
  arranque), se pasa `RangoDistancia::ERROR`.

## Cómo funciona ClasificadorDistancia.cpp

### 1. Clasificación pura por umbrales (función privada)

```cpp
RangoDistancia clasificarPorUmbrales(float distanciaCm) {
    if (distanciaCm < UMBRAL_CERCA_MEDIO_CM) return RangoDistancia::CERCANO;
    if (distanciaCm < UMBRAL_MEDIO_LEJOS_CM) return RangoDistancia::MEDIO;
    return RangoDistancia::LEJANO;
}
```

Está en un `namespace` anónimo porque es un detalle interno, no parte de la
interfaz pública del módulo. Usa `<` (estrictamente menor) para que el valor
exacto del umbral quede siempre en el rango de arriba: 20 cm → `MEDIO`,
40 cm → `LEJANO`. Los tres rangos son contiguos, sin huecos ni solapes.

### 2. Función pública `clasificarDistancia`

Orden de las comprobaciones:

1. **Lectura inválida** (`esValida == false`) → `ERROR`. Manda sobre
   cualquier otra cosa.
2. **Fuera del rango de trabajo** (`d < 2` o `d > 200`) → `ERROR`, aunque el
   sensor la haya marcado como válida.
3. **Sin rango anterior confiable** (`rangoAnterior == ERROR`) → se clasifica
   directo por umbrales, no hay nada que estabilizar todavía.
4. **Con rango anterior** → se aplica histéresis (ver siguiente sección):
   solo se cambia de rango si la distancia superó el umbral correspondiente
   por el margen configurado; si no, se mantiene el rango anterior.

## Histéresis: por qué no hay parpadeo en los umbrales

Es el mismo principio de un disparador Schmitt. Para el umbral de 20 cm con
margen de 2 cm:

- Si el rango anterior es `CERCANO`, solo pasa a `MEDIO` cuando `d >= 22`.
- Si el rango anterior es `MEDIO`, solo vuelve a `CERCANO` cuando `d < 18`.

Entre 18 y 22 cm el rango **no cambia**, sea cual sea el valor exacto: se
queda donde estaba. Así, lecturas ruidosas como 19, 21, 19, 21 nunca
disparan un cambio de rango. El mismo razonamiento aplica al umbral de 40 cm
entre `MEDIO` y `LEJANO`.

Si el salto es grande (por ejemplo, de `CERCANO` a una lectura de 150 cm en
una sola medición), la función ya limpió ambas bandas de histéresis de un
solo golpe, así que cae directo en `LEJANO` sin pasos intermedios raros.

## Cómo probar la clasificación sin placa ni sensor

`clasificarDistancia` es una función pura: no usa `Serial`, no usa `delay`,
no toca pines. Se puede probar con datos inventados, por ejemplo con
`platform = native` de PlatformIO o cualquier `main()` de escritorio:

```cpp
LecturaSensor lectura{15.0f, true};
RangoDistancia r = clasificarDistancia(lectura, RangoDistancia::ERROR);
// r debería ser CERCANO
```

Para simular una secuencia de lecturas (como lo haría el `loop()` real), se
llama la función repetidamente pasando como `rangoAnterior` el resultado de
la llamada anterior:

```cpp
RangoDistancia rango = RangoDistancia::ERROR;
rango = clasificarDistancia({19.0f, true}, rango); // CERCANO
rango = clasificarDistancia({21.0f, true}, rango); // sigue CERCANO (no superó 22)
rango = clasificarDistancia({22.0f, true}, rango); // ahora sí, MEDIO
```

## Casos de prueba en los límites

Con umbral 20, umbral 40, margen 2, mínimo 2, máximo 200 — partiendo de un
`rangoAnterior` conocido:

| Anterior | Distancia | Resultado esperado | Por qué |
|---|---|---|---|
| ERROR | 19.9 | CERCANO | justo bajo el umbral 1 |
| ERROR | 20.0 | MEDIO | el umbral pertenece al rango superior |
| CERCANO | 21.9 | CERCANO | no superó el margen (20+2=22) |
| CERCANO | 22.0 | MEDIO | superó el margen exactamente |
| MEDIO | 18.1 | MEDIO | no bajó lo suficiente (20-2=18) |
| MEDIO | 17.9 | CERCANO | bajó del margen |
| MEDIO | 39.9 | MEDIO | justo bajo el umbral 2 |
| ERROR | 40.0 | LEJANO | el umbral pertenece al rango superior |
| MEDIO | 42.0 | LEJANO | superó el margen del umbral 2 |
| LEJANO | 38.1 | LEJANO | no bajó lo suficiente (40-2=38) |
| LEJANO | 37.9 | MEDIO | bajó del margen |
| cualquiera | 2.0 | válido, clasifica normal | límite inferior de trabajo incluido |
| cualquiera | 1.9 | ERROR | por debajo del mínimo de trabajo |
| cualquiera | 200.0 | válido, LEJANO | límite superior de trabajo incluido |
| cualquiera | 200.1 | ERROR | por encima del máximo de trabajo |
| cualquiera (`esValida=false`) | cualquier valor | ERROR | lectura inválida manda sobre todo lo demás |

## Qué hacer con lectura inválida o fuera de rango

La función ya lo resuelve devolviendo `RangoDistancia::ERROR`, sin lanzar
excepciones ni tocar hardware. Qué hacer visualmente con `ERROR` (apagar los
LEDs, parpadear, etc.) es responsabilidad del módulo de LEDs: el
clasificador solo informa el estado, no actúa sobre el hardware.

## Qué no incluye este módulo (a propósito)

- Lectura del sensor HC-SR04 (pines, `pulseIn`, temporización).
- Control de los LEDs (pines, colores, `digitalWrite`).
- `main.cpp` / `setup()` / `loop()`.
- Archivos de pruebas automatizadas.

Esas partes corresponden a otros integrantes del equipo y solo dependen de
este módulo a través de `LecturaSensor`, `RangoDistancia` y
`clasificarDistancia`.
