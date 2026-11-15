# Entrega técnica del Proyecto 2

Base: `main` cff8778, que incorpora el PR #140. Rama: `feat/entrega-final`.
El documento histórico `project2-delivery-audit.md` describe también flujos retirados;
esta auditoría corresponde al flujo offline actual.

## Alcance de la auditoría

Se revisaron el código, las pruebas y el checklist de la solicitud y del issue #132.
**El documento oficial de la rúbrica no está en el repositorio ni en los issues
consultados; su enlace se solicitó al responsable.** La siguiente matriz acredita
los criterios técnicos disponibles, no una comparación literal ni una nota de la
rúbrica ausente. Exposición, participación y otras actividades académicas quedan
fuera de esta entrega técnica.

## Matriz de evidencias

| Comprobación | Código revisado | Evidencia ejecutable |
|---|---|---|
| Reutilización de Proyecto 1 | `Project2Session::step` delega en `NavigationEnvironment`; `Grid<Cell>`, `Agent`, `GameRules`, `Observation`, `PolicyController<GuardBfsPolicy>`, `ConsoleUI`, menús y animaciones compartidos | `project2_navigation_engine_parity_test`, `project2_observation_contract_test`, pruebas originales de environment/interactions/controllers/grid y render. Matriz detallada de reutilización en README. |
| Dimensiones recibidas por terminal | `map_generator` conserva `--rows/--columns`; `--interactive` pide filas/columnas y valida antes de generar. Launcher del instalador usa ese modo | `interactive_test`: 10×15, 20×30, 40×60 y 15×20; entrada incompleta, no numérica, negativa, fuera de rango y proporciones inválidas sin crear mapas. |
| Grafo propio, Kruskal y DSU | Aristas ponderadas ordenadas con desempate estable; DSU con compresión por mitades/unión por tamaño; extras separados del árbol | `kruskal_test`: costo contrastado con Prim independiente, árbol de V−1 aristas conectado, paredes de borde, semilla y aperturas extra; `disjoint_set_test`, `graph_test`. |
| Dijkstra para validación | Cola de prioridad, costos de entrada no negativos, reconstrucción; compara inicio→A→B→salida e inicio→B→A→salida | `dijkstra_test`: ruta barata distinta de ruta corta, inaccesibilidad, empates, overflow, datos inválidos; `map_validator_test`: ambos órdenes, energía > costo, baterías requieren simulación. |
| Mapas ganables, reglas de partida | Verificador simula; sesión usa motor P1. Captura antes de victoria, guardia en turnos pares, energía positiva y ambos interruptores para salir | `game_verifier_test`: captura al entrar/salir, A/B en ambos órdenes, salida bloqueada, espera, muros, límites, último turno, baterías una vez, trampas repetidas; integración contrasta sesión/verificador y reset. |
| BFS del vigilante | `GuardBfsPolicy` usa FIFO y vecinos arriba/izquierda/derecha/abajo, persigue posición actual | `controllers_test`: ruta, desempates, sin ruta, acciones legales, tres tamaños; `game_verifier_test`: movimiento en turnos pares/captura. |
| Tres dificultades | `select_maps`: mínimo, máximo e intermedio más cercano a la media de ganadores, empates por ID | `map_selector_test`: 10/18/32, empates, duplicados, negativos, no ganadores, menos de tres costos distintos. Mapas incluidos: Easy ID 3/costo 51, Medium ID 5/costo 61, Hard ID 9/costo 76. |
| Persistencia y juego offline | `save_selected_maps`, `load_map`; `Project2Application` comprueba presencia de los tres archivos y carga el elegido, sin llamar al generador | `map_storage_test`: roundtrip, corrupción, assets y `delivery_acceptance_test` completo en 10×15/20×30/40×60; prueba de terminal compara SHA-256 y mtime antes/después de jugar. |
| Ayuda H y Z | Dijkstra desde posición actual, objetivos pendientes, costos, energía, resaltado; Z cambia vista | Integración cubre antes/después de A/B, ambos órdenes, estado intacto. Terminal real: H, Z, 1/2/3/0, H, Z; compara turno, energía, jugador, guardia e interruptores; después gana la partida. |
| Concurrencia reproducible | Dos fases `parallel_for`, `jthread` con join al salir, IDs/seeds preasignados, índice atómico, slots separados, clasificación posterior | `deterministic_workers_test`: workers 1/4, grafos, aperturas, rutas, witnesses, selección y errores iguales. Windows compara archivos byte a byte de dos procesos del generador instalado. Tiempos se reportan separados y no se exige igualdad. |
| FTXUI conservado | Mismos renderers, cinemáticas y flujo de juego del main con #140 | Suite de animaciones, menús, responsive; terminal real recorre bienvenida, juego, ayuda, zoom y victoria. |
| Instalador autónomo x64 Release | PowerShell configura/compila Release, instala, resuelve DLL recursivamente con objdump y comprueba PE x64; NSIS instala por usuario, accesos y desinstalador | Workflow Windows compila, ejecuta CTest, empaqueta e instala el `.exe`; pruebas con PATH limitado al paquete + Windows. Manifest SHA-256 de ejecutables/DLLs. |
| Generar/jugar/desinstalar sin herramientas | Accesos Jugar, Generar mapas, Jugar mapas generados y Desinstalar. Mapas personales en LocalAppData separados del programa | `windows_acceptance.py`: registro/accesos/archivos, launcher con entrada 15×20, seis victorias (tres incluidas y tres generadas), desinstalación y preservación de mapas personales. |

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
El script `installer/build-installer.ps1 -BuildDirectory build-release` reutiliza
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
- Windows: resultado y enlace del runner se registran al finalizar la validación.
- Prueba humana pendiente: apariencia de emojis/fuentes, interacción por ratón
  con accesos del menú Inicio, instalación/desinstalación con sus ventanas visibles,
  y experiencia visual en Windows 11 limpio. El runner Windows Server 2022 con
  PATH restringido prueba funcionalidad nativa, pero no equivale a esa inspección.
- Cotejo literal contra la rúbrica oficial pendiente de recibir el documento.
