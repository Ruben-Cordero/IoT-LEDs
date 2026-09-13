#ifndef SENSOR_ULTRASONICO_H
#define SENSOR_ULTRASONICO_H

#include <Arduino.h>
#include "LecturaDistancia.h"


class SensorUltrasonico {
public:

    SensorUltrasonico(uint8_t pinTrig = 25, uint8_t pinEcho = 26, unsigned long timeoutUs = 30000);

    void begin();

    LecturaDistancia medirDistanciaCm();

private:
    uint8_t _pinTrig;
    uint8_t _pinEcho;
    unsigned long _timeoutUs;

    static constexpr float VELOCIDAD_SONIDO_CM_US = 0.0343f;

    static constexpr float DISTANCIA_MIN_CM = 2.0f;
    static constexpr float DISTANCIA_MAX_CM = 400.0f;
};

#endif
