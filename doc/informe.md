# 1. Requerimientos Funcionales y No Funcionales

## 1.1 Requerimientos funcionales

El sistema objeto inteligente está definido como un subsistema de medición y visualización de proximidad, construido sobre un ESP32, un sensor ultrasónico HC-SR04 y un conjunto de tres LEDs. El sistema debe detectar la distancia entre el sensor y un objeto, clasificar esa distancia en rangos contiguos y no solapados, y activar un actuador visual distinto para cada rango.

### 1.1.1 RF1. Medición de distancia entre sensor y objeto

El sistema debe medir la distancia entre el sensor ultrasónico HC-SR04 y un objeto presente en el campo de observación. La medida se obtiene por el módulo `SensorUltrasonico`, que dispara el pulso ultrasónico y calcula la distancia en centímetros a partir del tiempo de retorno del eco. La salida del módulo se representa mediante la estructura `LecturaDistancia`, que incluye la distancia calculada y un indicador de validez.

El comportamiento requerido es que, si la lectura es válida, la información se entregue al flujo principal de control para su clasificación. Si la lectura no se recibe o la distancia queda fuera del rango de trabajo declarado, el sistema debe marcar la lectura como inválida y no confiar en el valor para la activación de LEDs.

### 1.1.2 RF2. Interpretación y clasificación de la distancia en rangos contiguos y no solapados

El sistema debe interpretar la distancia leída y clasificarla en al menos tres rangos contiguos y sin solapamiento. La clasificación se implementa en el módulo `ClasificadorDistancia` mediante umbrales configurables y una lógica de histéresis que estabiliza la transición entre rangos.

La lógica de control documentada para el proyecto define el rango de trabajo de 2 cm a 200 cm. Los rangos de distancia y su correspondencia visual son los siguientes:

| Intervalo de distancia | LED activado | Estado visual |
|---|---|---|
| 2 cm ≤ d < 20 cm | Rojo | LED rojo encendido |
| 20 cm ≤ d < 40 cm | Amarillo | LED amarillo encendido |
| 40 cm ≤ d ≤ 200 cm | Verde | LED verde encendido |
| lectura inválida, sin eco o fuera del rango de trabajo | Apagado | los tres LEDs apagados y aviso por el puerto serie |

La representación anterior cumple la condición de disyunción de intervalos: el valor del umbral se asigna al rango superior y cada distancia pertenece a un único intervalo. La clasificación considera también el caso de lectura inválida, sin eco o fuera del rango de trabajo, que no produce activación visual y genera un aviso por el puerto serie.

La lógica de control aplica además un margen de estabilidad de 2 cm alrededor de cada umbral para evitar oscilaciones del LED cuando el objeto queda justo en el límite de transición. Este margen se documenta como decisión de diseño: el cambio de rango solo se produce cuando la distancia supera el umbral por ese margen de estabilidad. Por ejemplo, la transición entre el rango cercano y el rango medio se produce al superar 20 cm en 2 cm, es decir, cuando la distancia pasa de un valor cercano a 18 cm o se mantiene por debajo de 20 cm, y el cambio hacia el siguiente rango solo ocurre si la distancia supera el umbral por el margen definido. La misma regla aplica en el umbral de 40 cm. La política de margen es compatible con el error de medición declarado de ±3 cm con respecto a una cinta métrica, porque la decisión de cambio de rango no se activa en un punto exacto del umbral sino con una zona de tolerancia documentada.

### 1.1.3 RF3. Activación de los actuadores según el rango detectado

El sistema debe activar los actuadores de forma diferenciada según el rango detectado. El módulo `IndicadorLeds` recibe un `EstadoIndicador` y activa un LED distinto para cada nivel de proximidad: rojo para el rango cercano, amarillo para el rango medio, y verde para el rango lejano. El estado de error o lectura inválida se representa mediante apagado de todos los LEDs y un mensaje de aviso por el puerto serie.

