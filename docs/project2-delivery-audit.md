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
| Mapas listos para jugar | Tres assets preparados en `assets/maps`; `Project2Application` ya carga la dificultad elegida. No genera mapas al iniciar. |
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
