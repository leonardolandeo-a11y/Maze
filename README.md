# Circuito de Escape — Maze

Motor de navegación por turnos desarrollado en **C++20** para el curso **CS2013 — Programación III**. El proyecto separa el motor de simulación de la interfaz de consola, utiliza **CMake** para la construcción y **FTXUI** para la presentación interactiva.

---

## 1. Información del proyecto

| Campo | Información |
|---|---|
| Curso | CS2013 — Programación III |
| Proyecto | Proyecto 1 — Circuito de Escape |
| Semestre | 2026-2 |
| Grupo | 5 |
| Lenguaje | C++20 |
| Construcción | CMake |
| Interfaz | Consola con FTXUI |
| Rama de entrega | `main` |
| Tag de entrega | `proyecto-1-entrega` |

### Integrantes

| Integrante | Código UTEC | GitHub |
|---|---:|---|
| **Leonardo Landeo** | `202520151` | `@leonardolandeo-a11y` |
| **Fabricio Nick** | `202520045` | `@SRHOUSE` |
| **André Brando** | `202520185` | `@andremejia-hub` |
| **Jared Chala** | `202520040` | `@jaredchala-bot` |
| **Iker García** | `202520059` | `@iygt8-iterate` |

El detalle verificable de responsabilidades y aportes se mantiene en [`docs/contributions.md`](docs/contributions.md).

---

## 2. Descripción general

**Circuito de Escape** es un juego de navegación por turnos sobre una cuadrícula rectangular. El agente debe alcanzar la salida antes de quedarse sin energía o superar el límite máximo de turnos.

Durante el recorrido puede encontrar:

- espacios libres;
- muros;
- terreno de costo elevado;
- recursos;
- baterías;
- trampas;
- una salida.

El proyecto se concentra en el **motor de simulación**. La presentación se mantiene separada del estado y de las reglas para que el mismo `NavigationEnvironment` pueda utilizarse tanto en una partida humana como en simulaciones automáticas reproducibles.

---

## 3. Características principales

- tablero genérico `Grid<Cell, Rows, Columns>`;
- escenarios principales de **20 × 30**;
- siete tipos de celda;
- perfiles **Easy**, **Standard** y **Hard**;
- renderizado **Emoji** y **ASCII**;
- interfaz interactiva con FTXUI;
- estado observable mediante `Observation`;
- resultado de cada acción mediante `StepResult`;
- eventos tipados con `std::variant`;
- controlador aleatorio reproducible;
- controlador heurístico basado en distancia Manhattan;
- `concept NavigationPolicy`;
- adaptador genérico `PolicyController<Policy>`;
- polimorfismo dinámico mediante `IController`;
- controladores polimórficos administrados mediante `std::unique_ptr<IController>`;
- simulación automática reproducible con semilla controlable;
- pruebas automáticas integradas con CMake y CTest.

---

## 4. Reglas de la partida

El agente mantiene, como mínimo:

- posición actual;
- energía actual y máxima;
- puntaje acumulado;
- cantidad de recursos recogidos;
- estado activo o finalizado.

### Condiciones de término

Una partida puede terminar por:

```cpp
enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};
```

La precedencia utilizada es:

1. `goalReached`, cuando el agente se encuentra en la salida y conserva energía;
2. `noEnergy`;
3. `turnLimit`.

Por tanto:

- llegar a la salida con energía positiva produce victoria;
- llegar a la salida con energía cero no produce victoria;
- llegar con energía positiva exactamente en el último turno sí produce victoria.

El estado final conserva la información necesaria para conocer el resultado de la partida, incluyendo turnos, energía restante, recursos recogidos y puntaje.

### Resolución de una acción

`NavigationEnvironment::step(Action)` concentra la lógica de cada turno.

El orden general es:

1. incrementar el turno;
2. procesar una acción inválida o `wait`;
3. calcular la posición candidata para un movimiento;
4. rechazar movimientos fuera del tablero o contra un muro;
5. actualizar la posición cuando el movimiento es válido;
6. descontar el costo de entrada;
7. aplicar el efecto de la celda destino;
8. generar los eventos correspondientes;
9. comprobar las condiciones de término;
10. devolver un `StepResult`.

