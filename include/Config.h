#ifndef CONFIG_H
#define CONFIG_H

// Umbral entre el rango "cercano" y el rango "medio", en centimetros.
// El propio valor del umbral pertenece al rango medio (rango superior).
constexpr float UMBRAL_CERCA_MEDIO_CM = 20.0f;

// Umbral entre el rango "medio" y el rango "lejano", en centimetros.
// El propio valor del umbral pertenece al rango lejano (rango superior).
constexpr float UMBRAL_MEDIO_LEJOS_CM = 40.0f;

// Margen de histeresis, en centimetros: cuanto debe superarse un umbral
// para que el rango cambie. Evita que el rango oscile cuando la
// distancia queda justo en el limite entre dos rangos.
constexpr float MARGEN_HISTERESIS_CM = 2.0f;

// Limites del rango de trabajo del sensor, en centimetros. Fuera de este
// rango la lectura se considera un error aunque el sensor la marque como valida.
constexpr float DISTANCIA_MINIMA_VALIDA_CM = 2.0f;
constexpr float DISTANCIA_MAXIMA_VALIDA_CM = 200.0f;

#endif
