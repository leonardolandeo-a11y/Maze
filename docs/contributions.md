# Contribuciones del equipo

Este documento resume las principales responsabilidades y contribuciones de
cada integrante del equipo durante el desarrollo de **Circuito de Escape**.

Las contribuciones indicadas pueden verificarse mediante el historial de Git
del repositorio.

---

## Leonardo Landeo

GitHub: `@leonardolandeo-a11y`

Principales contribuciones:

- creación y organización inicial de la estructura del proyecto;
- implementación inicial de `Agent`, `Cell`, `Grid` y tipos básicos;
- implementación de los perfiles de dificultad y `GameRules`;
- implementación de `HeuristicPolicy` utilizando distancia Manhattan;
- integración de efectos de celdas dentro de `NavigationEnvironment`;
- implementación inicial de `ConsoleUI`;
- implementación de `GameApplication` y coordinación del ciclo interactivo;
- correcciones generales y reorganización del código;
- manejo de comandos desconocidos;
- implementación de estadísticas posteriores a la partida.

Commits representativos:

- `8101aab` — estructura inicial del proyecto.
- `5d6e9c3` — implementación de agent, cells, grid y types.
- `3f5a006` — perfiles de dificultad y `GameRules`.
- `80d4b17` — implementación de `HeuristicPolicy`.
- `716b11a` — `ConsoleUI`, key mapping y render inicial.
- `495fbc1` — integración de `GameApplication` y ciclo FTXUI.
- `78c6d65` — estadísticas posteriores a la partida.

---

## Fabricio Nick

GitHub: `@SRHOUSE`

Principales contribuciones:

- implementación del núcleo de `NavigationEnvironment`;
- definición de `Observation` y `StepResult`;
- implementación del estado y acciones disponibles del entorno;
- integración de eventos generados por el entorno;
- validaciones del estado y límites de energía;
- implementación de `RandomPolicy` reproducible;
- implementación de simulaciones automáticas reproducibles;
- integración de políticas y controladores;
- pruebas de controladores;
- pruebas de recursos, baterías y trampas;
- pruebas de consistencia entre eventos y estado;
- pruebas de simulación reproducible;
- implementación de animaciones de inicio, victoria y derrota.

Commits representativos:

- `a0c58cd` — estructura base de `NavigationEnvironment`.
- `c1aaf05` — `Observation` y tipos de resultado.
- `c2e26e8` — eventos y `StepResult`.
- `a5d9054` — `RandomPolicy` reproducible.
- `c54bb3e` — simulación automática reproducible.
- `8b768d6` — pruebas de políticas y controladores.
- `6ecdb6d` — pruebas de eventos e interacciones.
- `c8f4baf` — pruebas de simulación reproducible.

---

## André Mejía

GitHub: `@andremejia-hub`

Principales contribuciones:

- implementación de costos de movimiento;
- implementación del concept `NavigationPolicy`;
- creación de pruebas de `NavigationEnvironment`;
- implementación del Scenario 1;
- pruebas del Scenario 1;
- implementación del Scenario 2;
- mejora del diseño de los mapas de ambos escenarios;
- validación de rutas alcanzables;
- implementación y posterior eliminación del escenario aleatorio al quedar
  fuera de la versión final del proyecto;
- correcciones relacionadas con recompensas de recursos.

Commits representativos:

- `d45c7e7` — implementación de costos de movimiento.
- `2e087e1` — implementación de `NavigationPolicy`.
- `d7e66b7` — pruebas del entorno.
- `5e71854` — implementación del Scenario 1.
- `ccfe029` — implementación del Scenario 2.
- `26b05b9` — mejora del diseño del Scenario 1.
- `835ea62` — mejora del diseño del Scenario 2.

---

## Jared Chala

GitHub: `@jaredchala-bot`

Principales contribuciones:

- implementación de las reglas de `ResourceCell` y `Battery`;
- integración de efectos de recursos y baterías en el entorno;
- corrección de la actualización de energía de las baterías;
- implementación del adaptador genérico `PolicyController<Policy>`;
- validaciones adicionales para `Grid`;
- correcciones del sistema de puntuación de recursos;
- integración de las selecciones del menú con la configuración de la partida;
- ampliación y reorganización de `ConsoleUI`;
- soporte de movimiento mediante WASD y flechas;
- implementación y diseño de la pantalla de ayuda;
- separación del renderizado en barra superior, tablero y barra inferior;
- renderizado correcto de recursos y baterías consumidos;
- implementación y validación de templates de funciones;
- implementación de especialización total y parcial mediante traits;
- implementación de variadic templates y fold expressions;
- ampliación de las pruebas automáticas asociadas a estos requisitos.
- integración final de `std::unique_ptr<IController>` para controladores polimórficos;
- documentación y validación de la prueba negativa de `NavigationPolicy`;
- revisión y documentación final del proyecto.

Commits representativos:

- `aac2c26` — reglas de `ResourceCell` y `Battery`.
- `6a78cbd` — implementación de `PolicyController`.
- `7f550e6` — validaciones de `Grid` y corrección de puntuación.
- `ee7907d` — integración de selecciones del menú.
- `54c1ff8` — ampliación y reorganización de `ConsoleUI`.
- `41c4264` — renderizado de recursos consumibles.
- `71c67af` — templates y especializaciones.
- `0231ae8` — templates, fold expressions y validación de pruebas.

---

## Iker García

GitHub: `@iygt8-iterate`

Principales contribuciones:

- implementación de la interfaz polimórfica `IController`;
- definición de la interfaz de renderizado del tablero en `ConsoleUI`;
- creación del layout del menú principal;
- implementación de navegación reutilizable para los menús;
- implementación de transiciones entre pantallas del menú;
- actualización y ampliación del `README.md`;
- documentación de decisiones de diseño en `docs/design.md`;
- correcciones de documentación e información de integrantes.

Commits representativos:

- `20ace07` — implementación de `IController`.
- `8fbef14` — interfaz de renderizado del tablero.
- `ac653bf` — layout del menú principal.
- `680fc0d` — navegación reutilizable de menús.
- `32ff576` — transiciones entre pantallas.
- `5c28391` — ampliación del README.
- `42b483d` — documentación de diseño.

---

## Trabajo colaborativo

Además de las responsabilidades individuales, el equipo realizó de manera
colaborativa:

- integración mediante ramas y pull requests;
- corrección de conflictos entre ramas;
- revisión de funcionalidades desarrolladas por otros integrantes;
- validación de escenarios;
- ejecución de pruebas mediante CMake y CTest;
- corrección de errores encontrados durante la integración;
- preparación y validación de la versión final del proyecto.

Debido al uso de diferentes configuraciones locales de Git durante el
desarrollo, algunos commits históricos pueden aparecer bajo distintos nombres
de autor, pero corresponden a los integrantes indicados en este documento.