Un movimiento hacia un muro o fuera del tablero:

- consume un turno;
- no cambia la posición;
- descuenta `waitOrInvalidCost`;
- genera `MovementRejectedEvent`.

La acción `wait`:

- consume un turno;
- no cambia la posición;
- descuenta `waitOrInvalidCost`;
- no vuelve a activar la celda actual.

---

## 5. Tipos de celda

Las celdas se representan mediante `std::variant`:

```cpp
using Cell = std::variant<
    Empty,
    Wall,
    RoughTerrain,
    ResourceCell<int>,
    Battery,
    Trap,
    Exit
>;
```

| Celda | Emoji | ASCII | Comportamiento |
|---|:---:|:---:|---|
| Agente | 🤖 | `@` | posición actual del jugador |
| Espacio libre | ⬜ | `.` | movimiento normal |
| Muro | ⬛ | `#` | bloquea el movimiento |
| Terreno elevado | 🟫 | `~` | consume energía adicional |
| Recurso | 💎 | `R` | suma puntaje una sola vez |
| Batería | ⚡ | `B` | recupera energía una sola vez |
| Trampa | 💥 | `T` | penaliza energía y puntaje en cada entrada |
| Salida | 🏁 | `S` | objetivo del escenario |

Los recursos y las baterías son consumibles. Después de activarse no vuelven a aplicar su beneficio. Las trampas no son consumibles y vuelven a aplicar su penalización cada vez que el agente entra en ellas.

---

## 6. Perfiles de dificultad

Las reglas variables se centralizan en `GameRules` y se obtienen mediante:

```cpp
GameRules rulesFor(Difficulty difficulty);
```

| Parámetro | Easy | Standard | Hard |
|---|---:|---:|---:|
| Energía inicial | 80 | 60 | 40 |
| Energía máxima | 80 | 60 | 40 |
| Límite de turnos | 240 | 180 | 140 |
| Costo de celda normal | 1 | 1 | 1 |
| Costo de terreno elevado | 2 | 2 | 3 |
| Costo de `wait` o intento inválido | 1 | 1 | 1 |
| Puntos por recurso | +15 | +10 | +8 |
| Recarga de batería | +5 | +3 | +2 |
| Penalización de energía por trampa | -1 | -2 | -3 |
| Penalización de puntaje por trampa | 0 | -1 | -2 |

El perfil predeterminado es **Standard**.

`NavigationEnvironment` recibe un objeto `GameRules`, por lo que las diferencias entre dificultades se mantienen como datos configurables en lugar de condicionales dispersos por el motor.

---

## 7. Interfaz de consola

La capa de presentación utiliza **FTXUI v7.0.3**.

### Menú principal

Desde el menú principal se puede:

- iniciar la partida;
- seleccionar escenario;
- seleccionar dificultad;
- seleccionar modo de renderizado;
- abrir la ayuda;
- salir.

La navegación de menús utiliza `W` / `S` o las flechas verticales y `Enter` para seleccionar.

### Controles durante la partida

| Tecla | Alternativa | Acción |
|---|---|---|
| `W` | `↑` | mover arriba |
| `S` | `↓` | mover abajo |
| `A` | `←` | mover izquierda |
| `D` | `→` | mover derecha |
| `E` | — | esperar un turno |
| `H` | — | abrir/cerrar ayuda |
| `Q` | — | salir |

Las letras se aceptan en mayúsculas y minúsculas.

Una entrada no reconocida muestra un aviso y vuelve a esperar una entrada válida **sin ejecutar `step()`**.

### Modos de renderizado

El menú ofrece:

- **Emoji**, modo predeterminado;
- **ASCII**, como alternativa para terminales que no representen correctamente Unicode o emojis de ancho completo.

Durante la partida se mantienen visibles el tablero y el estado relevante: turno, energía, puntaje, recursos y el último resultado o evento.

---

## 8. Escenarios

El proyecto incluye dos escenarios principales de **20 × 30**:

- **Scenario 1**;
- **Scenario 2**.

Sus implementaciones se encuentran en:

