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

void setup() {
    sensor.begin();
    indicador.begin();
}

void loop() {
    LecturaDistancia lectura = sensor.medirDistanciaCm();

    rangoAnterior = clasificarDistancia(lectura, rangoAnterior);

    EstadoIndicador estado = convertirAEstado(rangoAnterior);
    indicador.mostrar(estado);

    delay(100);
}