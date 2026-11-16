# Entrega técnica del Proyecto 2

Base: `main` cff8778, que incorpora el PR #140. Rama: `feat/entrega-final`.
El documento histórico `project2-delivery-audit.md` describe también flujos retirados;
esta auditoría corresponde al flujo offline actual.

## Fuente y alcance

Cotejo de `proyecto_2_2026_2.pdf`, «Proyecto 2: Circuito de Escape - agentes que
planifican», Rubén Rivas Medina, octubre de 2026, 14 páginas. Rúbrica: sección 9,
página impresa 12 (página 13 del PDF). SHA-256 del documento consultado:
`97e3241dd1777b4e62237c0d4245c840f41fb1bbda7e401502b12121a9c4a049`.
No se añade ni se modifica el PDF. La exposición individual obligatoria queda a
cargo del equipo; esta auditoría no asigna una nota ni sustituye la evaluación.

La base `ecf4ecf` ya aprobó el [run 43760665268](https://github.com/leonardolandeo-a11y/Maze/actions/runs/43760665268).
El cotejo encontró dos faltantes respecto a §2.3, §3.3 y apéndice A:

- H ya calculaba `segment_costs`, pero no mostraba los gastos de los tramos.
  Ahora los presenta en `CURRENT -[costo]-> A/B -[costo]-> EXIT`, junto a
  pendientes/activados, salida con requisitos A/B o habilitada y saldo.
- El diagnóstico de mapa inaccesible era genérico. Ahora identifica los tramos
  fallidos, por ejemplo `START->A` o `B->EXIT`.

No se cambian algoritmos, reglas, mapa seleccionado, FTXUI ni preparación offline.
La regresión nativa reabre H después de moverse y activar interruptores, suma los
costos mostrados, comprueba los objetivos omitidos y usa Esc para volver sin
consumir turnos. El nuevo instalador solo se entrega después de aprobar Windows.

## Las 32 comprobaciones de la rúbrica

Los identificadores conservan el orden de los ocho criterios y sus cuatro
comprobaciones. «Cubierta» indica evidencia técnica en código/pruebas; las pruebas
visuales y exposición pendientes se separan al final. Las funciones de pruebas
citadas se ejecutan mediante `Maze_test` en CTest Release, con asserts activos.

| ID / criterio (máximo) | Comprobación oficial | Código y evidencia | Estado técnico |
|---|---|---|---|
| R1 Reutilización (2) | Grid configurable conserva Position, vecindad e iteradores | `grid.h`, `grid_test.cpp`, `delivery_acceptance_test`: 10×15, 20×30, 40×60 | Cubierta |
| R2 | Amplía motor conservando movimiento, energía y reset | `Project2Session` compone `NavigationEnvironment`; `project2_navigation_engine_parity_test` compara cada turno, reinicio y nueva reproducción con el verificador | Cubierta |
| R3 | Adapta controladores y usa utilidad genérica | `PolicyController<GuardBfsPolicy>`/`IController`; `NavigationEnvironment` usa `countMatching`/`LinearSearch`; tests controllers/generic_functions/environment | Cubierta |
| R4 | README identifica adaptaciones, usos y pruebas | Tabla «Reutilización comprobable», seis componentes originales y pruebas enlazadas | Cubierta |
| K1 Grafo y Kruskal (3) | Grafo y DisjointSet propios funcionan | `graph.cpp`, `graph/disjoint_set.cpp`; graph_test/disjoint_set_test | Cubierta |
| K2 | Kruskal conectado, sin ciclos, costo mínimo | `kruskal.cpp`; V−1 aristas y conectividad, comparación con Prim independiente en `test_dimensions_and_minimum_cost`, costo conocido 1 050 488 | Cubierta |
| K3 | Aperturas adicionales distintas del árbol | `tree_openings`/`extra_openings`; `test_seed_and_extra_paths` compara conjuntos y árbol original | Cubierta |
| K4 | Semilla reproduce mapa | RNG local y desempates fijos; `check_same_maze`, assets reproducidos en `delivery_acceptance_test` | Cubierta |
| V1 Validación/dificultad (2) | Dijkstra compara ambos recorridos completos | `validate_map_routes`; `basic_order_and_path_test`, `choose_b_first_test`, `directed_one_order_test` | Cubierta |
| V2 | Tramos inaccesibles, energía/recargas según §2.3 | `unreachable_exit_test`, `unreachable_switch_diagnostic_test`, `energy_threshold_test`, `forced_battery_detour_test`; recargas en motor sin pesos negativos | Cubierta; diagnóstico completado |
| V3 | Mínimo/promedio/máximo y desempates | `select_maps`; `test_basic_selection` 10/18/32, empates por ID, media decimal, independencia del orden | Cubierta |
| V4 | Victoria comprobada por nivel y fallo si faltan tres costos | `validate_map`, `GameVerifier::replay`, `select_maps`; `route_does_not_prove_win_test`, tests no ganadores/menos de tres costos y CLI código 2 | Cubierta |
| H1 Dijkstra/ayuda H (3) | Dijkstra propio de menor gasto compartido | `pathfinding/dijkstra.cpp` usado en validador y sesión; `test_cheapest_path`, `test_generated_mazes` contrasta BFS de costo unitario | Cubierta |
| H2 | Compara órdenes y omite interruptores activos | `Project2Session::help_report`; `project2_dijkstra_help_progression_test`; terminal reabre después de activar A/B | Cubierta |
| H3 | Costos y saldo distinguen suficiente/cero/insuficiente | `format_project2_help_lines`; `project2_active_help_text_test` prueba saldo 1/0/−1, sin camino, tramos y estados; responsive conserva legibilidad | Cubierta; tramos/estados completados |
| H4 | H actualiza y detiene turnos | Handler recalcula al abrir; integración y terminal comparan turno, energía, jugador, guardia y switches antes/después, también tras moverse; H/Esc cierran | Cubierta |
| B1 Vigilante BFS (2) | BFS propio minimiza movimientos | FIFO en `GuardBfsPolicy`; tests shortest_path/detours y controlador vía IController | Cubierta |
| B2 | Recalcula hacia jugador actual | `next_guard_position(guard, player)` recibe estado actual; prueba cambia objetivo, witness completo evita guardia móvil | Cubierta |
| B3 | Solo avanza en turnos pares | `Project2Session::step`/`GameVerifier::step`; `guard_even_turn_capture_test` y paridad motor/verificador | Cubierta |
| B4 | Muros, desempate fijo, espera sin ruta | Orden arriba/izquierda/derecha/abajo; tests BFS detours, tie_break y disconnected | Cubierta |
| P1 Reglas/partida (2) | A/B persistentes y necesarios | `exit_requires_both_switches_test`, `reverse_switch_order_test`, integración | Cubierta |
| P2 | Captura prioritaria al escape | `capture_priority_test`, `capture_on_enabled_exit_test`, paridad de la sesión real | Cubierta |
| P3 | Energía y límite, incluido último turno | `limit_and_wait_test`, batería consumida una vez, trampas reiteradas; casos de energía agotada/último turno en integración | Cubierta |
| P4 | Reset y victoria guardada | `project2_navigation_engine_parity_test`, `delivery_acceptance_test`; seis victorias nativas de mapas cargados | Cubierta |
| C1 Concurrencia (4) | Construcción realmente paralela | `run_map_batch` lanza varios `std::jthread`, cada tarea llama `build_map`; workers configurables 1/4, sin serialización del trabajo | Cubierta por código y ejecución |
| C2 | Validación paralela posterior a construcción | Dos llamadas separadas a `parallel_for`; destrucción de todos los jthread/join antes de la segunda; Dijkstra secuencial dentro de cada mapa | Cubierta por código y ejecución |
| C3 | Escritura segura y reproducibilidad | Slots reservados por índice, reparto atómico, RNG/distancias/colas locales; `deterministic_workers_test` compara grafos, aperturas, rutas, costos, witnesses y niveles; archivos 1/4 idénticos en Windows | Cubierta |
| C4 | Errores por mapa y tiempos separados | Try/catch por tarea, diagnósticos por ID; `dimensions_and_rejections_test`, `revalidation_clears_stale_results_test`; CLI emite Construction/Validation | Cubierta |
| E1 Pruebas/entrega/instalador (2) | CMake/C++20 y automatización de §7 | `CMakeLists.txt`, `Maze_test`, `windows-delivery.yml`; cobertura detallada de §7 debajo | Cubierta |
| E2 | Instala/juega/genera sin compilar | NSIS/PowerShell, ejecutables PE x64, DLL recursivas, mapas; `windows_acceptance.py` instala y ejecuta con PATH limitado a paquete+Windows | Cubierta en CI nativo |
| E3 | README documenta build/tests/instalación/ejecución/semillas/desinstalación | Secciones de inicio rápido, mapas reproducibles e instalador ya existentes; no se crea una guía aparte | Cubierta |
| E4 | Demuestra preparación y partida completa | Launcher recibe 15×20, carga tres niveles y reproduce seis victorias en ConPTY; secuencia de demo debajo | Cubierta en CI; exposición del equipo pendiente |

## Comprobaciones adicionales de la sección 7

- Kruskal: caso de costo conocido, árbol y extras separados (K2/K3).
- Validación: ambos órdenes, inaccesibilidad, energía >/=/< costo y recargas (V1/V2).
- Dijkstra/H: menor gasto y comparación BFS con pasos unitarios (`test_generated_mazes`);
  actualización y estado detenido (H1–H4).
- Dimensiones: generar/guardar/cargar/jugar 10×15, 20×30, 40×60 en
  `delivery_acceptance_test`; entradas inválidas en CLI/`interactive_test` y
  configuración sin espacio en `dimensions_and_rejections_test`.
- Niveles: 10/18/32, promedio 20, empates y menos de tres costos (V3/V4).
- Interruptores/salida y vigilante: ambos órdenes, salida anticipada, captura,
  rutas recalculadas y turnos pares (P1/P2/B1–B4).
- Partida guardada, reinicio, espera, movimiento inválido, agotamiento de energía,
  último turno y captura en salida: integración y game_verifier (P1–P4).
- Concurrencia: workers 1/4 y errores por tarea (C1–C4). Un lote vacío es rechazado
  por contrato antes de lanzar workers (`candidate_count=0` en
  `invalid_configuration_test`); selector vacío devuelve `nullopt` en
  `test_empty_collection`. No se confunde con un lote válido sin ganadores.

## Reproducción de comprobaciones

Linux:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel 4
ctest --test-dir build-release -C Release --output-on-failure
python -m pip install pyte==0.8.2 pexpect==4.9.0
python tests/delivery/terminal_playthrough.py --exe build-release/escape_circuit --maps assets/maps --evidence dist/linux-evidence
```

Windows: ejecutar el workflow **Windows Release delivery** en esta rama.
El script `installer/windows/build-installer.ps1 -BuildDirectory build-release` reutiliza
PowerShell/NSIS. Las dependencias de compilación y Python de pruebas son exclusivas
del runner; no se requieren para instalar ni jugar.
`Maze_test` mantiene aserciones activas en Release (`-UNDEBUG`/`/UNDEBUG`).
CTest informa 1/1 porque agrupa la suite C++ completa en un ejecutable.

Artefactos del workflow:

- `CircuitoEscape-Windows-x64-Release`: `CircuitoEscape-Setup.exe`, SHA256SUMS y commit de construcción.
- `Windows-validation-evidence`: CTest/JUnit, manifest instalado, resultados JSON,
  capturas de terminal ANSI/texto inicial/final y mapas de comparación workers=1/4.
- El instalador se publica como artefacto solo después de completar las pruebas.
  Retención: 90 días; descargar y conservar para la entrega.

## Recorrido exacto de demo

1. Descargar y extraer el artefacto del workflow aprobado. Ejecutar
   `CircuitoEscape-Setup.exe`; instalación por usuario sin administrador.
2. Abrir **Inicio → Circuito de Escape → Jugar**. Ampliar la terminal (la
   automatización usa 160 columnas × 60 filas). Esperar WELCOME, pulsar Enter;
   EASY es el nivel inicial; seleccionar START GAME con Enter.
3. Pulsar H: mostrar costos/ruta Dijkstra; pulsar 1, 2, 3 y 0 para resaltados.
   Pulsar Z dentro de ayuda y comprobar turno/energía constantes. H cierra ayuda;
   Z vuelve a FULL MAP.
4. En Easy incluido, introducir estas **46 teclas individuales**, por grupos
   solo para facilitar su lectura (sin espacios, sin Enter y sin movimientos extra):

   ```text
   wwwwww dd ww dddd ww dd ss dd ssss wwww aa ww aaaaaa wwww dd
   ```

   W=arriba, A=izquierda, S=abajo, D=derecha. Activan A/B y llegan a la salida;
   esperar cinemática y pantalla VICTORY. Esta secuencia corresponde únicamente
   al mapa Easy incluido, ID 3, seed 3668352356. H ofrece una estimación energética
   de terreno, sin simular al vigilante, baterías, penalizaciones de trampas ni
   esperas; no sustituye al witness ganador.
5. Tras la animación, Enter vuelve al menú. En DIFFICULTY elegir MEDIUM y luego
   HARD, iniciar y mostrar carga del nivel y H/Z. Q cierra el juego.
6. Abrir **Generar mapas**. Escribir `15`, Enter, `20`, Enter. Esperar los tres
   niveles guardados; pulsar una tecla al terminar. Abrir **Jugar mapas generados**:
   mostrar SIZE 15×20, seleccionar dificultad e iniciar. El juego no regenera.
7. Cerrar con Q, reabrir y comprobar que permanece el mismo mapa. **Desinstalar**
   elimina programa/accesos/registro; conserva los mapas de
   `%LOCALAPPDATA%\CircuitoEscape\generated-maps`.

El perfil interactivo usa energía 1000, límite 2000, 15 aperturas extra y budget
100; semilla 42 para 40×60 y 12345 en otros tamaños. Es el perfil determinista de
aceptación de dimensiones; no incluye ítems. Los mapas incluidos sí conservan los
recursos, baterías y trampas de Proyecto 1 (energía 100/límite 240).
Los parámetros avanzados siguen disponibles con `map_generator --help`.
Si un lote no obtiene tres costos distintos ganadores, devuelve 2 con diagnóstico;
no anuncia éxito ni ofrece un lote incompleto como válido.

## Resultados y límites

- Linux GCC 15.2 Release: CTest 1/1 PASS, 1.05 s. Terminal real PTY:
  Easy 46, Medium 56 y Hard 68 acciones hasta VICTORY; H/Z y Q correctos;
  los tres archivos conservan contenido y mtime.
- Windows de la base `ecf4ecf`: [run 43760665268 aprobado](https://github.com/leonardolandeo-a11y/Maze/actions/runs/43760665268), seis victorias,
  instalación/generación/desinstalación y reproducibilidad 1/4. Su SHA-256 de Setup:
  `d18dedb3fb6d5fb1f8e9f2fc867f1530050ad685c9df997637edf4d963673584`.
  El [PR #141](https://github.com/leonardolandeo-a11y/Maze/pull/141) registra el run,
  artefacto y SHA-256 vigentes después de validar los ajustes de esta auditoría.
  En runs de pull_request, BUILD.txt identifica el merge de prueba de GitHub;
  el `head_sha` del run identifica el commit de la rama.
- Prueba humana pendiente: apariencia de emojis/fuentes, interacción por ratón
  con accesos del menú Inicio, instalación/desinstalación con sus ventanas visibles,
  y experiencia visual en Windows 11 limpio. El runner Windows Server 2022 con
  PATH restringido prueba funcionalidad nativa, pero no equivale a esa inspección.
- Las 32 comprobaciones se cotejaron con la rúbrica oficial identificada arriba;
  exposición individual y valoración docente no se atribuyen a CI.
