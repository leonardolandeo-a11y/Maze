#pragma once

#include <cstddef>

/*
 * Define los tres niveles de dificultad del juego.
 * Cada nivel cambia algunos valores de las reglas,
 * como la energía inicial o el costo del terreno difícil.
 */
enum class Difficulty {
    easy,       /// Nivel Fácil: mayor energía, más turnos y menores penalizaciones.
    standard,   /// Nivel Estándar: balance predeterminado para demostración.
    hard        /// Nivel Difícil: recursos limitados, mayor costo de terreno y castigos altos.
};

/*
 * Aquí guardamos todos los valores que determinan
 * cómo funcionan las reglas de la partida.
 *
 * La idea es que el resto del programa no tenga que
 * escribir números como "3 de energía" directamente,
 * sino que pueda obtenerlos desde GameRules.
 */
struct GameRules {

    // Valores con los que empieza el agente y límite de turnos.
    int initialEnergy{60};       /// Energía con la que inicia el agente.
    int maxEnergy{60};           /// Límite máximo de energía acumulable.
    std::size_t turnLimit{180};  /// Límite máximo de turnos permitidos.

    // Cuánta energía se gasta dependiendo del movimiento.
    int normalCellCost{1};       /// Costo de entrar a una celda normal.
    int roughTerrainCost{2};     /// Costo de entrar a terreno elevado.
    int invalidMovementCost{1};  /// Costo de un movimiento inválido.
    int waitCost{1};             /// Costo de la acción 'wait'.

    // Valores relacionados con recursos, baterías y trampas.
    int resourcePoints{10};      /// Puntos obtenidos al recoger un recurso.
    int batteryRecharge{3};      /// Energía recuperada por una batería.
    int trapEnergyPenalty{2};    /// Energía perdida al pisar una trampa.
    int trapScorePenalty{1};     /// Puntos perdidos al pisar una trampa.

    /*
     * Crea las reglas según la dificultad elegida.
     *
     * Por ejemplo, en Hard el terreno elevado cuesta
     * 3 de energía en lugar de 2.
     */
    [[nodiscard]]
    static GameRules rulesFor(Difficulty diff) {

        switch (diff) {

            case Difficulty::easy:
                return {
                    80,  // Energía inicial
                    80,  // Energía máxima
                    240, // Límite de turnos

                    1,   // Celda normal
                    2,   // Terreno elevado
                    1,   // Movimiento inválido
                    1,   // Esperar

                    15,  // Puntos por recurso
                    5,   // Energía de batería
                    1,   // Penalización de energía de trampa
                    0    // Penalización de puntos de trampa
                };

            case Difficulty::hard:
                return {
                    40,  // Energía inicial
                    40,  // Energía máxima
                    140, // Límite de turnos

                    1,   // Celda normal
                    3,   // Terreno elevado
                    1,   // Movimiento inválido
                    1,   // Esperar

                    8,   // Puntos por recurso
                    2,   // Energía de batería
                    3,   // Penalización de energía de trampa
                    2    // Penalización de puntos de trampa
                };

            case Difficulty::standard:
            default:
                return {
                    60,  // Energía inicial
                    60,  // Energía máxima
                    180, // Límite de turnos

                    1,   // Celda normal
                    2,   // Terreno elevado
                    1,   // Movimiento inválido
                    1,   // Esperar

                    10,  // Puntos por recurso
                    3,   // Energía de batería
                    2,   // Penalización de energía de trampa
                    1    // Penalización de puntos de trampa
                };
        }
    }
};
