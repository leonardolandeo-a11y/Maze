# Proyecto 2 — auditoría de preparación de entrega

Fecha: 2026-11-06. Issue #132. Base: main f472a19, incluido PR #130.

## Fuente y alcance

Se auditó el checklist detallado proporcionado por el responsable del repositorio
para CS2013 Proyecto 2 — Circuito de Escape y los criterios de #117, #119, #125 y
#129. No se encontró un enunciado/rúbrica oficial adjunto en el repositorio; se
solicitó su enlace. Esta evidencia no sustituye cotejar el documento oficial ni
promete una nota 20/20. La matriz de reutilización Proyecto 1 → Proyecto 2 está en
README. No hubo cambios de audio/música ni trabajo en #131.

## Evidencia por requisito

| Requisito | Implementación existente y evidencia ejecutable |
|---|---|
| Dimensiones runtime, rechazo de configuración inválida | Grid dinámico; `map_batch.cpp`/CLI; grid, batch y CLI tests. Aceptación completa 10×15, 20×30, 40×60 en `delivery_acceptance_test`. |
| Graph, DSU y Kruskal propios | `graph.cpp`, `graph/disjoint_set.cpp`, `map_generation/kruskal.cpp`; pruebas de grafo, compresión de caminos/unión por tamaño, conectividad, ausencia de ciclos del árbol, pesos positivos, seed y aperturas extra separadas. |
| Dijkstra propio | `pathfinding/dijkstra.cpp`, `map_validator.cpp`; pruebas de costo de entrada destino, ambos órdenes A/B, tramos inaccesibles, mínimo total y energía estrictamente mayor al costo. Las baterías requieren simulación, nunca pesos negativos. |
| Selector único | `map_selector.cpp`; caso 10/18/32 (promedio 20), empates por ID menor, negativos incluso no verificados, duplicados, exclusión de no ganadores y menos de tres costos distintos. |
| Persistencia completa | `map_storage.cpp`; dimensiones, celdas/ítems/valores, costos, posiciones, energía, turnos, ID/seed/dificultad, costo y witness. Pruebas de roundtrip y rechazo de corrupción. Los ítems se reconstruyen en orden de filas; se comparan posiciones/valores, no el orden original de generación. |
| Mapas listos para jugar | Tres assets preparados en `assets/maps`; la carga directa por `--difficulty` los conserva. El menú interactivo genera por tamaño antes de ofrecer la dificultad. |
| Gameplay | `GameVerifier`, `Project2Session` y motor original; switches en ambos órdenes, persistencia, salida bloqueada, BFS hacia jugador actual y orden determinista, muros/límites/sin ruta, turnos pares, captura antes de victoria, espera/acción inválida, energía, último turno y reset. Nueva regresión de captura en salida habilitada, tanto al entrar como tras mover el guardia. |
| Ayuda H | Pruebas de ayuda inicial, tras A, tras B y tras ambos; costos/ruta/energía/estado; sin modificar partida. Handler de H/Z revisado y recorrido en pseudoterminal. |
| FULL MAP / DETAIL | PR #130 preservado sin cambios en renderer; `project2_responsive_layout_test` cubre 10×15, 20×30, 30×40, 40×60 y otros aspectos; cámara y estado estable. |
| Concurrencia | Dos llamadas separadas a `parallel_for`; destructores `std::jthread` esperan todos; IDs/seeds precomputados, distribución atómica y un slot por candidato, captura de errores por mapa, clasificación secuencial. Comparación workers=1/4 reforzada: aperturas completas, grafos, rutas de ambos órdenes, costos, witnesses y selección. Tiempos independientes no se comparan por igualdad. |
| Fallos | Configuración inválida lanza error; candidate_count=0 inválido por contrato (rango 1..10); fallo de construcción/validación no produce mapas seleccionables; diagnóstico retenido y revalidación limpia resultados viejos. CLI devuelve 2 si no hay tres costos distintos. |
| Instalador, solo fuentes | CMake instala ejecutables, README, launchers y tres mapas; PowerShell usa assets versionados y Release por defecto, copia dependencias DLL recursivas y comprueba arranque; NSIS define shortcuts/desinstalador. No se ejecutaron PowerShell/NSIS ni instalación Windows. |

## Mapas y reproducibilidad

Configuración de entrega: 20×30, seed base 12345, 10 candidatos, workers 4,
energía inicial/máxima 100, límite 240, 15 aperturas extra, terreno 15%/costo 2,
budget 30000 estados, 5 recursos de 10 puntos, 3 baterías de 5, 3 trampas de
penalización energética 2/puntaje 1. Comando completo en README.

| Mapa | ID | Seed candidato | Costo | Witness cargado en Project2Session |
|---|---:|---:|---:|---|
| Easy | 3 | 3668352356 | 51 | Victoria |
| Medium | 5 | 387289302 | 61 | Victoria |
| Hard | 9 | 2415097786 | 76 | Victoria |