```text
src/scenarios/scenario_1.cpp
src/scenarios/scenario_2.cpp
```

con interfaces en:

```text
include/circuit_escape/scenarios/scenario_1.h
include/circuit_escape/scenarios/scenario_2.h
```

Ambos escenarios cuentan con una ruta alcanzable desde la posición inicial hasta la salida y se validan mediante pruebas.

La posición inicial utilizada por `GameApplication` es:

```cpp
Position{1, 1}
```

La generación procedural compleja de mapas no forma parte del alcance del proyecto.

---

## 9. Arquitectura

El diseño separa la presentación, la coordinación de la aplicación y el motor:

```text
┌──────────────────────────┐
│        ConsoleUI         │
│  FTXUI + input + render  │
└────────────┬─────────────┘
             │ UICommand
             ▼
┌──────────────────────────┐
│     GameApplication      │
│   coordina la partida    │
└────────────┬─────────────┘
             │ Action
             ▼
┌──────────────────────────┐
│ NavigationEnvironment    │
│ estado + reglas + step() │
└──────────┬───────┬───────┘
           │       │
           ▼       ▼
  Grid<Cell,R,C>   NavigationEvent
           │
           ▼
      Observation
           │
           ▼
     IController
           ▲
           │
 PolicyController<Policy>
      /            \
RandomPolicy   HeuristicPolicy
```

### Responsabilidades principales

| Componente | Responsabilidad |
|---|---|
| `ConsoleUI` | traducir eventos del teclado y renderizar con FTXUI |
| `GameApplication` | coordinar menús, partida, animaciones y llamadas a `step()` |
| `NavigationEnvironment` | mantener el estado, validar y ejecutar acciones, aplicar reglas y producir resultados |
| `Grid<Cell, Rows, Columns>` | almacenar celdas, validar posiciones y exponer iteradores |
| `IController` | interfaz virtual común para controladores automáticos |
| `PolicyController<Policy>` | adaptar una policy genérica a `IController` |
| `RandomPolicy` | seleccionar una acción legal con aleatoriedad reproducible |
| `HeuristicPolicy` | escoger una acción legal usando distancia Manhattan |

Para una partida humana no se necesita una clase `HumanController`: `ConsoleUI` traduce la entrada del jugador y `GameApplication` entrega la acción directamente a `NavigationEnvironment::step()`.

---

## 10. Estado observable y resultado de un paso

### `Observation`

Los controladores reciben una copia del estado observable, sin referencias modificables al estado interno:

```cpp
struct Observation {
    Position agent;
    Position goal;
    int energy{};
    int maximumEnergy{};
    int score{};
    std::size_t collectedResources{};
    std::size_t turn{};
    std::size_t turnLimit{};
    std::vector<Action> availableActions;
};
```

### `StepResult`

Cada acción produce información equivalente a:

```cpp
struct StepResult {
    Observation observation;
    std::vector<NavigationEvent> events;
    bool finished{false};
    EndReason reason{EndReason::none};
};
```

Así, la UI y los controladores trabajan con datos del motor sin interpretar texto de consola.

---

## 11. Sistema de eventos

Los eventos se modelan mediante tipos pequeños y `std::variant`:

```cpp
using NavigationEvent = std::variant<
    MovedEvent,
    MovementRejectedEvent,
    ResourceCollectedEvent,
    EnergyChangedEvent,
    TrapTriggeredEvent,
    GoalReachedEvent
>;
```

Entre los eventos se encuentran:

- movimiento realizado;
- movimiento rechazado;
- recurso recogido;
- cambio de energía;
- trampa activada;
- llegada a la salida.

La interfaz procesa estos valores con `std::visit`; el motor no imprime directamente en consola.

---

## 12. Controladores y policies

### `IController`

`IController` define el contrato común para decisiones automáticas:

```cpp
class IController {
public:
    virtual ~IController() = default;

    virtual Action selectAction(
        const Observation& observation,
        std::span<const Action> legalActions
    ) = 0;
};
```

Los controladores polimórficos se administran mediante `std::unique_ptr<IController>`, lo que permite intercambiar implementaciones a través de la misma interfaz sin almacenar objetos polimórficos por valor.

