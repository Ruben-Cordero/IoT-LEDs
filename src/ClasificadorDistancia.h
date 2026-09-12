#ifndef CLASIFICADOR_DISTANCIA_H
#define CLASIFICADOR_DISTANCIA_H

// Dato de entrada: lo que entrega el modulo del sensor.
// Solo dos campos, tal como los produce la medicion.
struct LecturaSensor {
    float distanciaCm;
    bool esValida;
};

// Resultado de la clasificacion. Los nombres describen distancia,
// no color: la asignacion de color a cada rango es responsabilidad
// del modulo que controla los LEDs.
enum class RangoDistancia {
    CERCANO,
    MEDIO,
    LEJANO,
    ERROR
};

// Clasifica una lectura del sensor en uno de los rangos de distancia.
//
// rangoAnterior es el resultado de la clasificacion previa y se usa
// unicamente para aplicar histeresis (evitar cambios inestables cuando
// la distancia esta justo en un umbral). Si no hay clasificacion previa,
// se puede pasar RangoDistancia::ERROR.
RangoDistancia clasificarDistancia(const LecturaSensor& lectura, RangoDistancia rangoAnterior);

#endif
