#ifndef SENSOR_ULTRASONICO_H
#define SENSOR_ULTRASONICO_H

#include <Arduino.h>

// Resultado de una medicion: la distancia y si el dato es confiable.
struct LecturaDistancia {
    float distanciaCm;
    bool valida;
};

class SensorUltrasonico {
public:
    // Por defecto usa TRIG en GPIO25 y ECHO en GPIO26.
    // timeoutUs limita cuanto se espera por el eco antes de rendirse.
    SensorUltrasonico(uint8_t pinTrig = 25, uint8_t pinEcho = 26, unsigned long timeoutUs = 30000);

    // Configura los pines. Debe llamarse una vez dentro de setup().
    void begin();

    // Dispara el sensor y devuelve la distancia medida en centimetros.
    LecturaDistancia medirDistanciaCm();

private:
    uint8_t _pinTrig;
    uint8_t _pinEcho;
    unsigned long _timeoutUs;

    // Velocidad del sonido en cm/us (aprox. 343 m/s), usada para convertir
    // el tiempo del eco en distancia.
    static constexpr float VELOCIDAD_SONIDO_CM_US = 0.0343f;

    // Rango de operacion confiable del HC-SR04, segun su datasheet.
    static constexpr float DISTANCIA_MIN_CM = 2.0f;
    static constexpr float DISTANCIA_MAX_CM = 400.0f;
};

#endif
