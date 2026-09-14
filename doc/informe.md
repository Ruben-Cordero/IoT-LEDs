# 1. Requerimientos Funcionales y No Funcionales

El objeto inteligente se implementa con un microcontrolador ESP32, un sensor ultrasónico HC-SR04 y tres LEDs. Su propósito es medir la distancia hasta un objeto, clasificarla y presentar el resultado mediante una señal visual fácil de interpretar.

## 1.1 Requerimientos funcionales

| Identificador | Requerimiento | Criterio de aceptación |
| --- | --- | --- |
| **RF1** | El sistema debe medir la distancia entre el HC-SR04 y un objeto. | Cada medición debe producir una distancia en centímetros acompañada por un indicador de validez. La ausencia de eco y los valores físicamente inválidos deben reconocerse como lectura inválida. |
| **RF2** | El sistema debe interpretar la medición y clasificarla en tres rangos contiguos y sin solapamiento. | Toda lectura válida dentro del rango de trabajo debe producir exactamente uno de estos resultados: <code>CERCANO</code>, <code>MEDIO</code> o <code>LEJANO</code>. |
| **RF3** | El sistema debe activar un actuador diferente según el rango detectado. | Para una lectura válida debe encenderse únicamente el LED asociado al rango. Ante una lectura inválida o fuera del rango de trabajo, los tres LEDs deben permanecer apagados. |

### 1.1.1 Rangos y lógica de control

El rango de trabajo definido para el objeto inteligente es de 2 a 200 cm, ambos límites incluidos. La clasificación base y la respuesta visual son:

| Distancia o condición | Rango lógico | Respuesta del actuador |
| --- | --- | --- |
| Lectura inválida, sin eco, menor que 2 cm o mayor que 200 cm | <code>ERROR</code> | Todos los LEDs apagados. |
| 2 cm ≤ distancia < 20 cm | <code>CERCANO</code> | LED rojo encendido; amarillo y verde apagados. |
| 20 cm ≤ distancia < 40 cm | <code>MEDIO</code> | LED amarillo encendido; rojo y verde apagados. |
| 40 cm ≤ distancia ≤ 200 cm | <code>LEJANO</code> | LED verde encendido; rojo y amarillo apagados. |

Los intervalos válidos son contiguos: terminando un rango comienza inmediatamente el siguiente. Tampoco se solapan, porque una misma distancia no puede pertenecer a dos intervalos al mismo tiempo. Los valores exactos de 20 y 40 cm se asignan al rango superior.

Para evitar cambios repetidos de color provocados por pequeñas variaciones de medición cerca de los límites, se declara un margen de histéresis de 2 cm. La histéresis no crea un cuarto rango; solamente determina cuándo se confirma una transición:

| Estado actual | Transición confirmada |
| --- | --- |
| <code>CERCANO</code> | Cambia al superar el límite cercano con el margen definido: distancia ≥ 22 cm. |
| <code>MEDIO</code> | Cambia a <code>CERCANO</code> cuando la distancia < 18 cm y a <code>LEJANO</code> cuando la distancia ≥ 42 cm. |
| <code>LEJANO</code> | Cambia al descender por debajo del límite lejano con el margen definido: distancia < 38 cm. |
| <code>ERROR</code> | La siguiente lectura válida se clasifica directamente con los límites base de 20 y 40 cm. |

En todos los casos, cada ciclo termina con un único rango lógico y, como máximo, un LED encendido.

## 1.2 Requerimientos no funcionales

Los siguientes valores se declaran como objetivos medibles. Su cumplimiento se demuestra en la sección de Pruebas y Validaciones.

| Identificador | Atributo | Requerimiento medible | Forma de verificación |
| --- | --- | --- | --- |
| **RNF1** | Estabilidad | El sistema debe operar durante al menos 10 minutos sin reinicios, bloqueos ni interrupciones del ciclo de medición y visualización. | Prueba continua cronometrada y registro de cualquier reinicio o bloqueo. |
| **RNF2** | Exactitud | El error de medición debe ser como máximo ±3 cm dentro del rango declarado de 2 a 200 cm. | Comparación de las lecturas con distancias marcadas mediante una cinta métrica. |
| **RNF3** | Tiempo de respuesta | El LED debe reflejar un cambio de rango en menos de 1 segundo. | Medición del tiempo desde el cambio físico de distancia hasta la actualización del indicador. |
| **RNF4** | Frecuencia de muestreo | El sistema debe completar al menos 2 lecturas por segundo. | Conteo de lecturas válidas e inválidas durante un intervalo conocido. |
| **RNF5** | Calidad del código | El código debe ser legible, modular, documentado y mantener responsabilidades separadas. | Revisión del código, sus interfaces, nombres, documentación y correspondencia con RF1–RF3. |

## 1.3 Trazabilidad de los requerimientos

La siguiente matriz permite seguir cada requerimiento desde su definición hasta el diseño, la implementación y su verificación.

| Requisito | Elemento de diseño | Evidencia de implementación | Verificación |
| --- | --- | --- | --- |
| **RF1** | Módulo de adquisición y estructura de lectura | <code>SensorUltrasonico::medirDistanciaCm()</code> y <code>LecturaDistancia</code> | <code>PR-SENSOR-001</code> |
| **RF2** | Módulo de clasificación y límites configurables | <code>clasificarDistancia()</code>, <code>RangoDistancia</code> y <code>Config.h</code> | <code>PR-CLASIFICACION-001</code> |
| **RF3** | Conversión de rango y módulo actuador | <code>convertirAEstado()</code>, <code>IndicadorLeds::mostrar()</code> y <code>main.cpp</code> | <code>PR-INDICADOR-001</code> |
| **RNF1** | Timeout del sensor y ciclo de control acotado | <code>_timeoutUs</code>, <code>pulseIn()</code> y <code>loop()</code> | <code>PR-ESTABILIDAD-001</code> |
| **RNF2** | Conversión del tiempo de eco y validación física | <code>VELOCIDAD_SONIDO_CM_US</code> y límites de 2–400 cm del sensor | <code>PR-EXACTITUD-001</code> |
| **RNF3** | Actualización periódica del indicador | Secuencia medir → clasificar → mostrar, registro temporal y espera de 100 ms | <code>PR-ANALISIS-TEMPORAL-001</code> |
| **RNF4** | Ciclo periódico de adquisición | <code>loop()</code>, timeout máximo de 30 000 µs y <code>delay(100)</code> | <code>PR-MUESTREO-001</code> |
| **RNF5** | Arquitectura modular con interfaces limitadas | Clases <code>SensorUltrasonico</code> e <code>IndicadorLeds</code>, función de clasificación y dato compartido | <code>PR-CALIDAD-CODIGO-001</code> |

# 2. Análisis y Diseño

Esta sección transforma los requerimientos anteriores en una solución concreta. La arquitectura presenta los componentes y sus dependencias; el circuito especifica las conexiones físicas; el diagrama estructural muestra la organización del código; y los diagramas de comportamiento describen el orden de ejecución.

## 2.1 Diagrama de arquitectura del sistema

El ESP32 funciona como unidad central. <code>main.cpp</code> coordina los módulos: solicita una medición al sensor, envía la lectura al clasificador y entrega al indicador el estado visual resultante. Las flechas continuas representan llamadas o datos; la flecha discontinua representa configuración.