### `NavigationPolicy`

El contrato de las policies se valida en compilación:

```cpp
template<typename Policy>
concept NavigationPolicy = requires(
    Policy& policy,
    const Observation& observation,
    std::span<const Action> actions
) {
    { policy.selectAction(observation, actions) } -> std::same_as<Action>;
};
```

### `PolicyController<Policy>`

```cpp
template<typename Policy>
requires NavigationPolicy<Policy>
class PolicyController : public IController {
    // ...
};
```

Este adaptador combina:

- validación estática mediante `NavigationPolicy`;
- implementación genérica mediante templates;
- despacho dinámico mediante `IController`.

### `RandomPolicy`

- utiliza `std::mt19937`;
- recibe una semilla controlable;
- selecciona una acción legal;
- permite reproducir secuencias de decisiones en las pruebas.

### `HeuristicPolicy`

Selecciona la acción legal que reduce la distancia Manhattan respecto de la salida:

```text
|row1 - row2| + |column1 - column2|
```

Es una estrategia heurística local: no pretende encontrar siempre una ruta global óptima.

### Intercambio de controladores

La simulación utiliza la interfaz común `IController`, de modo que un controlador basado en `RandomPolicy` puede sustituirse por otro basado en `HeuristicPolicy` sin modificar `NavigationEnvironment`.

---

## 13. Simulación automática reproducible

El proyecto permite ejecutar una simulación sin interacción humana.

La API existente incluye:

```cpp
template<std::size_t Rows, std::size_t Columns>
SimulationResult runRandomSimulation(
    NavigationEnvironment<Rows, Columns>& environment,
    std::uint32_t seed
);
```

La simulación:

1. reinicia el entorno;
2. utiliza una policy con semilla controlable;
3. obtiene `Observation` y acciones legales;
4. selecciona una acción;
5. ejecuta `step()`;
6. repite hasta finalizar;
7. conserva acciones, motivo de término, turnos, puntaje y energía restante.

Las pruebas utilizan semillas fijas para verificar reproducibilidad.

---

## 14. Aplicación de C++20 y temas del curso

La ubicación detallada de cada decisión se documenta en [`docs/design.md`](docs/design.md).

### Templates de funciones

`include/circuit_escape/generic_functions.h` contiene algoritmos genéricos basados en iteradores, entre ellos operaciones equivalentes a conteo y búsqueda sobre rangos.

Estos algoritmos se utilizan con más de un tipo de contenedor y también se prueban con rangos vacíos.

La simulación automática añade otro uso de templates parametrizado por las dimensiones del entorno.

### Template de clase

```cpp
template<typename CellType, std::size_t Rows, std::size_t Columns>
class Grid;
```

`Grid` utiliza:

- un parámetro de tipo;
- dos parámetros no-tipo;
- almacenamiento mediante `std::array<CellType, Rows * Columns>`;
- acceso validado;
- iteradores const y no const;
- comprobación de dimensiones válidas en compilación.

### Especialización total y parcial

`CellTraits` expresa diferencias reales del dominio.

Especialización total para `Wall`:

```cpp
template<>
struct CellTraits<Wall>;
```

Especialización parcial para recursos:

```cpp
template<typename Reward>
struct CellTraits<ResourceCell<Reward>>;
```

### Template variádico

```cpp
template<class... FLambda>
struct Overloaded : FLambda... {
    using FLambda::operator()...;
};
```

`Overloaded` permite agrupar lambdas utilizadas por `std::visit`.

### Fold expression

Las condiciones de término utilizan una fold expression dentro de la lógica que evalúa la primera condición satisfecha:

```cpp
(evaluate(conditions), ...);
```

### Concepts y polimorfismo dinámico

- `NavigationPolicy` comprueba el contrato de una policy en compilación;
- `PolicyController<Policy>` adapta una policy válida;
- `IController` proporciona despacho dinámico en ejecución.

La evidencia de una compilación intencionalmente inválida para una policy que no satisface `NavigationPolicy` se documenta en `docs/negative_concept_test.md`. Ese ejemplo se mantiene fuera del build normal.

