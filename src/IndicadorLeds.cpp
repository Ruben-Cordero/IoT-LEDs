#include "IndicadorLeds.h"

#include <Arduino.h>

void IndicadorLeds::begin() {
    pinMode(PIN_ROJO, OUTPUT);
    pinMode(PIN_AMARILLO, OUTPUT);
    pinMode(PIN_VERDE, OUTPUT);
    apagarTodos();
}

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

void IndicadorLeds::apagarTodos() {
    digitalWrite(PIN_ROJO, LOW);
    digitalWrite(PIN_AMARILLO, LOW);
    digitalWrite(PIN_VERDE, LOW);
}
