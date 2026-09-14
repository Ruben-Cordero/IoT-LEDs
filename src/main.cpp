#include <Arduino.h>

#include "SensorUltrasonico.h"
#include "ClasificadorDistancia.h"
#include "IndicadorLeds.h"

SensorUltrasonico sensor;
IndicadorLeds indicador;

RangoDistancia rangoAnterior = RangoDistancia::ERROR;

EstadoIndicador convertirAEstado(RangoDistancia rango) {
    switch (rango) {
        case RangoDistancia::CERCANO:
            return EstadoIndicador::Rojo;

        case RangoDistancia::MEDIO:
            return EstadoIndicador::Amarillo;

        case RangoDistancia::LEJANO:
            return EstadoIndicador::Verde;

        case RangoDistancia::ERROR:
        default:
            return EstadoIndicador::Error;
    }
}

const char* rangoATexto(RangoDistancia rango) {
    switch (rango) {
        case RangoDistancia::CERCANO:
            return "CERCANO";
        case RangoDistancia::MEDIO:
            return "MEDIO";
        case RangoDistancia::LEJANO:
            return "LEJANO";
        case RangoDistancia::ERROR:
        default:
            return "ERROR";
    }
}

void setup() {
    Serial.begin(115200);
    sensor.begin();
    indicador.begin();
}

void loop() {
    LecturaDistancia lectura = sensor.medirDistanciaCm();

    rangoAnterior = clasificarDistancia(lectura, rangoAnterior);

    EstadoIndicador estado = convertirAEstado(rangoAnterior);
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