~~~mermaid
flowchart LR
    subgraph ENTRADA["Hardware de entrada"]
        HC["HC-SR04"]
    end

    subgraph MCU["ESP32"]
        MAIN["main.cpp<br/>coordinación"]
        SENSOR["SensorUltrasonico<br/>adquisición y validación"]
        LECTURA["LecturaDistancia<br/>distanciaCm + valida"]
        CLASIF["ClasificadorDistancia<br/>rango lógico"]
        CONFIG["Config.h<br/>umbrales e histéresis"]
        INDICADOR["IndicadorLeds<br/>salidas digitales"]
    end

    subgraph SALIDA["Salidas del sistema"]
        LEDR["LED rojo"]
        LEDA["LED amarillo"]
        LEDV["LED verde"]
        MONITOR["Monitor serie<br/>datos de diagnóstico"]
    end

    MAIN -->|"solicita medición"| SENSOR
    SENSOR -->|"pulso TRIG"| HC
    HC -->|"pulso ECHO"| SENSOR
    SENSOR -->|"produce"| LECTURA
    LECTURA -->|"regresa a"| MAIN
    MAIN -->|"lectura + rango anterior"| CLASIF
    CONFIG -.->|"límites"| CLASIF
    CLASIF -->|"RangoDistancia"| MAIN
    MAIN -->|"EstadoIndicador"| INDICADOR
    INDICADOR --> LEDR
    INDICADOR --> LEDA
    INDICADOR --> LEDV
    MAIN -->|"tiempo, distancia, validez y rango"| MONITOR
~~~

### 2.1.1 Responsabilidades y datos intercambiados

| Componente | Entrada | Salida | Responsabilidad |
| --- | --- | --- | --- |
| HC-SR04 | Pulso de disparo | Pulso de eco | Detectar el recorrido del sonido. |
| <code>SensorUltrasonico</code> | Duración del eco | <code>LecturaDistancia</code> | Calcular centímetros y marcar la validez física de la lectura. |
| <code>ClasificadorDistancia</code> | Lectura actual y rango anterior | <code>RangoDistancia</code> | Aplicar límites, rango de trabajo e histéresis. |
| <code>main.cpp</code> | Lectura y rango obtenido | <code>EstadoIndicador</code> y registro serie | Coordinar el ciclo, convertir el rango lógico en un estado visual y publicar datos de diagnóstico. |
| <code>IndicadorLeds</code> | <code>EstadoIndicador</code> | Niveles eléctricos en tres GPIO | Apagar los LEDs no seleccionados y encender el correspondiente. |
| Monitor serie | Registro generado por <code>main.cpp</code> | Líneas separadas por comas | Permitir la observación de tiempo, distancia, validez y clasificación. |

## 2.2 Diagrama de circuito

El circuito utiliza alimentación y tierra comunes. Cada LED incorpora una resistencia de 330 Ω para limitar la corriente. La señal ECHO del HC-SR04 no se conecta directamente al GPIO26: pasa por un divisor de tensión que reduce su nivel antes de ingresar al ESP32.

~~~mermaid
flowchart LR
    subgraph ESP["ESP32"]
        V5["5V"]
        GND["GND"]
        P25["GPIO25"]
        P26["GPIO26"]
        P27["GPIO27"]
        P32["GPIO32"]
        P33["GPIO33"]
    end

    subgraph US["HC-SR04"]
        VCC["VCC"]
        TRIG["TRIG"]
        ECHO["ECHO"]
        SGND["GND"]
    end

    RUP["Resistencia superior<br/>del divisor"]
    NODE["Nivel reducido<br/>aprox. 3.3 V"]
    RDOWN["Resistencia inferior<br/>del divisor"]

    RR["330 Ω"]
    RA["330 Ω"]
    RV["330 Ω"]
    LR["LED rojo"]
    LA["LED amarillo"]
    LV["LED verde"]

    V5 --> VCC
    GND --- SGND
    P25 --> TRIG
    ECHO --> RUP --> NODE --> P26
    NODE --> RDOWN --> GND

    P27 --> RR --> LR --> GND
    P32 --> RA --> LA --> GND
    P33 --> RV --> LV --> GND
~~~

| Elemento | Conexión |
| --- | --- |
| HC-SR04 VCC | Pin de 5 V del ESP32 |
| HC-SR04 GND | GND común |
| HC-SR04 TRIG | GPIO25 |
| HC-SR04 ECHO | Divisor de tensión y luego GPIO26 |
| LED rojo | GPIO27 → resistencia de 330 Ω → ánodo; cátodo → GND |
| LED amarillo | GPIO32 → resistencia de 330 Ω → ánodo; cátodo → GND |
| LED verde | GPIO33 → resistencia de 330 Ω → ánodo; cátodo → GND |

## 2.3 Diagrama estructural

El diagrama representa las clases, estructuras, enumeraciones, funciones y dependencias existentes en el código. <code>main.cpp</code> posee las instancias del sensor y del indicador; el sensor crea una lectura; el clasificador utiliza esa lectura y las constantes de configuración; finalmente, el indicador recibe el estado visual.

~~~mermaid
classDiagram
    class Main {
        <<archivo>>
        +setup() void
        +loop() void
        +convertirAEstado(rango) EstadoIndicador
        +rangoATexto(rango) const char*
        -rangoAnterior : RangoDistancia
    }

    class SensorUltrasonico {
        -_pinTrig : uint8_t
        -_pinEcho : uint8_t
        -_timeoutUs : unsigned long
        -VELOCIDAD_SONIDO_CM_US : float
        -DISTANCIA_MIN_CM : float
        -DISTANCIA_MAX_CM : float
        +SensorUltrasonico(pinTrig, pinEcho, timeoutUs)
        +begin() void
        +medirDistanciaCm() LecturaDistancia
    }

    class LecturaDistancia {
        +distanciaCm : float
        +valida : bool
    }

    class ClasificadorDistancia {
        <<módulo>>
        +clasificarDistancia(lectura, rangoAnterior) RangoDistancia
    }

    class Config {
        <<constantes>>
        +UMBRAL_CERCA_MEDIO_CM : float
        +UMBRAL_MEDIO_LEJOS_CM : float
        +MARGEN_HISTERESIS_CM : float
        +DISTANCIA_MINIMA_VALIDA_CM : float
        +DISTANCIA_MAXIMA_VALIDA_CM : float
    }

    class RangoDistancia {
        <<enumeration>>
        CERCANO
        MEDIO
        LEJANO
        ERROR
    }

    class IndicadorLeds {
        -PIN_ROJO : int
        -PIN_AMARILLO : int
        -PIN_VERDE : int
        -apagarTodos() void
        +begin() void
        +mostrar(estado) void
    }

    class EstadoIndicador {
        <<enumeration>>
        Rojo
        Amarillo
        Verde
        Error
    }

    Main *-- SensorUltrasonico : posee
    Main *-- IndicadorLeds : posee
    Main ..> LecturaDistancia : recibe
    Main ..> ClasificadorDistancia : llama
    Main ..> RangoDistancia : conserva
    Main ..> EstadoIndicador : produce
    SensorUltrasonico ..> LecturaDistancia : crea
    ClasificadorDistancia ..> LecturaDistancia : procesa
    ClasificadorDistancia ..> Config : consulta
    ClasificadorDistancia ..> RangoDistancia : devuelve
    IndicadorLeds ..> EstadoIndicador : recibe
~~~

## 2.4 Diagramas de comportamiento

