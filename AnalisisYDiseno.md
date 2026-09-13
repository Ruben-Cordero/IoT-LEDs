# 2. Análisis y Diseño

Diagramas basados en el flujo y la implementación real del proyecto:

`ESP32 → sensor ultrasónico → medición de distancia → validación → clasificación de distancia → conversión a estado → LEDs`

Componentes de hardware:

- ESP32
- HC-SR04 — TRIG: GPIO25, ECHO: GPIO26 (con divisor de tensión)
- LED rojo: GPIO27, LED amarillo: GPIO32, LED verde: GPIO33
- Resistencias de 220 ohm para cada LED

Módulos de software: `Config.h`, `LecturaDistancia`, `SensorUltrasonico`, `ClasificadorDistancia`, `IndicadorLeds`, `main.cpp`.

## 2.1 Diagrama de arquitectura

Muestra 3 zonas: hardware de entrada, el ESP32 con sus módulos de software, y hardware de salida. `Config.h` alimenta con constantes a `SensorUltrasonico` y `ClasificadorDistancia`; el resto de flechas sigue el flujo real de datos: HC-SR04 → SensorUltrasonico → LecturaDistancia → ClasificadorDistancia → main.cpp (que hace la conversión a estado) → IndicadorLeds → LEDs.

```mermaid
flowchart TB
    subgraph EXT_IN["Hardware de entrada"]
        HCSR04["HC-SR04<br/>(sensor ultrasonico)"]
    end

    subgraph ESP32["ESP32 (microcontrolador)"]
        CONFIG["Config.h<br/>(umbrales y pines)"]
        SENSOR["SensorUltrasonico"]
        LECTURA["LecturaDistancia<br/>(distanciaCm, valida)"]
        CLASIF["ClasificadorDistancia<br/>(clasificarDistancia)"]
        MAIN["main.cpp<br/>(loop + convertirAEstado)"]
        LEDSMOD["IndicadorLeds"]
    end

    subgraph EXT_OUT["Hardware de salida"]
        LEDR["LED rojo (GPIO27)"]
        LEDA["LED amarillo (GPIO32)"]
        LEDV["LED verde (GPIO33)"]
    end

    HCSR04 -- "TRIG / ECHO" --> SENSOR
    SENSOR --> LECTURA
    LECTURA --> CLASIF
    CLASIF --> MAIN
    MAIN --> LEDSMOD
    LEDSMOD --> LEDR
    LEDSMOD --> LEDA
    LEDSMOD --> LEDV

    CONFIG -.-> SENSOR
    CONFIG -.-> CLASIF
```

## 2.2 Diagrama de circuito (conexiones y pines)

No es un esquemático electrónico formal (para eso se usa Fritzing/KiCad), sino un diagrama de conexiones simplificado, suficiente para el informe.

```mermaid
flowchart LR
    subgraph ESP32PINS["ESP32"]
        P25["GPIO25"]
        P26["GPIO26"]
        P27["GPIO27"]
        P32["GPIO32"]
        P33["GPIO33"]
        V5["5V"]
        GND["GND"]
    end

    HVCC["HC-SR04 VCC"]
    HTRIG["HC-SR04 TRIG"]
    HECHO["HC-SR04 ECHO"]
    HGND["HC-SR04 GND"]
    DIV["Divisor de tension"]

    LEDR["LED rojo"]
    LEDA["LED amarillo"]
    LEDV["LED verde"]
    R1["Resistencia 220 ohm"]
    R2["Resistencia 220 ohm"]
    R3["Resistencia 220 ohm"]

    V5 --> HVCC
    P25 --> HTRIG
    HECHO --> DIV --> P26
    HGND --> GND

    P27 --> R1 --> LEDR --> GND
    P32 --> R2 --> LEDA --> GND
    P33 --> R3 --> LEDV --> GND
```

## 2.3 Diagrama estructural

Solo se necesita un diagrama de clases: el de arquitectura (2.1) ya cubre la vista de módulos/componentes, así que no se agrega un diagrama de componentes aparte.

