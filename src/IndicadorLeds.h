#ifndef INDICADOR_LEDS_H
#define INDICADOR_LEDS_H

enum class EstadoIndicador {
    Rojo,
    Amarillo,
    Verde,
    Error
};

class IndicadorLeds {
public:
    void begin();

    void mostrar(EstadoIndicador estado);

private:
    static constexpr int PIN_ROJO = 27;
    static constexpr int PIN_AMARILLO = 32;
    static constexpr int PIN_VERDE = 33;

    void apagarTodos();
};

#endif