Los dos diagramas siguientes muestran aspectos diferentes. El diagrama de actividad explica las decisiones del algoritmo, mientras que el diagrama de secuencia muestra qué módulo llama a cuál durante un ciclo.

### 2.4.1 Diagrama de actividad

~~~mermaid
flowchart TD
    START(["Encendido o reinicio"])
    SETUP["setup(): iniciar comunicación serie,<br/>sensor e indicador"]
    MEDIR["Solicitar LecturaDistancia"]
    CLASIFICAR["Clasificar usando lectura<br/>y rango anterior"]
    DECISION{"Rango obtenido"}
    CR["Convertir a Rojo"]
    CA["Convertir a Amarillo"]
    CV["Convertir a Verde"]
    CE["Convertir a Error"]
    MOSTRAR["Apagar todos y encender<br/>solo el LED correspondiente"]
    REGISTRAR["Enviar tiempo, distancia,<br/>validez y rango por Serial"]
    ESPERAR["Esperar 100 ms"]

    START --> SETUP --> MEDIR --> CLASIFICAR --> DECISION
    DECISION -->|CERCANO| CR
    DECISION -->|MEDIO| CA
    DECISION -->|LEJANO| CV
    DECISION -->|"ERROR"| CE
    CR --> MOSTRAR
    CA --> MOSTRAR
    CV --> MOSTRAR
    CE --> MOSTRAR
    MOSTRAR --> REGISTRAR --> ESPERAR --> MEDIR
~~~

El estado <code>ERROR</code> incluye la ausencia de eco, una lectura físicamente inválida o una distancia fuera del rango de trabajo de 2–200 cm. En ese estado, la operación “mostrar” apaga los tres LEDs.

### 2.4.2 Diagrama de secuencia

~~~mermaid
sequenceDiagram
    participant Arduino
    participant Main as main.cpp
    participant Sensor as SensorUltrasonico
    participant HC as HC-SR04
    participant Clasificador as ClasificadorDistancia
    participant Indicador as IndicadorLeds
    participant Monitor as Monitor serie

    Arduino->>Main: ejecutar loop()
    Main->>Sensor: medirDistanciaCm()
    Sensor->>HC: pulso TRIG de 10 µs
    HC-->>Sensor: pulso ECHO
    Sensor-->>Main: LecturaDistancia
    Main->>Clasificador: clasificarDistancia(lectura, rangoAnterior)
    Clasificador-->>Main: RangoDistancia
    Main->>Main: convertirAEstado(rango)
    Main->>Indicador: mostrar(estado)
    Indicador-->>Main: salidas actualizadas
    Main->>Monitor: tiempo, distancia, validez, rango
    Main-->>Arduino: fin del ciclo
~~~

## 2.5 Decisiones principales de diseño

- **Separación de responsabilidades:** la adquisición, la decisión y la actuación se encuentran en módulos diferentes.
- **Dato compartido único:** <code>LecturaDistancia</code> evita duplicar estructuras incompatibles entre sensor y clasificador.
- **Configuración centralizada:** los umbrales del comportamiento se modifican desde <code>Config.h</code>.
- **Histéresis:** el clasificador utiliza el rango anterior para reducir cambios inestables cerca de 20 y 40 cm.
- **Fallo seguro:** una lectura inválida conduce a <code>ERROR</code> y mantiene los tres LEDs apagados.
- **Observabilidad:** el registro serie permite verificar las lecturas sin alterar la decisión ni el control de los LEDs.
- **Coordinador simple:** <code>main.cpp</code> conecta los módulos sin implementar directamente la medición ultrasónica ni la escritura individual de los GPIO de los LEDs.

# 3. Desarrollo e Implementación

## 3.1 Entorno de desarrollo

El firmware fue desarrollado en C++ con el framework Arduino y organizado como un proyecto PlatformIO.

| Elemento | Selección | Uso |
| --- | --- | --- |
| Microcontrolador | ESP32 Dev Module | Procesamiento y control de entradas y salidas. |
| Framework | Arduino | Funciones de GPIO, temporización y medición de pulsos. |
| Lenguaje | C++ | Clases, estructuras, enumeraciones y funciones del firmware. |
| Herramienta de construcción | PlatformIO | Configuración de la plataforma y generación del firmware. |

La configuración está declarada en [<code>platformio.ini</code>](../platformio.ini):

~~~ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
~~~

## 3.2 Organización del código fuente

Los encabezados <code>.h</code> definen las interfaces y los tipos compartidos. Los archivos <code>.cpp</code> contienen las operaciones que realizan la medición, la clasificación, el control de los actuadores y la coordinación.

| Archivo | Responsabilidad |
| --- | --- |
| [<code>src/main.cpp</code>](../src/main.cpp) | Inicialización y coordinación del ciclo completo. |
| [<code>src/LecturaDistancia.h</code>](../src/LecturaDistancia.h) | Formato común de una medición. |
| [<code>src/SensorUltrasonico.h</code>](../src/SensorUltrasonico.h) | Interfaz, pines y parámetros privados del sensor. |
| [<code>src/SensorUltrasonico.cpp</code>](../src/SensorUltrasonico.cpp) | Disparo, recepción del eco, cálculo y validación física. |
| [<code>src/ClasificadorDistancia.h</code>](../src/ClasificadorDistancia.h) | Rangos disponibles e interfaz de clasificación. |
| [<code>src/ClasificadorDistancia.cpp</code>](../src/ClasificadorDistancia.cpp) | Validación del rango de trabajo, clasificación e histéresis. |
| [<code>src/Config.h</code>](../src/Config.h) | Umbrales y límites configurables del clasificador. |
| [<code>src/IndicadorLeds.h</code>](../src/IndicadorLeds.h) | Estados visuales, pines e interfaz del indicador. |
| [<code>src/IndicadorLeds.cpp</code>](../src/IndicadorLeds.cpp) | Configuración de salidas y activación exclusiva de LEDs. |

## 3.3 Adquisición y representación de la lectura

### 3.3.1 Estructura compartida

La medición se representa con una estructura que conserva tanto el valor como su confiabilidad:

~~~cpp
struct LecturaDistancia {
    float distanciaCm;
    bool valida;
};
~~~

<code>distanciaCm</code> guarda el resultado en centímetros y <code>valida</code> indica si puede utilizarse. Esta estructura es producida por el sensor y recibida sin conversiones por el clasificador.

### 3.3.2 Clase <code>SensorUltrasonico</code>

El objeto se crea en <code>main.cpp</code> sin argumentos, por lo que adopta los valores predeterminados definidos en su constructor:

~~~cpp
SensorUltrasonico(
    uint8_t pinTrig = 25,
    uint8_t pinEcho = 26,
    unsigned long timeoutUs = 30000
);
~~~

El método <code>begin()</code> prepara los pines una sola vez:

~~~cpp
void SensorUltrasonico::begin() {
    pinMode(_pinTrig, OUTPUT);
    pinMode(_pinEcho, INPUT);
    digitalWrite(_pinTrig, LOW);
}
~~~

El método <code>medirDistanciaCm()</code> inicia cada lectura con <code>distanciaCm = 0.0</code> y <code>valida = false</code>. Después:

1. Mantiene TRIG en nivel bajo durante 2 µs.
2. Envía por TRIG un pulso alto de 10 µs.
3. Mide mediante <code>pulseIn()</code> la duración del pulso ECHO.
4. Si el tiempo recibido es cero, devuelve la lectura inválida.
5. Calcula la distancia utilizando la velocidad aproximada del sonido.
6. Acepta físicamente valores de 2 a 400 cm.

