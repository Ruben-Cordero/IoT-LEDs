#include "ClasificadorDistancia.h"
#include "Config.h"

namespace {

RangoDistancia clasificarPorUmbrales(float distanciaCm) {
    if (distanciaCm < UMBRAL_CERCA_MEDIO_CM) {
        return RangoDistancia::CERCANO;
    }
    if (distanciaCm < UMBRAL_MEDIO_LEJOS_CM) {
        return RangoDistancia::MEDIO;
    }
    return RangoDistancia::LEJANO;
}

}

RangoDistancia clasificarDistancia(const LecturaDistancia& lectura, RangoDistancia rangoAnterior) {

    if (!lectura.valida) {
        return RangoDistancia::ERROR;
    }

    const float distancia = lectura.distanciaCm;

    
    if (distancia < DISTANCIA_MINIMA_VALIDA_CM || distancia > DISTANCIA_MAXIMA_VALIDA_CM) {
        return RangoDistancia::ERROR;
    }

    if (rangoAnterior == RangoDistancia::ERROR) {
        return clasificarPorUmbrales(distancia);
    }

    switch (rangoAnterior) {
        case RangoDistancia::CERCANO:
            if (distancia >= UMBRAL_CERCA_MEDIO_CM + MARGEN_HISTERESIS_CM) {
                return clasificarPorUmbrales(distancia);
            }
            return RangoDistancia::CERCANO;

        case RangoDistancia::MEDIO:
            if (distancia < UMBRAL_CERCA_MEDIO_CM - MARGEN_HISTERESIS_CM) {
                return RangoDistancia::CERCANO;
            }
            if (distancia >= UMBRAL_MEDIO_LEJOS_CM + MARGEN_HISTERESIS_CM) {
                return RangoDistancia::LEJANO;
            }
            return RangoDistancia::MEDIO;

        case RangoDistancia::LEJANO:
            if (distancia < UMBRAL_MEDIO_LEJOS_CM - MARGEN_HISTERESIS_CM) {
                return clasificarPorUmbrales(distancia);
            }
            return RangoDistancia::LEJANO;

        default:
            return clasificarPorUmbrales(distancia);
    }
}