- `LecturaDistancia` (struct): `distanciaCm: float`, `valida: bool`
- `SensorUltrasonico` (clase): atributos privados de pines/timeout, métodos `begin()` y `medirDistanciaCm(): LecturaDistancia`
- `RangoDistancia` (enum): CERCANO, MEDIO, LEJANO, ERROR
- `ClasificadorDistancia` (función libre, no clase): `clasificarDistancia(lectura, rangoAnterior): RangoDistancia`
- `EstadoIndicador` (enum): Rojo, Amarillo, Verde, Error
- `IndicadorLeds` (clase): pines privados, métodos `begin()`, `mostrar(estado)`, `apagarTodos()` privado
- `Config` (constantes, no clase): umbrales y rango válido

```mermaid
classDiagram
    class Config {
        <<constantes>>
        +UMBRAL_CERCA_MEDIO_CM
        +UMBRAL_MEDIO_LEJOS_CM
        +MARGEN_HISTERESIS_CM
        +DISTANCIA_MINIMA_VALIDA_CM
        +DISTANCIA_MAXIMA_VALIDA_CM
    }

    class LecturaDistancia {
        +distanciaCm : float
        +valida : bool
    }

    class RangoDistancia {
        <<enumeration>>
        CERCANO
        MEDIO
        LEJANO
        ERROR
    }

    class EstadoIndicador {
        <<enumeration>>
        Rojo
        Amarillo
        Verde
        Error
    }

    class SensorUltrasonico {
        -pinTrig : uint8_t
        -pinEcho : uint8_t
        -timeoutUs : unsigned long
        +begin() void
        +medirDistanciaCm() LecturaDistancia
    }

    class ClasificadorDistancia {
        <<funcion libre>>
        +clasificarDistancia(lectura, rangoAnterior) RangoDistancia
    }

    class IndicadorLeds {
        -PIN_ROJO : int
        -PIN_AMARILLO : int
        -PIN_VERDE : int
        +begin() void
        +mostrar(estado) void
        -apagarTodos() void
    }

    SensorUltrasonico ..> LecturaDistancia : crea
    ClasificadorDistancia ..> LecturaDistancia : usa
    ClasificadorDistancia ..> RangoDistancia : devuelve
    IndicadorLeds ..> EstadoIndicador : usa
    SensorUltrasonico ..> Config : usa
    ClasificadorDistancia ..> Config : usa
```

## 2.4 Diagramas de comportamiento

Se usan 2: uno para el flujo de control (actividad) y otro para la interacción entre módulos (secuencia). No se agrega diagrama de estados aparte porque las transiciones de rango (con histéresis) ya se explican como parte de la actividad.

### 2.4.1 Diagrama de actividad (ciclo del `loop`)

```mermaid
flowchart TD
    START(["Inicio del ciclo (loop)"])
    MEDIR["sensor.medirDistanciaCm()"]
    CLASIF{"clasificarDistancia(lectura, rangoAnterior)"}
    ROJO["convertirAEstado -> Rojo"]
    AMARILLO["convertirAEstado -> Amarillo"]
    VERDE["convertirAEstado -> Verde"]
    ERRORST["convertirAEstado -> Error"]
    MOSTRAR["indicador.mostrar(estado)"]
    ESPERA["delay(100 ms)"]

    START --> MEDIR --> CLASIF
    CLASIF -- CERCANO --> ROJO
    CLASIF -- MEDIO --> AMARILLO
    CLASIF -- LEJANO --> VERDE
    CLASIF -- "ERROR (invalida o fuera de rango)" --> ERRORST
    ROJO --> MOSTRAR
    AMARILLO --> MOSTRAR
    VERDE --> MOSTRAR
    ERRORST --> MOSTRAR
    MOSTRAR --> ESPERA --> START
```

*Nota: `clasificarDistancia` aplica internamente un margen de histéresis (2 cm) usando el rango anterior, para evitar parpadeo en los límites de 20/40 cm — por eso el diagrama lo trata como una sola decisión en vez de desglosar cada comparación.*

### 2.4.2 Diagrama de secuencia (un ciclo de medición)

```mermaid
sequenceDiagram
    participant Main as main.cpp
    participant Sensor as SensorUltrasonico
    participant Clasif as ClasificadorDistancia
    participant Leds as IndicadorLeds

    Main->>Sensor: medirDistanciaCm()
    Sensor-->>Main: LecturaDistancia(distanciaCm, valida)
    Main->>Clasif: clasificarDistancia(lectura, rangoAnterior)
    Clasif-->>Main: RangoDistancia
    Main->>Main: convertirAEstado(rango)
    Main->>Leds: mostrar(estado)
    Leds-->>Main: LEDs actualizados
```