La conversión utilizada es:

~~~cpp
float distancia =
    (duracionUs * VELOCIDAD_SONIDO_CM_US) / 2.0f;
~~~

<code>VELOCIDAD_SONIDO_CM_US</code> vale <code>0.0343</code>. La división entre dos corresponde al viaje de ida y vuelta del sonido. La lectura se marca como válida únicamente después de superar todas las comprobaciones:

~~~cpp
if (duracionUs == 0) {
    return lectura;
}

if (distancia < DISTANCIA_MIN_CM ||
    distancia > DISTANCIA_MAX_CM) {
    return lectura;
}

lectura.distanciaCm = distancia;
lectura.valida = true;
return lectura;
~~~

Esta implementación proporciona la evidencia de código para **RF1**.

## 3.4 Clasificación de la distancia

La interfaz pública define cuatro resultados posibles y una función que recibe la lectura actual junto con el rango anterior:

~~~cpp
enum class RangoDistancia {
    CERCANO,
    MEDIO,
    LEJANO,
    ERROR
};

RangoDistancia clasificarDistancia(
    const LecturaDistancia& lectura,
    RangoDistancia rangoAnterior
);
~~~

Los valores configurables son:

~~~cpp
constexpr float UMBRAL_CERCA_MEDIO_CM = 20.0f;
constexpr float UMBRAL_MEDIO_LEJOS_CM = 40.0f;
constexpr float MARGEN_HISTERESIS_CM = 2.0f;
constexpr float DISTANCIA_MINIMA_VALIDA_CM = 2.0f;
constexpr float DISTANCIA_MAXIMA_VALIDA_CM = 200.0f;
~~~

El clasificador aplica las comprobaciones en este orden:

1. Si <code>lectura.valida</code> es falsa, devuelve <code>ERROR</code>.
2. Si la distancia queda fuera de 2–200 cm, devuelve <code>ERROR</code>.
3. Si no existe un rango anterior válido, clasifica directamente con los límites de 20 y 40 cm.
4. Si existe un rango anterior, aplica el margen de histéresis declarado en la sección 1.1.1.

La clasificación base está aislada en una función interna:

~~~cpp
RangoDistancia clasificarPorUmbrales(float distanciaCm) {
    if (distanciaCm < UMBRAL_CERCA_MEDIO_CM) {
        return RangoDistancia::CERCANO;
    }
    if (distanciaCm < UMBRAL_MEDIO_LEJOS_CM) {
        return RangoDistancia::MEDIO;
    }
    return RangoDistancia::LEJANO;
}
~~~

La histéresis se implementa con una selección basada en <code>rangoAnterior</code>. Por ejemplo, cuando el estado previo es <code>MEDIO</code>:

~~~cpp
case RangoDistancia::MEDIO:
    if (distancia <
        UMBRAL_CERCA_MEDIO_CM - MARGEN_HISTERESIS_CM) {
        return RangoDistancia::CERCANO;
    }
    if (distancia >=
        UMBRAL_MEDIO_LEJOS_CM + MARGEN_HISTERESIS_CM) {
        return RangoDistancia::LEJANO;
    }
    return RangoDistancia::MEDIO;
~~~

De esta manera, una pequeña variación alrededor de un límite no cambia inmediatamente el resultado, pero toda llamada sigue produciendo exactamente un rango. Esta implementación proporciona la evidencia de código para **RF2**.

## 3.5 Control de los LEDs

El indicador recibe uno de cuatro estados:

~~~cpp
enum class EstadoIndicador {
    Rojo,
    Amarillo,
    Verde,
    Error
};
~~~

Los pines GPIO27, GPIO32 y GPIO33 están encapsulados como constantes privadas de la clase. El método <code>begin()</code> configura las salidas y deja los tres LEDs apagados:

~~~cpp
void IndicadorLeds::begin() {
    pinMode(PIN_ROJO, OUTPUT);
    pinMode(PIN_AMARILLO, OUTPUT);
    pinMode(PIN_VERDE, OUTPUT);
    apagarTodos();
}
~~~

Antes de encender un color, <code>mostrar()</code> llama a <code>apagarTodos()</code>. Esta secuencia garantiza que no permanezcan activos dos colores después de una transición:

~~~cpp
void IndicadorLeds::mostrar(EstadoIndicador estado) {
    apagarTodos();

    switch (estado) {
        case EstadoIndicador::Rojo:
            digitalWrite(PIN_ROJO, HIGH);
            break;
        case EstadoIndicador::Amarillo:
            digitalWrite(PIN_AMARILLO, HIGH);
            break;
        case EstadoIndicador::Verde:
            digitalWrite(PIN_VERDE, HIGH);
            break;
        case EstadoIndicador::Error:
            break;
    }
}
~~~

En <code>Error</code> no se ejecuta ningún encendido, por lo que los tres LEDs permanecen apagados.

## 3.6 Integración en <code>main.cpp</code>

El archivo principal incluye las tres interfaces y crea las instancias necesarias:

~~~cpp
SensorUltrasonico sensor;
IndicadorLeds indicador;
RangoDistancia rangoAnterior = RangoDistancia::ERROR;
~~~

<code>rangoAnterior</code> comienza en <code>ERROR</code> porque al encender la placa todavía no existe una lectura previa. Posteriormente conserva el resultado de cada ciclo para aplicar la histéresis en la medición siguiente.

La inicialización se ejecuta una sola vez. Además de preparar el sensor y los LEDs, configura el puerto serie a la misma velocidad declarada en <code>platformio.ini</code>:

~~~cpp
void setup() {
    Serial.begin(115200);
    sensor.begin();
    indicador.begin();
}
~~~

La relación entre rango lógico y salida visual está definida por <code>convertirAEstado()</code>:

| Rango recibido | Estado devuelto |
| --- | --- |
| <code>CERCANO</code> | <code>Rojo</code> |
| <code>MEDIO</code> | <code>Amarillo</code> |
| <code>LEJANO</code> | <code>Verde</code> |
| <code>ERROR</code> | <code>Error</code> |

La función <code>rangoATexto()</code> convierte el valor de <code>RangoDistancia</code> en una etiqueta legible. Finalmente, <code>loop()</code> conecta los módulos en el orden diseñado y registra el resultado de cada ciclo:

~~~cpp
void loop() {
    LecturaDistancia lectura = sensor.medirDistanciaCm();

    rangoAnterior =
        clasificarDistancia(lectura, rangoAnterior);

    EstadoIndicador estado =
        convertirAEstado(rangoAnterior);

    indicador.mostrar(estado);

    Serial.print(millis());
    Serial.print(",");
    Serial.print(lectura.distanciaCm, 2);
    Serial.print(",");
    Serial.print(lectura.valida ? "true" : "false");
    Serial.print(",");
    Serial.println(rangoATexto(rangoAnterior));

    delay(100);
}
~~~

Cada línea enviada presenta el formato <code>tiempo_ms,distancia_cm,valida,rango</code>. Por ejemplo, <code>4250,31.11,true,MEDIO</code> indica que, a los 4 250 ms de funcionamiento, se obtuvo una lectura válida de 31,11 cm clasificada como <code>MEDIO</code>. Esta salida facilita las pruebas y no participa en la decisión del actuador.