8/10 candidatos verificados. Repetir con workers=1 produjo exactamente los tres
archivos de workers=4 (`cmp` de cada archivo exitoso). Los tiempos varían.
La suite regenera el lote y compara sus mapas con los assets versionados.

Aceptación rápida por dimensiones (no son los assets de entrega): energía 1000,
límite 2000, 15 aperturas extra, 10 candidatos, 4 workers, budget 100, sin ítems;
resto de valores por defecto.

| Tamaño | Seed base | Easy ID/costo | Medium ID/costo | Hard ID/costo | Resultado |
|---|---:|---|---|---|---|
| 10×15 | 12345 | 3/9 | 1/20 | 2/30 | Guardar/cargar/replay: victoria en los tres |
| 20×30 | 12345 | 3/51 | 6/63 | 9/76 | Guardar/cargar/replay: victoria en los tres |
| 40×60 | 42 | 10/146 | 7/184 | 1/267 | Guardar/cargar/replay: victoria en los tres |

El intento 40×60 seed 12345 con budget 100 produjo solo dos victorias y código 2;
se eligió seed 42 determinista, sin cambiar validación ni fabricar mapas.

## Validación realizada

```bash
cmake -S . -B build
cmake --build build --parallel
cmake --build build --target Maze_test map_generator escape_circuit
./build/Maze_test
ctest --test-dir build --output-on-failure
./build/map_generator --help
./build/escape_circuit --help
```

Build y Maze_test: PASS. CTest: 1/1 PASS (el ejecutable agrupa toda la suite,
no una sola aserción), aproximadamente 6 segundos. Se mantienen aserciones en
Release con `-UNDEBUG` o `/UNDEBUG` para no reportar falsos positivos.
Generación CLI ejecutada en los tres tamaños y en workers=1/4 para el lote default.
Arranque real Linux en pseudoterminal de 160×65: Easy/Medium/Hard con mapas default,
H, Z (dentro y fuera de ayuda), Q y retorno 0. Esto es un smoke de interacción,
no una prueba visual de fuentes ni una instalación Windows.

## Issues anteriores y trabajo restante

#117, #119, #125 y #129 ya estaban **cerrados** al consultar GitHub; sus criterios
implementados conservan cobertura verde. No se reabrieron ni se modificaron.
Issue #132 y su PR deben permanecer abiertos hasta revisión/merge por el equipo.

Para completar entrega y evaluar 20/20: cotejar la rúbrica oficial, revisar el PR,
mergear en una tarea posterior, compilar Release, generar el instalador NSIS,
probar en Windows limpio (instalar, tres dificultades, H/Z, generador, partida
completa, desinstalar y conservación de mapas personales), preparar demostración
y exposición individual, y finalmente publicar/entregar según el curso.
No se creó Setup.exe, MSI, ZIP de entrega, GitHub Release ni paquete final.

## Nuevo requisito: elección de tamaño desde la UI (#132 / #133)

Flujo implementado: Main → Start Game → SELECT MAP SIZE → generación/validación
con animación → Difficulty → gameplay. Presets 10x15/20x30/40x60 y CUSTOM con reglas centrales
en map_sizes; CLI 100x150 devuelve error sin archivos.
La generación usa exactamente el perfil de aceptación rápida de la tabla anterior.

Persistencia por directorios generated-maps/<tamaño>; el worker guarda y vuelve a
cargar los tres mapas antes de publicar éxito. Future sincroniza resultados con
el hilo UI; temporizador de 60 ms envía Custom y solo el hilo UI avanza frames,
menús y sesión. Entradas de gameplay se ignoran durante carga. Q espera el lote
al salir, sin threads separados que sobrevivan a la aplicación.
Errores de generación, costos insuficientes, escritura o carga vuelven a Main
con mensaje. No se ofrece un directorio anterior como resultado de un fallo.

Pruebas nuevas: lista exacta y Back, dificultad posterior a éxito, retorno a menú
tras error, tamaños inválidos/100x150, configuración propagada, tres dificultades
con costos crecientes, roundtrip de dimensiones, witness ganador, aislamiento de
tamaños, estado loading sin directorio jugable y fallos de filesystem/asíncronos.
Se mantiene la suite previa de H/Z y layout responsive de PR #130.

Comprobación interactiva en pseudoterminal Linux 120x45: selector → carga animada →
Easy (10x15), Medium (20x30), Hard (40x60), H, Z dentro/fuera de ayuda y Q, exit 0.
Inspección del frame 40x60: HUD, tablero completo y controles dentro del viewport;
DETAIL conserva zoom. No sustituye una prueba visual en terminal Windows.

Sin cambios de audio/música/#131, sin makensis, instalador, ejecutable final ni
Release. PR #133 se conserva abierto y la rama no se elimina.

Validación final: configuración CMake y build correctos; Maze_test PASS; CTest
1/1 PASS (6.20 s).

## CUSTOM y selector visual (#132 / #133)

