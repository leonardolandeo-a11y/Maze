# Contribuciones técnicas

Resumen de aportes incorporados hasta `052413d`, contrastado con el historial de Git, los pull requests de integración y el [registro anterior](https://github.com/leonardolandeo-a11y/Maze/blob/fa2342014c3c6353d3350455156063c2b9354f61/docs/contributions.md). Las referencias permiten consultar los cambios y su integración en los componentes compartidos.

## Leonardo Landeo — [@leonardolandeo-a11y](https://github.com/leonardolandeo-a11y)

- **Proyecto 1:** estructura inicial, tipos y tablero, perfiles `GameRules`, política heurística, integración de efectos, `ConsoleUI`, ciclo `GameApplication` y estadísticas de partida.
- **Proyecto 2:** `DisjointSet`, selección de mapas por costo, validación de entradas del selector, integración de menús y ayuda, configuración de tamaños y preparación de los paquetes de Windows y Ubuntu.
- **Integración, pruebas y documentación:** pruebas del selector y de entrega, aceptación del instalador Windows, instrucciones de instalación y consolidación del README.
- **Referencias:** [base del motor y tablero](https://github.com/leonardolandeo-a11y/Maze/commit/5d6e9c3), [DisjointSet](https://github.com/leonardolandeo-a11y/Maze/commit/d8843fd), [selector y pruebas](https://github.com/leonardolandeo-a11y/Maze/commit/3cd2b67), [entrega Windows](https://github.com/leonardolandeo-a11y/Maze/pull/141), [paquete Ubuntu](https://github.com/leonardolandeo-a11y/Maze/pull/142).

## Fabricio Nick — [@SRHOUSE](https://github.com/SRHOUSE)

- **Proyecto 1:** núcleo de `NavigationEnvironment`, observaciones y resultados, integración de eventos, política aleatoria y simulación reproducible, y animaciones de inicio y resultados.
- **Proyecto 2:** dimensiones dinámicas, generación Kruskal, Dijkstra, validación y búsqueda de victorias, lotes paralelos, persistencia y CLI; integración del motor y del controlador BFS en la sesión.
- **Integración, pruebas y documentación:** integración de grafo y ayuda, pruebas de simulación y reglas, reproducción de mapas guardados, correcciones de contratos y documentación de reutilización.
- **Referencias:** [motor original](https://github.com/leonardolandeo-a11y/Maze/commit/d658b13), [Kruskal](https://github.com/leonardolandeo-a11y/Maze/commit/90938d7), [reutilización del motor](https://github.com/leonardolandeo-a11y/Maze/commit/b43bf49), [integración y pruebas del Proyecto 2](https://github.com/leonardolandeo-a11y/Maze/pull/128).

## André Mejía — [@andremejia-hub](https://github.com/andremejia-hub)

- **Proyecto 1:** costos de movimiento, concept `NavigationPolicy`, escenarios 1 y 2 y ajustes de sus recorridos y recompensas.
- **Proyecto 2:** representación del grafo por listas de adyacencia, corrección del orden de vecinos y ajuste de separación de objetivos en los mapas generados; reporte de turnos de la secuencia ganadora.
- **Integración, pruebas y documentación:** pruebas del motor y escenarios, integración del grafo y regresiones para la distancia mínima entre objetivos.
- **Referencias:** [costos de movimiento](https://github.com/leonardolandeo-a11y/Maze/commit/af365be), [concept](https://github.com/leonardolandeo-a11y/Maze/commit/2e087e1), [grafo](https://github.com/leonardolandeo-a11y/Maze/commit/1800f73), [colocación de objetivos y pruebas](https://github.com/leonardolandeo-a11y/Maze/pull/148).

## Jared Chala — [@jaredchala-bot](https://github.com/jaredchala-bot)

- **Proyecto 1:** reglas de recursos y baterías, `PolicyController`, validaciones del tablero, integración del menú y controles, renderizado de consumibles y pantalla de ayuda.
- **Proyecto 2:** representación de interruptores y vigilante, estados del HUD, presentación del recorrido y balance energético de H y resaltado de rutas.
- **Integración, pruebas y documentación:** templates, traits, fold expressions y controladores polimórficos; pruebas de eventos, funciones genéricas y renderizado, y documentación de la prueba negativa del concept.
- **Referencias:** [adaptador de políticas](https://github.com/leonardolandeo-a11y/Maze/commit/6a78cbd), [templates y pruebas](https://github.com/leonardolandeo-a11y/Maze/commit/0231ae8), [reportes y rutas](https://github.com/leonardolandeo-a11y/Maze/commit/1a5d8fd), [integración de ayuda](https://github.com/leonardolandeo-a11y/Maze/pull/127).

## Iker García — [@iygt8-iterate](https://github.com/iygt8-iterate)

- **Proyecto 1:** interfaz `IController`, interfaz de renderizado del tablero, layout del menú, navegación reutilizable y transiciones entre pantallas.
- **Proyecto 2:** recuperación del flujo de juego con mapas preparados, unificación de la presentación adaptable al tamaño de terminal y restauración de las cinemáticas originales antes de los paneles finales.
- **Integración, pruebas y documentación:** README y decisiones de diseño, pruebas de persistencia del flujo offline, tamaños de terminal y secuencias de animación; documentación de esas integraciones.
- **Referencias:** [IController](https://github.com/leonardolandeo-a11y/Maze/commit/20ace07), [flujo offline](https://github.com/leonardolandeo-a11y/Maze/pull/137), [presentación adaptable](https://github.com/leonardolandeo-a11y/Maze/pull/135), [restauración de cinemáticas](https://github.com/leonardolandeo-a11y/Maze/pull/139).

## Integraciones compartidas

El [PR #128](https://github.com/leonardolandeo-a11y/Maze/pull/128) reúne el motor y los algoritmos con el grafo y la ayuda desarrollados en otras ramas. Los [PR #135](https://github.com/leonardolandeo-a11y/Maze/pull/135) y [#139](https://github.com/leonardolandeo-a11y/Maze/pull/139) continúan la interfaz y las animaciones previas. Los paquetes y pruebas de entrega reutilizan esos componentes integrados. Estas referencias describen cambios incorporados; no presuponen revisiones o aprobaciones individuales que no estén registradas.

La identificación de los integrantes y los datos académicos pendientes se indican en la sección [Equipo](../README.md#equipo) del README.