El archivo principal no genera directamente el pulso ultrasónico, no contiene los límites de clasificación y no escribe directamente en los pines de los LEDs. Su responsabilidad es coordinar las interfaces públicas y publicar el registro de diagnóstico, completando la implementación de **RF3** y aportando evidencia para las validaciones.

## 3.7 Calidad y mantenibilidad del código

Las prácticas aplicadas en la implementación son:

- **Responsabilidad única:** cada módulo atiende una parte del problema.
- **Encapsulamiento:** los pines y parámetros internos de las clases son privados.
- **Dato compartido:** <code>LecturaDistancia</code> evita tipos duplicados entre módulos.
- **Estados explícitos:** las enumeraciones reemplazan números sin significado.
- **Configuración centralizada:** los límites de la aplicación se encuentran en <code>Config.h</code>.
- **Constantes protegidas:** <code>constexpr</code> impide modificaciones accidentales.
- **Manejo de errores:** una medición no confiable produce un estado conocido.
- **Salida exclusiva:** el indicador apaga todos los LEDs antes de seleccionar uno.
- **Interfaces limitadas:** <code>main.cpp</code> usa operaciones públicas y no accede a los detalles privados.
- **Nombres descriptivos:** funciones como <code>medirDistanciaCm()</code>, <code>clasificarDistancia()</code> y <code>apagarTodos()</code> expresan su propósito.

La implementación también incorpora decisiones que apoyan los requerimientos no funcionales: el timeout limita la espera ante ausencia de eco, la histéresis busca reducir oscilaciones y la espera de 100 ms establece una actualización periódica. La salida serie proporciona los valores utilizados para comprobar exactitud, respuesta y frecuencia de muestreo en la sección siguiente.

# 4. Pruebas y Validaciones

## 4.1 Objetivo y estrategia

El plan de pruebas tiene como objetivo comprobar de manera trazable los requerimientos RF1–RF3 y RNF1–RNF5 definidos en la sección 1. Las verificaciones se dividen en tres grupos para distinguir las fuentes de evidencia:

1. **Pruebas de software:** revisión estática, compilación del firmware y pruebas automatizadas del clasificador.
2. **Pruebas experimentales:** medición del sensor, respuesta de los LEDs, exactitud, estabilidad e histéresis con el circuito montado.
3. **Análisis cuantitativo:** cálculo de errores, frecuencia de muestreo y duración estimada del ciclo.

Una prueba se registra como aprobada únicamente cuando satisface su criterio de aceptación. Las desviaciones observadas se conservan y se analizan, como ocurre en la secuencia H04 de la prueba de histéresis.

## 4.2 Plan general de pruebas

| Identificador | Tipo | Requisitos cubiertos | Objetivo | Estado |
| --- | --- | --- | --- | --- |
| <code>PR-COMPILACION-001</code> | Software, ejecutada | RNF5 | Verificar que todos los módulos puedan compilarse y enlazarse para el ESP32. | **Aprobada** |
| <code>PR-CLASIFICACION-001</code> | Automatizada, ejecutada | RF2, RNF5 | Validar rangos, límites, errores, recuperación e histéresis sin depender del sensor físico. | **Aprobada: 20/20** |
| <code>PR-CALIDAD-CODIGO-001</code> | Estática, ejecutada | RNF5 | Comprobar separación de responsabilidades, interfaces y manejo seguro del indicador. | **Aprobada: 7/7** |
| <code>PR-ANALISIS-TEMPORAL-001</code> | Analítica | RNF3, RNF4 | Comprobar la compatibilidad del ciclo con los objetivos de respuesta y muestreo. | **Aprobada** |
| <code>PR-SENSOR-001</code> | Experimental | RF1 | Confirmar que el HC-SR04 produce lecturas coherentes en distintas distancias. | **Aprobada** |
| <code>PR-INDICADOR-001</code> | Experimental | RF3 | Confirmar la correspondencia entre distancia, rango y LED, incluida la condición de error. | **Aprobada: 5/5** |
| <code>PR-HISTERESIS-001</code> | Experimental | RF2 | Observar el comportamiento cerca de los umbrales. | **No aprobada: 5/6** |
| <code>PR-EXACTITUD-001</code> | Experimental y cuantitativa | RNF2 | Calcular el error de medición respecto a una cinta métrica. | **Aprobada** |
| <code>PR-MUESTREO-001</code> | Experimental y cuantitativa | RNF4 | Medir cuántas lecturas completa el sistema por segundo. | **Aprobada** |
| <code>PR-ESTABILIDAD-001</code> | Experimental | RNF1 | Comprobar 10 minutos de operación continua sin reinicios ni bloqueos. | **Aprobada** |

## 4.3 Entorno de las pruebas de software

La verificación final del software se ejecutó el 14 de septiembre de 2026 con las siguientes herramientas:

| Elemento | Configuración |
| --- | --- |
| Gestor del proyecto | PlatformIO Core 6.2.0 |
| Compilación del firmware | Entorno <code>esp32dev</code>, framework Arduino |
| Pruebas automatizadas | Entorno <code>native</code> ejecutado en el equipo de desarrollo |
| Framework de pruebas | Unity 2.6.1 |
| Archivo probado automáticamente | <code>src/ClasificadorDistancia.cpp</code> |
| Archivo de casos de prueba | <code>test/test_clasificacion/test_main.cpp</code> |

Se añadió un entorno nativo en [<code>platformio.ini</code>](../platformio.ini) para ejecutar la lógica que no depende del hardware:

~~~ini
[env:native]
platform = native
test_framework = unity
test_build_src = yes
build_src_filter = +<ClasificadorDistancia.cpp>
~~~

La selección de un único archivo fuente es deliberada: el clasificador solo procesa datos y puede probarse en una computadora, mientras que <code>SensorUltrasonico</code> e <code>IndicadorLeds</code> necesitan las funciones y conexiones físicas del ESP32.

## 4.4 Pruebas de software ejecutadas

### 4.4.1 <code>PR-COMPILACION-001</code> — Compilación completa para ESP32

**Propósito:** comprobar que las declaraciones, implementaciones e integraciones del firmware son aceptadas por la plataforma de destino.

**Procedimiento:** ejecutar desde la raíz del proyecto:

~~~text
platformio run -e esp32dev
~~~

**Criterio de aceptación:** la construcción debe finalizar con estado <code>SUCCESS</code>, sin errores de compilación ni de enlazado.

**Resultado obtenido:** aprobado.

| Indicador | Resultado |
| --- | ---: |
| Entornos construidos correctamente | 1 de 1 |
| Estado de PlatformIO | <code>SUCCESS</code> |
| RAM utilizada | 21 472 bytes de 327 680 bytes (6,6 %) |
| Memoria flash utilizada | 270 889 bytes de 1 310 720 bytes (20,7 %) |

La compilación incluyó <code>ClasificadorDistancia.cpp</code>, <code>IndicadorLeds.cpp</code>, <code>SensorUltrasonico.cpp</code> y <code>main.cpp</code>, incluida la salida serie incorporada al programa actual.

### 4.4.2 <code>PR-CLASIFICACION-001</code> — Pruebas automatizadas

**Propósito:** verificar la clasificación con valores controlados, especialmente aquellos difíciles de producir exactamente con el HC-SR04, como 20,00; 22,00; 38,00; 40,00 y 42,00 cm.

**Procedimiento:** ejecutar:

~~~text
platformio test -e native
~~~

**Criterio de aceptación:** los 20 casos deben aprobar y los límites configurables deben conservar un orden válido.