Panel FTXUI centrado, borde redondeado, título cian, selección invertida, etiquetas
SMALL/RECOMMENDED/LARGE y pantalla CUSTOM con campos numéricos, área y estado vivo.
Tab/flechas cambian foco; Enter avanza/confirma; escribir reemplaza el valor al
entrar al campo; Backspace elimina y Escape vuelve. Letras/pegados no numéricos no
entran en los campos. Una entrada vacía o inválida no emite generación.

Reglas únicas en map_sizes: 5–40 filas, 5–60 columnas, área ≤2400,
rows ≤ columns ≤ 2×rows. El área máxima es redundante con los límites por eje.
Se valida antes de crear workers, archivos o candidatos. El mínimo 5x5 usa también
corredores transitables para colocar los cinco actores/objetivos distintos.

El preflight del renderer comparte el cálculo de panel con render_project2:
requiere todas las columnas, altura para filas comprimidas 2:1 y margen para HUD/
controles. Si falta espacio, no inicia worker. DETAIL conserva cámara; no se
agregaron casos especiales de render por tamaño. El CLI valida dimensiones sin
necesitar terminal. Los mapas se guardan en generated-maps/<rows>x<columns>/.

Validación ejecutada:
- cmake -S . -B build y cmake --build build --parallel: PASS (build de desarrollo).
- ./build/Maze_test: PASS; ctest --test-dir build --output-on-failure: 1/1 PASS.
- Generación de tres dificultades, persistencia aislada, dimensiones cargadas y
  witness ganador para 10x15, 20x30, 40x60, 30x40, 15x20 y 5x5: PASS.
- Rechazo previo de 100x150, 50x50, 40x70, 41x60, 40x61, 4x5, 0x0, overflow,
  cadenas vacías/no numéricas; 10x60 y 40x10 rechazados por proporciones: PASS.
- Layout FULL MAP/DETAIL estable para estos tamaños en terminales 80x40, 120x50
  y 160x60; suite responsive heredada de PR #130: PASS. GitHub informa PR #130
  MERGED sin checks asociados; no se atribuye un check remoto inexistente.
- UI real FTXUI mediante teclado automatizado en pseudoterminal Linux 120x50:
  10x15, 20x30, 30x40 CUSTOM, 40x60 y 15x20 CUSTOM → carga animada → juego → H/Z
  dentro/fuera de ayuda → Q, exit 0: PASS. Frames de formulario inspeccionados.
- UI: 100x150 muestra máximo 40×60/2400; 10x60 y 40x10 muestran proporciones
  soportadas; Enter no genera carpetas: PASS. Terminal 80x24 rechaza antes del
  worker con mensaje visible. Esta prueba no sustituye inspección humana en Windows.

No cambios a audio/música/#131 ni generación de installer/Release/exe final.

## Visual polish — issue #132 / PR #133 (2026-11-06)

- Project 2 Main Menu and Difficulty now share a centered, compact arcade panel,
  the existing MAZE block logo, cyan borders/focus, green configuration summary,
  and visible keyboard hints. The existing options and event handling are unchanged.
- Map Size and Custom share that panel, aligned presets, full-row selection,
  rows/columns hierarchy and green/red validation messages. Validation itself is unchanged.
- `MapGenerationAnimation` renders a rounded standby panel, selected dimensions,
  a looping pulse and spinner. It claims neither percentage nor worker stage.
  It consumes the existing frame counter; worker, polling, timing, failure handling,
  quit semantics and asynchronous generation are unchanged.
- The existing HUD line now reads `SIZE rows×columns | DIFFICULTY level` in cyan,
  followed by switch state. No extra HUD row or board-space reservation was added.
- Startup, Victory and Death remain unchanged and were inspected as visual references.

Validation of this visual-only increment:

- `cmake -S . -B build`: passed (existing debug build).
- `cmake --build build --parallel`: passed.
- `./build/Maze_test`: passed, with all existing assertions retained.
- `ctest --test-dir build --output-on-failure`: passed, 1/1, 7.11 seconds.
- Actual Linux PTY at 120×50: 10×15, 20×30, 30×40 CUSTOM, 40×60 and
  15×20 CUSTOM completed selector → animated generation → difficulty → gameplay;
  H/Z inside and outside help and Q passed. Invalid 100×150, 10×60, 40×10
  retained visible rejection and created no generation directory; 80×24 retained
  terminal-size rejection before generation.
- Inspected FTXUI frame captures for Startup, Main Menu, Map Size, Custom,
  invalid Custom, three Loading frames, Difficulty, Gameplay, Victory and Death.
  Rendered all four requested dimensions in FULL MAP/DETAIL, with/without help,
  at 80×40, 120×50 and 160×60. Existing responsive regression tests also passed.
  Captures were generated locally in /tmp; this is Linux render/PTY evidence,
  not a claim of native Windows-terminal validation.
- No test source changes were needed. No gameplay, algorithm, generation, size
  rules, seeds, concurrency, save/load, H/Z handlers or audio/music/#131 changes.
  No installer, Release build or final executable package was generated.