---

## 15. Biblioteca estándar y elección de contenedores

| Elemento | Uso principal |
|---|---|
| `std::array` | almacenamiento contiguo y de tamaño fijo del grid |
| `std::vector` | acciones, eventos y resultados de simulación |
| `std::variant` | representación de celdas y eventos |
| `std::visit` | procesamiento de alternativas de un `variant` |
| `std::optional` | resultados que pueden no existir, como posiciones o comandos válidos |
| `std::span` | vistas no propietarias sobre acciones o eventos |
| `std::unique_ptr` | propiedad de controladores polimórficos |
| `std::mt19937` | decisiones pseudoaleatorias reproducibles |
| `std::uniform_int_distribution` | selección aleatoria de acciones |
| algoritmos de `<algorithm>` | operaciones genéricas y pruebas |

### `std::array`

El tamaño del grid se conoce en compilación:

```cpp
Grid<Cell, 20, 30>
```

`std::array` ofrece almacenamiento contiguo, no requiere redimensionamiento durante la ejecución y permite exponer iteradores directamente.

### `std::vector`

Se utiliza cuando el número de elementos depende de la ejecución, como en:

- acciones disponibles;
- eventos de un turno;
- secuencias producidas por una simulación.

### `std::variant`

Los posibles tipos de celda y evento se conocen en compilación. `std::variant` permite representarlos de forma segura sin crear una jerarquía polimórfica separada para cada caso.

### `std::span`

Permite entregar una vista ligera de una secuencia contigua sin copiar el contenedor ni transferir su propiedad.

### `std::unique_ptr`

Se utiliza para expresar propiedad exclusiva de controladores polimórficos y permitir su intercambio mediante `IController` sin recurrir a `new` o `delete` directamente.

---

## 16. Estructura del repositorio

```text
Maze/
├── app/
│   └── main.cpp
├── assets/
│   └── maps/
├── docs/
│   ├── design.md
│   ├── contributions.md
│   └── negative_concept_test.md
├── include/
│   └── circuit_escape/
│       ├── agent.h
│       ├── cells.h
│       ├── console_ui.h
│       ├── controllers.h
│       ├── environment.h
│       ├── game_rules.h
│       ├── GameApplication.h
│       ├── generic_functions.h
│       ├── grid.h
│       ├── menus.h
│       ├── simulation.h
│       ├── types.h
│       └── scenarios/
│           ├── scenario_1.h
│           └── scenario_2.h
├── src/
│   ├── agent.cpp
│   ├── console_ui.cpp
│   ├── controllers.cpp
│   ├── environment.cpp
│   ├── game_rules.cpp
│   ├── GameApplication.cpp
│   ├── menus.cpp
│   ├── types.cpp
│   └── scenarios/
│       ├── scenario_1.cpp
│       └── scenario_2.cpp
├── tests/
├── .gitignore
├── CMakeLists.txt
└── README.md
```

Los archivos de animaciones y arte de la interfaz se mantienen junto a la capa de presentación.

---

## 17. Requisitos y dependencias

Se necesita:

- compilador compatible con **C++20**;
- **CMake 3.20** o superior;
- **Git**;
- una terminal moderna;
- conexión a Internet durante la primera configuración de CMake para descargar FTXUI mediante `FetchContent`.

Compiladores habituales:

- GCC;
- Clang / Apple Clang;
- MSVC.

FTXUI está fijado en la versión **v7.0.3**.

---

## 18. Compilación y ejecución

### Linux

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel

./build/Maze
```

### macOS

Con Xcode Command Line Tools y CMake instalados:

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel

./build/Maze
```

### Windows — PowerShell

```powershell
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --config Release
```

Con un generador multi-config, el ejecutable normalmente estará en:

```powershell
.\build\Release\Maze.exe
```

### Windows — Command Prompt

```cmd
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --config Release
```

Con Visual Studio como generador:

```cmd
build\Release\Maze.exe
```

> La ubicación exacta del ejecutable en Windows puede variar según el generador de CMake utilizado.

---

## 19. Pruebas automáticas