Los casos implementados en [<code>test/test_clasificacion/test_main.cpp</code>](../test/test_clasificacion/test_main.cpp) fueron:

| Caso | Lectura | Válida | Rango anterior | Resultado esperado | Resultado |
| --- | ---: | :---: | --- | --- | --- |
| C01 | 15,00 cm | No | <code>CERCANO</code> | <code>ERROR</code> | **Aprobado** |
| C02 | 1,99 cm | Sí | <code>ERROR</code> | <code>ERROR</code> | **Aprobado** |
| C03 | 2,00 cm | Sí | <code>ERROR</code> | <code>CERCANO</code> | **Aprobado** |
| C04 | 19,99 cm | Sí | <code>ERROR</code> | <code>CERCANO</code> | **Aprobado** |
| C05 | 20,00 cm | Sí | <code>ERROR</code> | <code>MEDIO</code> | **Aprobado** |
| C06 | 39,99 cm | Sí | <code>ERROR</code> | <code>MEDIO</code> | **Aprobado** |
| C07 | 40,00 cm | Sí | <code>ERROR</code> | <code>LEJANO</code> | **Aprobado** |
| C08 | 200,00 cm | Sí | <code>ERROR</code> | <code>LEJANO</code> | **Aprobado** |
| C09 | 200,01 cm | Sí | <code>ERROR</code> | <code>ERROR</code> | **Aprobado** |
| C10 | 21,99 cm | Sí | <code>CERCANO</code> | <code>CERCANO</code> | **Aprobado** |
| C11 | 22,00 cm | Sí | <code>CERCANO</code> | <code>MEDIO</code> | **Aprobado** |
| C12 | 100,00 cm | Sí | <code>CERCANO</code> | <code>LEJANO</code> | **Aprobado** |
| C13 | 18,00 cm | Sí | <code>MEDIO</code> | <code>MEDIO</code> | **Aprobado** |
| C14 | 17,99 cm | Sí | <code>MEDIO</code> | <code>CERCANO</code> | **Aprobado** |
| C15 | 41,99 cm | Sí | <code>MEDIO</code> | <code>MEDIO</code> | **Aprobado** |
| C16 | 42,00 cm | Sí | <code>MEDIO</code> | <code>LEJANO</code> | **Aprobado** |
| C17 | 38,00 cm | Sí | <code>LEJANO</code> | <code>LEJANO</code> | **Aprobado** |
| C18 | 37,99 cm | Sí | <code>LEJANO</code> | <code>MEDIO</code> | **Aprobado** |
| C19 | 10,00 cm | Sí | <code>LEJANO</code> | <code>CERCANO</code> | **Aprobado** |
| C20 | 25,00 cm | Sí | <code>ERROR</code> | <code>MEDIO</code> | **Aprobado** |

**Resultado consolidado:** 20 casos ejecutados, 20 aprobados y 0 fallidos. También aprobaron cuatro comprobaciones realizadas durante la compilación del test: mínimo < primer umbral < segundo umbral < máximo, y margen de histéresis no negativo.

### 4.4.3 <code>PR-CALIDAD-CODIGO-001</code> — Revisión estática

**Propósito:** verificar propiedades de organización que no requieren encender la placa.

| Caso | Comprobación | Evidencia revisada | Resultado |
| --- | --- | --- | --- |
| E01 | Existe un único tipo compartido para la medición. | No existe <code>LecturaSensor</code>; sensor y clasificador incluyen <code>LecturaDistancia.h</code>. | **Aprobado** |
| E02 | El ciclo principal integra los tres módulos. | <code>main.cpp</code> llama a <code>medirDistanciaCm()</code>, <code>clasificarDistancia()</code> y <code>mostrar()</code>. | **Aprobado** |
| E03 | <code>main.cpp</code> no controla directamente los GPIO. | No contiene <code>pinMode()</code>, <code>digitalWrite()</code> ni <code>pulseIn()</code>. | **Aprobado** |
| E04 | El clasificador es independiente del hardware. | No incluye Arduino ni utiliza pines, esperas o lectura de pulsos. | **Aprobado** |
| E05 | Los umbrales están ordenados. | Comprobaciones <code>static_assert</code> ejecutadas al compilar las pruebas. | **Aprobado** |
| E06 | Solo puede quedar encendido el LED seleccionado. | <code>mostrar()</code> ejecuta <code>apagarTodos()</code> antes de evaluar el estado. | **Aprobado** |
| E07 | El error produce una salida segura y definida. | El caso <code>EstadoIndicador::Error</code> no ejecuta ningún encendido después de apagar las salidas. | **Aprobado** |

**Resultado consolidado:** 7 verificaciones realizadas y 7 aprobadas.

### 4.4.4 <code>PR-ANALISIS-TEMPORAL-001</code> — Comprobación analítica

El ciclo incorpora una espera de 100 ms y el sensor limita la espera del eco a 30 ms. Sin considerar el pequeño tiempo adicional de procesamiento, un ciclo desfavorable se aproxima a:

~~~text
100 ms de espera + 30 ms de timeout = 130 ms por ciclo
1 000 ms / 130 ms ≈ 7,69 ciclos por segundo
~~~

Este valor teórico es compatible con RNF4 (al menos 2 lecturas por segundo). La medición, la clasificación y la actualización del LED ocurren dentro del mismo ciclo y antes de imprimir el registro serie. Por ello, un cambio físico detectado debería reflejarse aproximadamente en el siguiente ciclo, por debajo del objetivo de 1 segundo definido en RNF3.

La prueba de muestreo registró 74 ciclos en 10 segundos, equivalentes a un periodo medio de aproximadamente 135 ms. Este resultado es coherente con la estimación de 130 ms más el tiempo de procesamiento y transmisión. RNF3 se valida mediante el análisis del flujo y el registro temporal del sistema.

## 4.5 Configuración de las pruebas experimentales

### 4.5.1 Equipo necesario

- ESP32 con el firmware cargado.
- HC-SR04 conectado mediante divisor de tensión en ECHO.
- Tres LEDs con sus resistencias limitadoras de 330 Ω.
- Objeto plano y estable, colocado perpendicularmente al sensor.
- Cinta métrica o regla con resolución mínima de 1 mm.
- Cronómetro para la prueba de estabilidad.
- Computadora con PlatformIO para registrar las lecturas cuando la prueba necesite valores numéricos.

### 4.5.2 Condiciones de ensayo

- Realizar las mediciones en un espacio interior sin obstáculos cercanos al cono de detección.
- Mantener firmes el sensor, la cinta métrica y el objeto.
- Medir desde la cara frontal de los transductores del HC-SR04 hasta la superficie del objeto.
- Esperar a que la lectura se estabilice antes de registrar cada posición.
- Reiniciar el ESP32 antes de los casos que comprueben exactamente el rango inicial.
- Registrar las lecturas y cualquier comportamiento anormal.

### 4.5.3 Instrumentación para valores numéricos

El firmware actual comunica el resultado mediante LEDs y, además, envía por el monitor serie a 115 200 baudios los siguientes datos por ciclo:

~~~text
tiempo_ms, distancia_cm, valida, rango
~~~

El campo <code>tiempo_ms</code> permite contar ciclos; <code>distancia_cm</code> contiene la medición; <code>valida</code> indica si la lectura es utilizable; y <code>rango</code> muestra la clasificación aplicada. Esta instrumentación permite observar y registrar resultados sin modificar los umbrales, la histéresis ni el control de los LEDs.