La interacción entre módulos se coordina desde `main.cpp`, que realiza el ciclo de lectura, clasificación, conversión del rango a un estado de indicador y envío del comando de visualización. El comportamiento requerido es determinista y consistente con los rangos definidos en RF2: una sola salida de hardware debe quedar encendida para cada lectura válida y los LEDs restantes deben permanecer apagados.

## 1.2 Requerimientos no funcionales

Los requerimientos no funcionales deben expresarse con valores medibles y verificables en las pruebas. Los valores declarados a continuación constituyen objetivos de diseño para la validación del sistema.

### 1.2.1 RNF1. Estabilidad operativa

El sistema debe operar de forma continua durante al menos 10 minutos sin reinicios ni bloqueos. La estabilidad se verificará en pruebas de funcionamiento ininterrumpido, con monitoreo del comportamiento del sistema y ausencia de reseteos del microcontrolador o de la secuencia de lectura y visualización.

### 1.2.2 RNF2. Exactitud de medición

La exactitud de medición del sistema se define como un error máximo de ±3 cm respecto a una cinta métrica, dentro del rango de trabajo declarado de 2 cm a 200 cm. La medición se realizará en condiciones de ensayo controladas, comparando la distancia calculada por `SensorUltrasonico` con la referencia física aplicada mediante una cinta métrica.

### 1.2.3 RNF3. Tiempo de respuesta

El LED debe reflejar el cambio de rango en menos de 1 segundo desde que la distancia observada entra en un nuevo intervalo. El tiempo de respuesta se mide entre la lectura nueva, la actualización del rango por el clasificador y la activación del LED correspondiente por `IndicadorLeds`.

### 1.2.4 RNF4. Frecuencia de muestreo

El sistema debe realizar al menos 2 lecturas de distancia por segundo. Esta frecuencia es la base para mantener una respuesta visual estable y consistente en el indicador de proximidad, sin depender de una sola lectura aislada.

### 1.2.5 RNF5. Calidad del código

El código debe ser legible, modular, orientado a objetos y documentado, además de mantener convenciones de codificación consistentes. La estructura del firmware debe separar el acceso al sensor, la lógica de clasificación y la representación física de los LEDs. La documentación textual y los nombres de archivos, clases, estructuras y funciones deben reflejar el dominio del problema y facilitar el mantenimiento.

## 1.3 Tabla de trazabilidad de requerimientos

| Identificador | Requerimiento | Módulo principal responsable | Prueba de verificación |
|---|---|---|---|
| RF1 | Medición de distancia entre sensor y objeto | `SensorUltrasonico` y `main.cpp` | `PR-SENSOR-001` |
| RF2 | Interpretación y clasificación en rangos contiguos y sin solapamiento | `ClasificadorDistancia` y `main.cpp` | `PR-CLASIFICACION-001` |
| RF3 | Activación de actuadores según rango detectado | `IndicadorLeds` y `main.cpp` | `PR-INDICADOR-001` |
| RNF1 | Estabilidad operativa durante al menos 10 minutos | `main.cpp`, `SensorUltrasonico`, `ClasificadorDistancia`, `IndicadorLeds` | `PR-ESTABILIDAD-001` |
| RNF2 | Exactitud de medición de ±3 cm en rango de 2 a 200 cm | `SensorUltrasonico` y `main.cpp` | `PR-EXACTITUD-001` |
| RNF3 | Tiempo de respuesta de cambio de LED menor a 1 segundo | `ClasificadorDistancia`, `IndicadorLeds` y `main.cpp` | `PR-RESPUESTA-001` |
| RNF4 | Frecuencia de muestreo mínima de 2 lecturas por segundo | `SensorUltrasonico` y `main.cpp` | `PR-MUESTREO-001` |
| RNF5 | Calidad del código: legible, modular, OO, documentado y consistente | `SensorUltrasonico`, `ClasificadorDistancia`, `IndicadorLeds`, `main.cpp` | `PR-CALIDAD-CODIGO-001` |
