#include "SensorUltrasonico.h"


SensorUltrasonico::SensorUltrasonico(uint8_t pinTrig, uint8_t pinEcho, unsigned long timeoutUs)
    : _pinTrig(pinTrig), _pinEcho(pinEcho), _timeoutUs(timeoutUs) {
}

void SensorUltrasonico::begin() {
    pinMode(_pinTrig, OUTPUT);
    pinMode(_pinEcho, INPUT);
    digitalWrite(_pinTrig, LOW);
}

LecturaDistancia SensorUltrasonico::medirDistanciaCm() {
    LecturaDistancia lectura;
    lectura.distanciaCm = 0.0f;
    lectura.valida = false;

    digitalWrite(_pinTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(_pinTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_pinTrig, LOW);

    unsigned long duracionUs = pulseIn(_pinEcho, HIGH, _timeoutUs);

    if (duracionUs == 0) {
        return lectura;
    }

    float distancia = (duracionUs * VELOCIDAD_SONIDO_CM_US) / 2.0f;

    if (distancia < DISTANCIA_MIN_CM || distancia > DISTANCIA_MAX_CM) {
        return lectura; 
    }

    lectura.distanciaCm = distancia;
    lectura.valida = true;
    return lectura;
}