## 4.6 Pruebas experimentales ejecutadas

### 4.6.1 <code>PR-SENSOR-001</code> y <code>PR-EXACTITUD-001</code>

**Requisitos:** RF1 y RNF2.

**Objetivo:** confirmar que la distancia medida cambia de forma coherente y que el error máximo es ≤ ±3 cm.

**Procedimiento:**

1. Abrir el monitor serie con la configuración descrita en 4.5.3.
2. Colocar el objeto en 10, 20, 30, 40, 60, 100, 150 y 200 cm.
3. En cada posición, esperar la estabilización y registrar tres lecturas consecutivas.
4. Calcular el promedio de las tres lecturas.
5. Calcular el error absoluto: <code>|promedio medido − distancia de referencia|</code>.

**Criterios de aceptación:** todas las posiciones deben producir una lectura válida; las mediciones deben aumentar al alejar el objeto; y el error absoluto máximo debe ser ≤ 3 cm.

| Referencia | Lectura 1 | Lectura 2 | Lectura 3 | Promedio | Error absoluto | Estado |
| ---: | ---: | ---: | ---: | ---: | ---: | :---: |
| 10 cm | 11,20 cm | 10,75 cm | 10,56 cm | 10,84 cm | 0,84 cm | **Cumple** |
| 20 cm | 21,43 cm | 22,19 cm | 21,27 cm | 21,63 cm | 1,63 cm | **Cumple** |
| 30 cm | 31,11 cm | 31,50 cm | 30,73 cm | 31,11 cm | 1,11 cm | **Cumple** |
| 40 cm | 40,51 cm | 40,41 cm | 40,26 cm | 40,39 cm | 0,39 cm | **Cumple** |
| 60 cm | 60,02 cm | 60,97 cm | 60,32 cm | 60,44 cm | 0,44 cm | **Cumple** |
| 100 cm | 100,24 cm | 100,02 cm | 100,25 cm | 100,17 cm | 0,17 cm | **Cumple** |
| 150 cm | 149,98 cm | 150,79 cm | 149,77 cm | 150,18 cm | 0,18 cm | **Cumple** |
| 200 cm | 201,36 cm | 200,23 cm | 199,39 cm | 200,33 cm | 0,33 cm | **Cumple** |

Las 24 lecturas fueron coherentes con el incremento de la distancia. El error absoluto máximo fue de **1,63 cm**, obtenido en la referencia de 20 cm, y el promedio de los ocho errores absolutos fue de **0,64 cm**. Ambos resultados se encuentran dentro del límite de 3 cm; por tanto, RF1 y RNF2 quedan validados en las posiciones ensayadas.

### 4.6.2 <code>PR-INDICADOR-001</code>

**Requisito:** RF3.

**Objetivo:** comprobar el comportamiento conjunto sensor → clasificador → indicador.

| Caso | Preparación | Resultado esperado | Resultado observado | Estado |
| --- | --- | --- | --- | --- |
| I01 | Objeto estable a 10 cm | Solo LED rojo encendido. | Solo LED rojo encendido. | **Aprobado** |
| I02 | Objeto estable a 30 cm | Solo LED amarillo encendido. | Solo LED amarillo encendido. | **Aprobado** |
| I03 | Objeto estable a 60 cm | Solo LED verde encendido. | Solo LED verde encendido. | **Aprobado** |
| I04 | Retirar el objeto para provocar timeout | Los tres LEDs apagados. | Los tres LEDs apagados. | **Aprobado** |
| I05 | Objeto estable por encima de 200 cm y dentro del alcance físico | Los tres LEDs apagados por estar fuera del rango de trabajo. | Los tres LEDs apagados por estar fuera del rango de trabajo. | **Aprobado** |

**Procedimiento:** colocar el objeto en cada condición, esperar al menos un segundo y observar los tres LEDs. En ningún caso válido pueden permanecer encendidos dos LEDs simultáneamente.

**Criterio de aceptación:** los cinco casos deben coincidir con la salida esperada.

**Resultado:** los cinco casos coincidieron con el comportamiento esperado y no se observó más de un LED encendido simultáneamente. RF3 queda validado.

### 4.6.3 <code>PR-HISTERESIS-001</code>

**Requisito:** RF2.

**Objetivo:** confirmar que pequeñas variaciones alrededor de 20 y 40 cm no producen oscilación continua del indicador.

| Secuencia | Estado inicial | Distancias aplicadas | Resultado esperado | Resultado observado | Estado |
| --- | --- | --- | --- | --- | --- |
| H01 | <code>CERCANO</code> | 19 → 21 → 19 → 21 cm | Permanece rojo. | Permaneció rojo. | **Aprobada** |
| H02 | <code>CERCANO</code> | 21 → 22 cm | Rojo en 21 cm y amarillo en 22 cm. | Coincidió con lo esperado. | **Aprobada** |
| H03 | <code>MEDIO</code> | 20 → 19 → 18 → 17 cm | Amarillo hasta 18 cm y rojo en 17 cm. | Coincidió con lo esperado. | **Aprobada** |
| H04 | <code>MEDIO</code> | 39 → 41 → 39 → 41 cm | Permanece amarillo. | Osciló entre amarillo y verde. | **No aprobada** |
| H05 | <code>MEDIO</code> | 41 → 42 cm | Amarillo en 41 cm y verde en 42 cm. | Coincidió con lo esperado. | **Aprobada** |
| H06 | <code>LEJANO</code> | 40 → 39 → 38 → 37 cm | Verde hasta 38 cm y amarillo en 37 cm. | Coincidió con lo esperado. | **Aprobada** |

**Procedimiento:** utilizar el valor mostrado por la instrumentación de diagnóstico para posicionar el objeto, aplicar cada secuencia lentamente y registrar el color observado después de cada lectura.

**Criterio de aceptación:** las seis secuencias deben mantener o cambiar el estado en los puntos indicados, sin parpadeo alternado mientras el objeto permanece dentro de la banda de histéresis.

**Resultado:** se aprobaron cinco de las seis secuencias; por tanto, la prueba no satisface por completo su criterio de aceptación. La lógica aislada superó los casos automatizados equivalentes, por lo que H04 no demuestra un error en las comparaciones del clasificador. La oscilación observada es compatible con variaciones físicas de la lectura que podrían atravesar el límite de transición de 42 cm, aunque sería necesario registrar una serie más extensa alrededor de 40 cm para confirmar la causa.

### 4.6.4 <code>PR-MUESTREO-001</code>

**Requisito:** RNF4.

**Objetivo:** verificar que el sistema completa al menos dos lecturas por segundo.

**Procedimiento:** con la salida de diagnóstico activa, registrar durante 10 segundos todas las líneas generadas. Dividir la cantidad de lecturas registradas entre el tiempo efectivo del registro.

~~~text
frecuencia = cantidad de lecturas / duración en segundos
~~~

| Duración | Lecturas registradas | Frecuencia calculada | Criterio | Estado |
| ---: | ---: | ---: | ---: | --- |
| 10 s | 74 | 7,4 lecturas/s | ≥ 2 lecturas/s | **Aprobada** |

La frecuencia observada equivale a 3,7 veces el mínimo requerido, por lo que RNF4 queda validado.

### 4.6.5 <code>PR-ESTABILIDAD-001</code>

**Requisito:** RNF1.

**Objetivo:** comprobar operación continua durante al menos 10 minutos.

