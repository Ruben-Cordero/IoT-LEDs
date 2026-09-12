#include "ClasificadorDistancia.h"
#include "Config.h"

namespace {

// Clasificacion "pura", solo por umbrales, sin histeresis.
// El valor del umbral pertenece siempre al rango superior.
RangoDistancia clasificarPorUmbrales(float distanciaCm) {
    if (distanciaCm < UMBRAL_CERCA_MEDIO_CM) {
        return RangoDistancia::CERCANO;
    }
    if (distanciaCm < UMBRAL_MEDIO_LEJOS_CM) {
        return RangoDistancia::MEDIO;
    }
    return RangoDistancia::LEJANO;
}

} // namespace

RangoDistancia clasificarDistancia(const LecturaSensor& lectura, RangoDistancia rangoAnterior) {
    // Lectura marcada como invalida por el sensor.
    if (!lectura.esValida) {
        return RangoDistancia::ERROR;
    }

    const float distancia = lectura.distanciaCm;

    // Fuera del rango de trabajo del sensor.
    if (distancia < DISTANCIA_MINIMA_VALIDA_CM || distancia > DISTANCIA_MAXIMA_VALIDA_CM) {
        return RangoDistancia::ERROR;
    }

    // Sin clasificacion previa valida: no hay nada que estabilizar.
    if (rangoAnterior == RangoDistancia::ERROR) {
        return clasificarPorUmbrales(distancia);
    }

    // Con clasificacion previa: solo se cambia de rango cuando la
    // distancia supera el umbral correspondiente por el margen configurado.
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
