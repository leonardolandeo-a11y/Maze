# Reparación local de cinemáticas — #138

Trabajo en `138-restaurar-cinemáticas-originales-sin-reemplazar-las-pantallas-responsive`.
Los cambios quedan sin stage, commit ni publicación. No se modifica
`cmake-build-debug/`, audio, reglas, menús, mapas ni algoritmos.

## Recuperación y fases

Las escenas proceden de `20316e5^`, copiadas a componentes separados:
`CinematicStartupAnimation`, `CinematicVictoryAnimation` y
`CinematicDeathAnimation`. Se conservan sprites, colores, posiciones, partículas
y cálculos originales. Los límites se obtuvieron de los dispatchers originales:

| Secuencia | Frames originales conservados | Fases |
|---|---|---|
| Inicio | 0–165 | Boot 0–37; pausa 38–41; glitch 42–63; formación 64–93; título 94–109; subtítulo desde 110; carga visual desde 124; listo 156–165 |
| Victoria | 0–77 | Caminar 0–23; girar 24–33; levantar brazo 34–45; baile y partículas 46–77 |
| Derrota | 0–141 | Caída 0–15; suelo 16–27; levantarse 28–41; girar 42–49; bomba 50–83 (sacar, elevar, inspeccionar); explosión 84–93; fragmentos 94–125; reposo 126–141 |

Los frames 78–109 de la antigua victoria y 142–159 de la antigua derrota
presentaban sus pantallas finales estáticas. Se omiten esas pantallas duplicadas,
no las escenas animadas del personaje. `victoryTitleFrame = 90` era una constante
sin uso en el dispatcher original, no el final de la celebración.

## Flujo final

Ambos controladores, `GameApplication` (Maze) y `Project2Application`
(escape_circuit), usan `animation_sequence.h` para encadenar los componentes:

- Inicio: 166 frames originales (~9.96 s al ritmo existente de 60 ms) →
  WELCOME responsive. WELCOME no desaparece automáticamente: Enter continúa.
  Enter durante la cinemática se ignora; Q permite salir.
- Victoria: 78 frames del personaje (~4.68 s) → VICTORY responsive con los datos
  de la partida. El panel conserva sus 36 frames y habilita el retorno al menú
  al finalizar, como antes.
- Derrota: 142 frames del personaje y efectos (~8.52 s) → GAME OVER responsive,
  con causa, energía, turnos, recursos, puntuación y controles conservados.

No se añaden threads ni bucles de eventos. Los contadores existentes avanzan
mediante callbacks publicados en el hilo de FTXUI. La espera se limita al timer
ya existente; el hilo de interfaz no duerme. Las clases responsive originales
siguen separadas; solo se añade ENTER THE CIRCUIT al texto final de WELCOME
para mantener visible esa opción cuando termina su animación.

La carga visual del antiguo inicio es decoración; no llama al generador ni
escribe mapas. El comportamiento offline y los argumentos de escape_circuit
se conservan, incluida la carga directa con `--difficulty`.

## Terminales pequeñas

`CinematicView::Stage` conserva el lienzo original de 90×20 celdas.
`CinematicView::Fit` muestra ese lienzo sin cambios cuando cabe; cuando no cabe,
lo reduce proporcionalmente mediante un framebuffer FTXUI. No recorta
laterales ni acelera/omite fases. A 80×24 conserva el escenario completo con
menos detalle; los paneles siguientes mantienen sus reglas responsive.
No se agregan condicionales de plataforma ni dependencias nuevas.

## Verificación local

- `cmake -S . -B build` y `cmake --build build --parallel`: aprobados.
- `./build/Maze_test`: aprobado, incluyendo la suite existente.
- `ctest --test-dir build --output-on-failure`: 1/1 aprobado, 9.96 s.
- Nuevas regresiones en `tests/cinematic_sequence_test.cpp`: ambos lados de
  cada límite de fase, ausencia de resultados/controles prematuros, conservación
  de estadísticas y causas, y encaje en 80×24, 100×30, 120×35 y 160×45.
- Comparación con Git local: los cuerpos originales de todas las escenas
  se conservan, salvo nombres de clase y del helper de presentación.
- Generador offline 20×30: tres archivos creados; carga directa de Medium
  a 80×24, salida 0, SHA-256/mtime sin cambios. Archivos faltantes devuelven
  error claro y código 1.
- PTY Linux de ambos ejecutables: intro completa, Enter anticipado ignorado,
  WELCOME persistente, Enter al menú, partida, victoria y derrota con retorno
  al menú y salida 0. Se redimensionó durante las cinemáticas entre 80×24,
  100×30, 120×35 y 160×45. La victoria de escape_circuit se condujo con el witness
  guardado; la de Maze con una ruta del escenario 1 validada por su motor.
  No aparecieron las antiguas pantallas FINAL STATS. Los hashes y mtimes de
  los mapas permanecieron idénticos. Esta comprobación fue automatizada.
- Los frames renderizados pueden exportarse con:
  `MAZE_UI_PREVIEW_DIR=build/cinematic-previews ./build/Maze_test`.

Se inspeccionaron imágenes de frames reales del renderer: glitch, formación/
título, celebración, bomba, explosión, fragmentos, WELCOME y tamaño pequeño.
Esto no equivale a una revisión humana de la reproducción completa en terminal.

## Revisión manual antes de aceptar

1. Ejecutar `./build/escape_circuit`: observar toda la intro, incluyendo glitch
   y formación del logo; comprobar que termina en WELCOME y espera Enter.
2. Ganar: verificar caminar, giro, brazo, baile y partículas antes del panel
   VICTORY nuevo; revisar sus estadísticas y retorno al menú.
3. Perder: verificar caída, levantarse, bomba, explosión, caída y reposo de
   fragmentos antes del panel GAME OVER; revisar causa y estadísticas.
4. Repetir en `./build/Maze`, incluyendo volver al menú sin repetir la intro.
5. Redimensionar durante escenas y paneles. Verificar fluidez, legibilidad y
   colores en Windows Terminal y Linux con las fuentes utilizadas por el equipo.
6. Confirmar generación offline, carga por dificultad y ausencia de cambios
   en los archivos de mapas después de jugar.

No se ejecutó una compilación ni una terminal Windows. La integración futura
de audio de introducción no forma parte de esta reparación.