CMake define el ejecutable de pruebas `Maze_test` y lo registra mediante CTest.

### Linux / macOS

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Windows

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Las pruebas cubren, entre otros aspectos:

- acceso válido e inválido a `Grid`;
- bordes y esquinas;
- movimiento libre;
- movimiento contra obstáculos;
- movimiento fuera del tablero;
- costos de movimiento, terreno elevado, `wait` e intentos inválidos;
- recursos de un solo uso;
- baterías consumibles y límite de recarga;
- trampas repetibles;
- perfiles `easy`, `standard` y `hard`;
- precedencia entre condiciones de término;
- acciones disponibles;
- `RandomPolicy`;
- `HeuristicPolicy`;
- `NavigationPolicy`;
- `PolicyController` a través de `IController`;
- simulaciones reproducibles con la misma semilla;
- procesamiento de eventos;
- algoritmos genéricos con distintos contenedores y rangos vacíos;
- traducción de comandos válidos;
- renderizado Emoji y ASCII;
- dimensiones 20 × 30 del renderizado;
- solucionabilidad de los escenarios de demostración.

La lógica del entorno puede probarse sin leer desde `std::cin` ni escribir en `std::cout`.

---

## 20. Ejemplos de uso

### Partida interactiva

1. ejecutar `Maze`;
2. seleccionar **Scenario 1** o **Scenario 2**;
3. seleccionar **Easy**, **Standard** o **Hard**;
4. elegir **Emoji** o **ASCII**;
5. seleccionar **Start Game**;
6. desplazarse con `WASD` o las flechas;
7. usar `E` para esperar;
8. consultar la ayuda con `H`;
9. alcanzar la salida antes de agotar la energía o el límite de turnos.

### Simulación reproducible

Una simulación aleatoria puede ejecutarse sin interfaz usando una semilla explícita:

```cpp
auto result = runRandomSimulation(environment, 2026);
```

Con la misma configuración inicial y la misma semilla, la secuencia pseudoaleatoria puede reproducirse en pruebas.

La arquitectura permite utilizar el mismo entorno con una estrategia aleatoria o heurística a través de `IController`.

---

## 21. Documentación complementaria

La documentación técnica de la entrega se encuentra en `docs/`:

- [`docs/design.md`](docs/design.md): decisiones de diseño y ubicación de los temas obligatorios del curso;
- [`docs/contributions.md`](docs/contributions.md): responsabilidades y contribuciones verificables de cada integrante;
- [`docs/negative_concept_test.md`](docs/negative_concept_test.md): evidencia del diagnóstico producido al intentar instanciar un controlador con una policy que no satisface `NavigationPolicy`.

El README concentra la información necesaria para compilar, ejecutar, probar y entender el proyecto; `docs/design.md` contiene el detalle técnico adicional.

---

## 22. Alcance y exclusiones

No forman parte del alcance de esta etapa:

- interfaz gráfica;
- juego en red;
- persistencia en base de datos;
- dimensiones de tablero elegidas durante la ejecución;
- movimiento en tiempo real;
- enemigos móviles o múltiples agentes;
- visión parcial compleja;
- generación procedural compleja;
- A*, Dijkstra u otros algoritmos avanzados de búsqueda;
- reinforcement learning.

La arquitectura mantiene el motor separado del controlador para facilitar futuras extensiones sin acoplarlas a la interfaz.

---

## 23. Verificación previa a la entrega

Antes de crear el tag final, la versión evaluada debe comprobarse desde un clon limpio siguiendo únicamente las instrucciones de este README:

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git Maze-verificacion
cd Maze-verificacion

cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/Maze
```

La entrega se identifica mediante el tag anotado:

```text
proyecto-1-entrega
```

El hash completo del commit etiquetado debe coincidir con el registrado en el medio de entrega del curso.

---

## 24. Resumen rápido

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure

./build/Maze
```

**Circuito de Escape** integra un motor de navegación por turnos desacoplado de la interfaz, reglas configurables, templates, especializaciones, concepts, eventos tipados, polimorfismo dinámico, controladores intercambiables y simulaciones reproducibles dentro de una aplicación de consola construida con C++20.