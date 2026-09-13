#include <unity.h>

#include "ClasificadorDistancia.h"
#include "Config.h"

static_assert(DISTANCIA_MINIMA_VALIDA_CM < UMBRAL_CERCA_MEDIO_CM,
              "La distancia minima debe ser menor que el primer umbral");
static_assert(UMBRAL_CERCA_MEDIO_CM < UMBRAL_MEDIO_LEJOS_CM,
              "Los umbrales deben estar ordenados");
static_assert(UMBRAL_MEDIO_LEJOS_CM < DISTANCIA_MAXIMA_VALIDA_CM,
              "El segundo umbral debe ser menor que la distancia maxima");
static_assert(MARGEN_HISTERESIS_CM >= 0.0f,
              "El margen de histeresis no puede ser negativo");

void setUp() {}
void tearDown() {}

void test_lectura_invalida_devuelve_error() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::ERROR),
        static_cast<int>(clasificarDistancia({15.0f, false}, RangoDistancia::CERCANO)));
}

void test_distancia_menor_al_minimo_devuelve_error() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::ERROR),
        static_cast<int>(clasificarDistancia({1.99f, true}, RangoDistancia::ERROR)));
}

void test_limite_minimo_es_cercano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::CERCANO),
        static_cast<int>(clasificarDistancia({2.0f, true}, RangoDistancia::ERROR)));
}

void test_valor_inferior_a_20_es_cercano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::CERCANO),
        static_cast<int>(clasificarDistancia({19.99f, true}, RangoDistancia::ERROR)));
}

void test_20_exacto_es_medio() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({20.0f, true}, RangoDistancia::ERROR)));
}

void test_valor_inferior_a_40_es_medio() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({39.99f, true}, RangoDistancia::ERROR)));
}

void test_40_exacto_es_lejano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::LEJANO),
        static_cast<int>(clasificarDistancia({40.0f, true}, RangoDistancia::ERROR)));
}

void test_limite_maximo_es_lejano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::LEJANO),
        static_cast<int>(clasificarDistancia({200.0f, true}, RangoDistancia::ERROR)));
}

void test_distancia_mayor_al_maximo_devuelve_error() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::ERROR),
        static_cast<int>(clasificarDistancia({200.01f, true}, RangoDistancia::ERROR)));
}

void test_cercano_se_mantiene_antes_de_22() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::CERCANO),
        static_cast<int>(clasificarDistancia({21.99f, true}, RangoDistancia::CERCANO)));
}

void test_cercano_cambia_a_medio_en_22() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({22.0f, true}, RangoDistancia::CERCANO)));
}

void test_cercano_puede_saltar_directamente_a_lejano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::LEJANO),
        static_cast<int>(clasificarDistancia({100.0f, true}, RangoDistancia::CERCANO)));
}

void test_medio_se_mantiene_en_18() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({18.0f, true}, RangoDistancia::MEDIO)));
}

void test_medio_cambia_a_cercano_debajo_de_18() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::CERCANO),
        static_cast<int>(clasificarDistancia({17.99f, true}, RangoDistancia::MEDIO)));
}

void test_medio_se_mantiene_antes_de_42() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({41.99f, true}, RangoDistancia::MEDIO)));
}

void test_medio_cambia_a_lejano_en_42() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::LEJANO),
        static_cast<int>(clasificarDistancia({42.0f, true}, RangoDistancia::MEDIO)));
}

void test_lejano_se_mantiene_en_38() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::LEJANO),
        static_cast<int>(clasificarDistancia({38.0f, true}, RangoDistancia::LEJANO)));
}

void test_lejano_cambia_a_medio_debajo_de_38() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({37.99f, true}, RangoDistancia::LEJANO)));
}

void test_lejano_puede_saltar_directamente_a_cercano() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::CERCANO),
        static_cast<int>(clasificarDistancia({10.0f, true}, RangoDistancia::LEJANO)));
}

void test_recuperacion_desde_error_clasifica_sin_histeresis() {
    TEST_ASSERT_EQUAL_INT(
        static_cast<int>(RangoDistancia::MEDIO),
        static_cast<int>(clasificarDistancia({25.0f, true}, RangoDistancia::ERROR)));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();

    RUN_TEST(test_lectura_invalida_devuelve_error);
    RUN_TEST(test_distancia_menor_al_minimo_devuelve_error);
    RUN_TEST(test_limite_minimo_es_cercano);
    RUN_TEST(test_valor_inferior_a_20_es_cercano);
    RUN_TEST(test_20_exacto_es_medio);
    RUN_TEST(test_valor_inferior_a_40_es_medio);
    RUN_TEST(test_40_exacto_es_lejano);
    RUN_TEST(test_limite_maximo_es_lejano);
    RUN_TEST(test_distancia_mayor_al_maximo_devuelve_error);
    RUN_TEST(test_cercano_se_mantiene_antes_de_22);
    RUN_TEST(test_cercano_cambia_a_medio_en_22);
    RUN_TEST(test_cercano_puede_saltar_directamente_a_lejano);
    RUN_TEST(test_medio_se_mantiene_en_18);
    RUN_TEST(test_medio_cambia_a_cercano_debajo_de_18);
    RUN_TEST(test_medio_se_mantiene_antes_de_42);
    RUN_TEST(test_medio_cambia_a_lejano_en_42);
    RUN_TEST(test_lejano_se_mantiene_en_38);
    RUN_TEST(test_lejano_cambia_a_medio_debajo_de_38);
    RUN_TEST(test_lejano_puede_saltar_directamente_a_cercano);
    RUN_TEST(test_recuperacion_desde_error_clasifica_sin_histeresis);

    return UNITY_END();
}
