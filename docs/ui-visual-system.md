# Sistema visual FTXUI — #134 / Draft #135

## Alcance e historial

Iteración sobre `134-responsive-interface-redesign`, base de trabajo `b6d144c`.
Se inspeccionaron los 18 commits de `main..HEAD` y el diff completo de la rama.
No se reescribió ni eliminó su historial.

Se conserva la dirección de `c012f22`: Canvas, cards, selección cyan oscura y chips.
También se conservan la separación de navegación de `89054b0` y la idea de políticas
responsive de `ec5b9e2` / `3836a49`. Se reemplaza la implementación de métricas y
composición que dejaba cards demasiado altas en terminales pequeñas.
Los experimentos de tipografía gigante y previews de acciones de `1ed45f3`,
`49f7eb0`, `20f4b41`, `872fc28` y `60da3e4` no gobiernan la UI activa de Proyecto 2.
El arte legacy de `menusArt.h` y la navegación de Proyecto 1 se conservan.

## Componentes compartidos

`ui_theme.h/.cpp` contiene:

- Paleta RGB: fondo casi negro, superficies oscuras, cyan de identidad, verde
  de éxito, amarillo para advertencias y rojo para fallos. La paleta semántica
  existente de las celdas del mapa se mantiene para reconocer ítems y terreno.
- `metrics`: mide la terminal en cada render; viewport opcional para tests.
- `shell`: encabezado, pie y marco redondeado común.
- `panel`, `choice`, `chip`: jerarquía, selección y metadatos.
- `logo`: un único MAZE de Canvas, 23 columnas × 3 filas, con bloques alineados
  a la retícula 2×4 de FTXUI; no usa fuentes ASCII para las opciones.
- `circuit`: señal de actividad Canvas reutilizable; no finge porcentaje ni
  fases del worker.
- `result`: presentación común de victoria/derrota y estadísticas finales.

Los helpers no conocen el generador ni modifican el juego. `UILayout` deja de
forzar 90×20. `Project2Application` solo cambia colores de fondo/error; conserva
el loop, los flags, los eventos y el worker existentes.

## Pantallas y ritmo

Startup, Main, Difficulty, Render Mode, Help, Map Size, Custom, Loading, HUD,
ayuda H, FULL MAP / DETAIL y resultados comparten estos componentes.
Startup/victoria/derrota duran 36 frames (aproximadamente 2.16 segundos al ritmo
existente de 60 ms). Enter conserva el salto de intro; el retorno al menú en los
resultados solo aparece al terminar. No se añadieron threads, timers ni loops.
Loading mantiene su spinner y actividad indeterminada hasta que el worker termina.

El HUD conserva sus datos y el espacio de mapa. En terminales bajas elimina
separaciones decorativas y ajusta la altura del tablero. El muestreo de mapas
altos conserva la prioridad de jugador, vigilante y objetivos frente al terreno.
La ayuda y DETAIL usan el mismo espacio; renderizar no consume energía ni turnos.
La compresión extrema puede representar varias celdas en una sola posición visual;
DETAIL conserva la cámara para inspeccionarlas.

## Responsive y validación

| Terminal | Menús | Gameplay |
|---|---|---|
| 80×24 | Filas compactas, sin subtítulos | Sin separadores decorativos; mapa completo comprimido / DETAIL |
| 100×30 | Cards apiladas | HUD y tablero ajustados al alto |
| 120×35 | Navegación y resumen en dos columnas | Paneles compartidos |
| 160×45 | Dos columnas amplias | Se conserva la escala proporcional del mapa |

`tests/responsive_ui_test.cpp` renderiza cada pantalla a esas dimensiones,
comprueba requisitos de tamaño y contenido obligatorio visible, y cubre los
marcadores del mapa cuando se comprimen varias filas.
Las aserciones anteriores de símbolos, colores, navegación y ayuda permanecen.
Los tests que usaban un framebuffer de 140×55 ahora pasan ese viewport
explícitamente al renderer, en vez de depender de la terminal del proceso.

Comprobaciones realizadas en Linux:

- `cmake -S . -B build`
- `cmake --build build --parallel`
- `./build/Maze_test`
- `ctest --test-dir build --output-on-failure`: 1/1 aprobado.
- PTY real: resize ascendente/descendente entre las cuatro resoluciones en Main,
  Difficulty, Render Mode, Help, Map Size, Custom, gameplay generado, gameplay
  cargado, H, DETAIL, H+DETAIL, victoria y derrota.
- Generación desde Custom a 160×45, selección de dificultad y partida.
- Witness de Easy enviado al ejecutable hasta VICTORY; Enter vuelve a Main.
- Esperar hasta GAME OVER; Enter vuelve a Main. Todos los procesos salen con Q
  y código 0.
- Comparación del cuerpo de `Menus::OnEvent` con `b6d144c`: idéntico.
- Sin diff en algoritmos, generación, concurrencia, sesión, reglas, assets,
  persistencia ni puntos de entrada.

Para producir frames de revisión del renderer:

```bash
MAZE_UI_PREVIEW_DIR=build/ui-previews ./build/Maze_test
```

Escribe archivos `.txt` y `.ansi` para cada pantalla y tamaño. Para revisar RGB
en una sesión Linux que tenga NO_COLOR activado, ejecutar el comando con
`env -u NO_COLOR TERM=xterm-256color COLORTERM=truecolor`.
No se fuerza el soporte de color del usuario desde el juego.

## Portabilidad y pendientes

Se usa FTXUI para medir, componer, dibujar y refrescar. Encabezados, controles y
selección no dependen de emojis ni del ancho de glifos decorativos. Se conserva
el modo ASCII del mapa. No se añadieron rutas `_WIN32` ni dependencias nuevas.
Windows Terminal, fuentes, RGB y fallback de caracteres requieren comprobación
nativa; esta iteración solo se ejecutó en Linux.

El PR #137 offline ya está en origin/main, pero no se incorporó en esta rama:
por instrucción, aquí se conserva la semántica de Map Size/Custom/Generation.
La reconciliación posterior deberá retirar esas rutas manteniendo `MazeUI`,
sus helpers y las pruebas de las pantallas restantes. No se cambia aquí el
rechazo existente de generación en terminales pequeñas; se puede cargar un mapa
preparado y jugar/redimensionar a 80×24.

La intro y los resultados también son compartidos por el ejecutable legacy Maze;
su presentación cambia, no sus reglas ni el menú legacy.
Audio/música/#131 no se modificaron. El PR debe permanecer Draft y sin merge.
