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

    // Pulso de disparo (trigger) de 10us, como pide el datasheet del HC-SR04.
    digitalWrite(_pinTrig, LOW);
    delayMicroseconds(2);
    digitalWrite(_pinTrig, HIGH);
    delayMicroseconds(10);
    digitalWrite(_pinTrig, LOW);

    // pulseIn espera a que ECHO suba y baje, y devuelve cuanto duro en HIGH.
    // Si no hay eco dentro de _timeoutUs, devuelve 0 en vez de bloquear para siempre.
    unsigned long duracionUs = pulseIn(_pinEcho, HIGH, _timeoutUs);

    if (duracionUs == 0) {
        return lectura; // timeout: no llego eco, lectura invalida
    }

    // distancia = (tiempo total del viaje del sonido) * velocidad / 2 (ida y vuelta)
    float distancia = (duracionUs * VELOCIDAD_SONIDO_CM_US) / 2.0f;

    if (distancia < DISTANCIA_MIN_CM || distancia > DISTANCIA_MAX_CM) {
        return lectura; // fuera del rango confiable del sensor
    }

    lectura.distanciaCm = distancia;
    lectura.valida = true;
    return lectura;
}