**Procedimiento:**

1. Encender el sistema e iniciar un cronómetro.
2. Mantenerlo funcionando durante 10 minutos sin reiniciarlo.
3. Cada dos minutos, colocar el objeto sucesivamente en un rango diferente y comprobar la respuesta.
4. Registrar reinicios, bloqueos, lecturas detenidas, LEDs incorrectos o calentamiento anormal.

| Tiempo | Condición aplicada | Respuesta esperada | Respuesta observada | Incidencias |
| ---: | --- | --- | --- | --- |
| 0 min | 10 cm | Rojo | Rojo | Ninguna |
| 2 min | 30 cm | Amarillo | Amarillo | Ninguna |
| 4 min | 60 cm | Verde | Verde | Ninguna |
| 6 min | Sin eco | Todos apagados | Todos apagados | Ninguna |
| 8 min | 10 cm | Rojo | Rojo | Ninguna |
| 10 min | 60 cm | Verde | Verde | Ninguna |

**Criterio de aceptación:** completar los 10 minutos sin reinicios ni bloqueos y responder correctamente en los seis puntos de observación.

**Resultado:** el sistema completó los 10 minutos sin reinicios ni bloqueos y respondió correctamente en los seis puntos de observación. RNF1 queda validado.

## 4.7 Validación consolidada de los requerimientos

| Requisito | Evidencia principal | Resultado |
| --- | --- | --- |
| **RF1** | 24 lecturas en ocho distancias, todas válidas y coherentes. | **Cumple** |
| **RF2** | 20/20 casos automatizados aprobados; 5/6 secuencias físicas de histéresis aprobadas. | **Cumple en la lógica; presenta la observación H04 en el sistema físico** |
| **RF3** | 5/5 condiciones del indicador aprobadas, incluidos timeout y fuera de rango. | **Cumple** |
| **RNF1** | 10 minutos de operación, seis controles y ninguna incidencia. | **Cumple** |
| **RNF2** | Error máximo de 1,63 cm frente al límite de 3 cm. | **Cumple** |
| **RNF3** | Ciclo estimado de 130 ms y periodo medio observado de aproximadamente 135 ms frente al límite de 1 s. | **Cumple mediante validación temporal** |
| **RNF4** | 7,4 lecturas/s frente al mínimo de 2 lecturas/s. | **Cumple** |
| **RNF5** | Compilación correcta, 20/20 pruebas automatizadas y 7/7 verificaciones estáticas. | **Cumple** |

# 5. Resultados

## 5.1 Resultados cuantificables

La integración completa permitió medir la distancia, clasificarla y activar una única salida visual. Los principales resultados obtenidos fueron:

| Aspecto evaluado | Resultado | Referencia de aceptación |
| --- | ---: | ---: |
| Compilación del firmware | <code>SUCCESS</code> | Sin errores |
| Pruebas automatizadas | 20 de 20 aprobadas | 20 de 20 |
| Verificaciones estáticas | 7 de 7 aprobadas | 7 de 7 |
| RAM utilizada | 21 472 bytes (6,6 %) | Dentro de la capacidad del ESP32 |
| Memoria flash utilizada | 270 889 bytes (20,7 %) | Dentro de la capacidad del ESP32 |
| Lecturas físicas registradas | 24 en 8 distancias | Lecturas válidas y crecientes |
| Error absoluto máximo | 1,63 cm | ≤ 3 cm |
| Promedio de errores absolutos | 0,64 cm | Valor informativo |
| Casos del indicador | 5 de 5 aprobados | 5 de 5 |
| Secuencias físicas de histéresis | 5 de 6 aprobadas | 6 de 6 |
| Frecuencia de muestreo | 7,4 lecturas/s | ≥ 2 lecturas/s |
| Estabilidad | 10 min sin incidencias | ≥ 10 min |
| Periodo medio observado | Aproximadamente 135 ms | Compatible con respuesta < 1 s |

## 5.2 Análisis de los resultados

Las mediciones fueron consistentes en todo el conjunto evaluado. El mayor error apareció en 20 cm, con 1,63 cm, y aun así mantuvo un margen de 1,37 cm respecto al máximo permitido. A partir de 40 cm, los errores de los promedios no superaron 0,44 cm. Estos valores muestran que el sensor cumplió el objetivo de exactitud en las condiciones de montaje utilizadas.

La frecuencia de 7,4 lecturas por segundo superó 3,7 veces el mínimo declarado. El periodo medio cercano a 135 ms fue coherente con los 130 ms estimados a partir del timeout y de la espera del ciclo. La prueba de estabilidad confirmó que esta operación se sostuvo durante 10 minutos sin reinicios, bloqueos ni respuestas incorrectas en los puntos de control.

La clasificación controlada y la activación de los LEDs presentaron resultados completos: 20/20 casos automatizados y 5/5 casos de integración fueron aprobados. La única desviación fue H04, donde el indicador osciló entre amarillo y verde al mover el objeto entre 39 y 41 cm. Como las comparaciones exactas del clasificador aprobaron en las pruebas automatizadas, el resultado señala una sensibilidad del sistema completo a la variación del sensor cerca del umbral de 40 cm, no una inconsistencia demostrada en la función de clasificación.

# 6. Conclusiones

1. El prototipo integra correctamente el HC-SR04, el ESP32 y los tres LEDs: cada ciclo obtiene una lectura, determina un rango y actualiza el indicador correspondiente.
2. Los tres requerimientos funcionales fueron verificados. El sensor produjo valores coherentes, la clasificación aprobó todos sus casos automatizados y el indicador respondió correctamente en las cinco condiciones de integración.
3. Los valores no funcionales declarados fueron alcanzados en las pruebas realizadas: 10 minutos de funcionamiento continuo, error máximo de 1,63 cm, 7,4 lecturas por segundo y un periodo medio de actualización cercano a 135 ms.
4. La estructura modular permitió comprobar el clasificador de forma independiente y mantener separadas la adquisición, la decisión y la actuación. La compilación correcta y las siete verificaciones estáticas respaldan la calidad de la implementación.
5. La histéresis funcionó en cinco de seis secuencias físicas, pero la oscilación observada alrededor de 40 cm demuestra que el margen actual no elimina todas las variaciones producidas por el montaje y el sensor. Esta limitación no impide el funcionamiento general, pero debe considerarse al ubicar objetos cerca del umbral.

# 7. Recomendaciones

1. Registrar una serie continua de lecturas entre 38 y 42 cm para determinar si la desviación H04 se debe a dispersión del sensor, movimiento del objeto o condiciones del entorno. Con esos datos podrá evaluarse un filtro de mediana, un promedio de varias lecturas o un ajuste del margen de histéresis.
2. Conservar la salida serie a 115 200 baudios durante las demostraciones y futuras validaciones, ya que permite relacionar directamente la distancia medida, su validez y el rango aplicado.
3. Repetir la prueba de exactitud si se cambia la posición del sensor, la alimentación, el montaje o el entorno, porque estas condiciones pueden alterar el resultado del HC-SR04.
4. Mantener el divisor de tensión en la señal ECHO y las resistencias limitadoras de los LEDs para proteger las entradas y salidas del ESP32.
5. Ejecutar <code>platformio run -e esp32dev</code> y <code>platformio test -e native</code> después de cualquier cambio en el código o en los umbrales, con el fin de confirmar que la integración y los 20 casos de clasificación continúan funcionando.
