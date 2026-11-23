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
- **Menú:** logo de seis líneas con sus contornos originales y lema, sin subtítulo. Cinco opciones en Circuito de Escape; la versión clásica conserva SCENARIO. Tarjetas con bordes individuales y panel contextual desde 100×30; logo lateral y tarjetas a la derecha en 80×24. Se eliminó el circuito decorativo del menú.
- **Derrota:** GAME OVER rojo pixelado, motivo real y estadísticas centradas: turnos, energía actual/máxima, puntaje y recursos. Sin marca MAZE.
- **Victoria:** ESCAPE encima de COMPLETED en bloques Unicode verdes, sin trofeo, mensaje «¡Has escapado del circuito!» y el mismo panel de estadísticas. Sin marca MAZE.
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
| `21dc4e0` | `docs(ui): documentar validación del rediseño visual` | Cierre de validación y documentación. |

Trabajo en `feat/ui-menu-cinematics-results`, sin push.

## Limitaciones y pendientes

- Los dos fallos de mapas ya existen en `07ffbf4`; quedan fuera del alcance de interfaz y no se modificaron algoritmos ni expectativas de esas pruebas.
- Las escenas con texto y el logo mantienen sus glifos a tamaño nativo para evitar deformarlos; la ampliación se aplica a los fotogramas de sprites por bloques.
- Se preservaron los cambios previos en README, el instalador Linux y `docs/INFORME_TECNICO_EXPOSICION_MAZE.md`, sin incluirlos en los commits.

## Corrección visual posterior

- Cambios localizados en `src/menus.cpp` y `src/ui_theme.cpp`: tarjetas individuales con separación adaptativa, títulos Unicode exactos facilitados por el usuario y estadísticas más anchas con valores alineados. Se retiró el trofeo y se ajustó el marco al contenido.
- Logo detallado, subtítulos, panel contextual y transición directa conservados. Sin cambios en cinemáticas, tiempos, mecánicas, concurrencia o dependencias.
- Actualizados únicamente `cinematic_sequence_test.cpp` y `responsive_ui_test.cpp`: bordes individuales, integridad de las líneas Unicode, estadísticas y presentación compacta en 60×20 y 80×20.
- `cmake --build build -j 2`: correcto. `NO_COLOR= TERM=xterm-256color COLORTERM=truecolor MAZE_UI_PREVIEW_DIR=/tmp/maze-ui-correction /tmp/maze-ui-check`: pasan los cuatro grupos de interfaz existentes en los cuatro tamaños originales y los dos compactos; capturas FTXUI revisadas.
- `python3 /tmp/maze-terminal-check.py`: vuelven a pasar las cuatro sesiones PTY, Enter al menú y Q. `ctest --test-dir build --output-on-failure` mantiene el fallo de mapas ya documentado; `/tmp/maze-all-check` mantiene 28/30 grupos aprobados y los mismos dos fallos previos.
- Commit correctivo: `fix(ui): restaurar tarjetas del menú y corregir tipografía de resultados`. Sin push.

## Ajuste de bloques y composición completa

- `src/ui_theme.cpp`: cada opción muestra icono, título y descripción, con variante compacta cuando falta altura. Se retiró CIRCUITO DE ESCAPE del encabezado principal; quedan el logo y el lema. Los resultados comparten los límites exteriores completos del menú y estadísticas con iconos, ancho adaptable y filas distribuidas.
- `src/menus.cpp`: se ajustó únicamente el cálculo de espacio para las tarjetas y el logo; se mantiene el panel contextual derecho cuando hay espacio. Títulos Unicode, cinemáticas, tiempos y lógica del juego conservados.
- `tests/responsive_ui_test.cpp`: comprobaciones mínimas de iconos, descripciones, ausencia del subtítulo y coincidencia de las esquinas exteriores de menú, derrota y victoria.
- `cmake --build build -j 2`: correcto. Los cuatro grupos existentes de interfaz pasan en 80×24, 100×30, 120×35 y 160×45, además de las variantes compactas 60×20 y 80×20. Capturas FTXUI con color revisadas en `/tmp/maze-ui-layout` mediante `NO_COLOR= TERM=xterm-256color COLORTERM=truecolor MAZE_UI_PREVIEW_DIR=/tmp/maze-ui-layout /tmp/maze-ui-check`.
- La suite completa conserva los dos fallos de mapas preexistentes (28/30 grupos aprobados); no se modificaron esas pruebas ni algoritmos.
- `NO_COLOR= python3 /tmp/maze-terminal-check.py`: pasan las cuatro sesiones PTY con color, victorias, captura, Enter al menú y Q; también Q durante el inicio de ambos ejecutables. Mapas sin cambios.

## Presentación final de resultados

- Renderer compartido en `src/ui_theme.cpp`: encabezado delgado con el resultado, borde cian oscuro, títulos originales intactos, panel FINAL STATS centrado de 55 columnas con padding, iconos, valores blancos y filas alternadas. Controles cian agrupados debajo del panel.
- Ajustadas únicamente las expectativas de los controles en `animations_test.cpp`, `cinematic_sequence_test.cpp` y `responsive_ui_test.cpp`; se conservaron las comprobaciones de tiempos, motivos y valores. Menú, cinemáticas y lógica sin cambios.
- Compilación correcta y cuatro grupos existentes de interfaz aprobados. Ambas pantallas comprobadas en 80×24, 100×30, 120×35 y 160×45: título del panel único y centrado, ancho de 55 columnas, filas de una línea con 0–1 líneas de separación y controles visibles. Capturas FTXUI revisadas en `/tmp/maze-ui-final`.
- PTY real: victorias en easy/medium/hard y captura; Enter al menú y Q funcionan, incluidos los inicios. Los mapas permanecen intactos. CTest mantiene el fallo preexistente `Map grid and graph do not match`.
- Adaptación: en 80×24, la victoria usa indicaciones cian en una sola línea para conservar el título completo y las cuatro estadísticas; las demás vistas con espacio muestran recuadros. El trazo exacto de los caracteres depende de la fuente de la terminal; se usan los glifos originales y no imágenes rasterizadas.
