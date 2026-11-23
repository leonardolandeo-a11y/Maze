# Informe de rediseño de interfaz MAZE

## Resumen

Interfaz FTXUI más compacta y coherente: inicio directo, menú cian con el logo original, resultados pixelados y ampliación proporcional de sprites. Se conservaron las mecánicas, los mapas, los fotogramas y la arquitectura de concurrencia. Sin dependencias nuevas ni cambios de CMake.

## Archivos modificados

| Archivo | Cambio |
| --- | --- |
| `include/circuit_escape/animation_sequence.h` | Fin del inicio al completar la cinemática; bienvenida desconectada. |
| `src/GameApplication.cpp` | Transición automática al menú del ejecutable clásico. |
| `src/project2_application.cpp` | Transición automática en Circuito de Escape. |
| `src/CinematicStartupAnimation.cpp` | Reutiliza el mismo logo detallado que el menú. |
| `src/menus.cpp` | Opciones compactas, selección cian y panel contextual. |
| `src/ui_theme.cpp` | Logo original, composición del menú y resultados compartidos. |
| `src/VictoryAnimation.cpp` | Título y mensaje de escape completado. |
| `src/cinematic_view.cpp` | Ampliación compartida de sprites con colores por medio bloque. |
| `tests/cinematic_sequence_test.cpp` | Expectativas del inicio/resultados, estadísticas y escalado. |
| `tests/responsive_ui_test.cpp` | Resultados nuevos y visibilidad del menú clásico. |
| `tests/delivery/terminal_playthrough.py` | Elimina la espera de WELCOME y reconoce el nuevo resultado. |
| `docs/UI_REDESIGN_REPORT.md` | Este informe. |

## Cambios por funcionalidad

- **Inicio:** conserva las 166 tramas originales; termina sin pedir Enter. Q sigue saliendo. `StartupAnimation` permanece disponible, fuera del flujo de ambas aplicaciones.
- **Menú:** logo de seis líneas con sus contornos originales, subtítulo y lema. Cinco opciones en Circuito de Escape; la versión clásica conserva SCENARIO. Panel contextual desde 100×30; resumen compacto en 80×24. Se eliminó el circuito decorativo del menú.
- **Derrota:** GAME OVER rojo pixelado, motivo real y estadísticas centradas: turnos, energía actual/máxima, puntaje y recursos. Sin marca MAZE.
- **Victoria:** ESCAPE COMPLETED verde pixelado, trofeo sencillo, mensaje «¡Has escapado del circuito!» y el mismo panel de estadísticas. Sin marca MAZE.
- **Escalado:** ajuste común en `CinematicView::Fit`; amplía los sprites según ancho y alto disponibles, conservando centrado, proporciones y colores superior/inferior. Sin editar las cinemáticas de derrota/victoria. Se conservan los tiempos y controles finales existentes.

## Pruebas ejecutadas

| Comando o comprobación | Resultado real |
| --- | --- |
| `cmake --build build -j 2` | Compilan Maze, escape_circuit, map_generator y Maze_test. |
| `git diff --check` antes de cada commit | Sin errores. |
| `MAZE_UI_PREVIEW_DIR=/tmp/maze-ui-preview /tmp/maze-ui-check` | Pasan los cuatro grupos existentes: cinematic_sequence, responsive_ui, menus y animations. |
| `python3 /tmp/maze-terminal-check.py` | PTY real: 80×24/easy, 100×30/medium y 120×35/hard llegan a victoria; 160×45 llega a derrota por captura. Inicio sin Enter, ayuda, Enter al menú y Q funcionan. Q durante el inicio funciona en ambos ejecutables. Mapas preparados sin cambios. |
| `ctest --test-dir build --output-on-failure` | Falla Maze_test con `Map grid and graph do not match`. |
| `/tmp/maze-all-check` | Ejecuta los 30 grupos existentes: 28 pasan; fallan almacenamiento e integración de mapas con la misma excepción. |
| `ctest --test-dir /tmp/maze-ui-baseline-build --output-on-failure` y `/tmp/maze-baseline-all-check` | Una copia limpia de `07ffbf4` reproduce el fallo de CTest y exactamente los mismos dos grupos fallidos. |

Los lanzadores en `/tmp` enlazan las pruebas C++ existentes o usan únicamente la biblioteca estándar de Python para enviar entradas a una PTY; no agregan suites ni dependencias al proyecto. Las capturas `.txt`/`.ansi` de FTXUI y sesiones PTY quedan en `/tmp/maze-ui-preview`.

Las comprobaciones cubren los cuatro tamaños solicitados, límites de fases de las cinemáticas, estadísticas con valores conocidos, tres motivos de derrota, navegación y conservación de la paleta al ampliar. La suite base se compiló desde `git archive 07ffbf4`, reutilizando la copia local de FTXUI mediante `FETCHCONTENT_SOURCE_DIR_FTXUI`.

## Commits realizados

| Referencia | Título | Resumen |
| --- | --- | --- |
| `022b4dc` | `feat(ui): unificar inicio y rediseñar menú principal` | Inicio automático y menú con logo original. |
| `6bb59f9` | `feat(ui): unificar pantallas de derrota y escape completado` | Resultados pixelados y estadísticas reales. |
| `276524a` | `fix(ui): ampliar cinemáticas sin deformar los sprites` | Escalado común y validación responsive. |
| Commit de este informe | `docs(ui): documentar validación del rediseño visual` | Cierre de validación y documentación. |

El hash del cuarto commit se obtiene con `git log -1 --format='%h %s' -- docs/UI_REDESIGN_REPORT.md`; se evita una autorreferencia que cambiaría el propio hash. Trabajo en `feat/ui-menu-cinematics-results`, sin push.

## Limitaciones y pendientes

- Los dos fallos de mapas ya existen en `07ffbf4`; quedan fuera del alcance de interfaz y no se modificaron algoritmos ni expectativas de esas pruebas.
- Las escenas con texto y el logo mantienen sus glifos a tamaño nativo para evitar deformarlos; la ampliación se aplica a los fotogramas de sprites por bloques.
- Se preservaron los cambios previos en README, el instalador Linux y `docs/INFORME_TECNICO_EXPOSICION_MAZE.md`, sin incluirlos en los commits.
