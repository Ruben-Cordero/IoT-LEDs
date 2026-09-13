#ifndef CLASIFICADOR_DISTANCIA_H
#define CLASIFICADOR_DISTANCIA_H
#include "LecturaDistancia.h"


enum class RangoDistancia {
    CERCANO,
    MEDIO,
    LEJANO,
    ERROR
};

RangoDistancia clasificarDistancia(const LecturaDistancia& lectura, RangoDistancia rangoAnterior);

#endif
