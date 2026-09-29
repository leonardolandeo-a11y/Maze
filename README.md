# Circuito de Escape — Maze

Motor de navegación por turnos desarrollado en **C++20** para el curso **CS2013 — Programación III**.  
El proyecto utiliza **CMake**, **FTXUI** para la presentación en consola y una arquitectura que separa el motor de simulación de la entrada/salida.

> Repositorio: https://github.com/leonardolandeo-a11y/Maze

---

## 1. Información de la entrega

| Campo | Información |
|---|---|
| Curso | CS2013 — Programación III |
| Proyecto | Proyecto 1 — Circuito de Escape |
| Semestre | 2026-2 |
| Grupo | 5 |
| Repositorio | https://github.com/leonardolandeo-a11y/Maze |
| Rama evaluada | `main` |
| Tag de entrega | `proyecto-1-entrega` |
| Commit evaluado | **[COMPLETAR: hash completo del commit]** |

### Integrantes


| Integrante | Código UTEC | GitHub | Responsabilidad principal |
|---|---|---|---|
| **[Nombre 1]** | **[202520059]** | **[@leonardolandeo-a11y]** | **[Agent, grid ,celdas, game_rules,types , Environment, GameApplication y Controllers]** |
| **[Nombre 2]** | **[202520151]** | **[@FabricioNick]** | **[StartupAnimation, Environment, game_rules , Death Animation, Layout y VictoryAnimation]** |
| **[Nombre 3]** | **[202520185]** | **[@andremejia-hub]** | **[Todos los escenarios,tests , game_rules y Simulation]** |
| **[Nombre 4]** | **[202520040]** | **[@jaredchala-bot]** | **[Escenario, Arreglo de errores, Environment, console_ui y game_rules]** |
| **[Nombre 5]** | **[202520045]** | **[@iygt8-iterate]** | **[Creacion de menu, dificultades y implementacion de controller]** |

---

## 2. Descripción general

**Circuito de Escape** es un juego de navegación por turnos sobre una cuadrícula rectangular.

El jugador controla un agente que debe:

- desplazarse por el tablero;
- evitar muros;
- administrar su energía;
- atravesar terreno de costo elevado;
- recoger recursos;
- utilizar baterías;
- evitar o asumir el costo de trampas;
- alcanzar la salida antes de quedarse sin energía o superar el límite de turnos.

El proyecto está centrado en el **motor de simulación**. La interfaz de consola es una capa separada que únicamente traduce entradas del usuario y representa el estado producido por el motor.

El mismo `NavigationEnvironment` puede utilizarse sin consola mediante controladores automáticos y simulaciones reproducibles.

---

## 3. Objetivos del proyecto

El proyecto busca aplicar de forma integrada conceptos de C++ moderno:

- tipos abstractos e invariantes;
- RAII y semántica de valores;
- templates de funciones;
- templates de clases;
- parámetros de tipo y parámetros no-tipo;
- especialización total y parcial;
- templates variádicos;
- fold expressions;
- `concepts` de C++20;
- polimorfismo dinámico;
- iteradores;
- algoritmos genéricos;
- contenedores de la biblioteca estándar;
- `std::variant` y `std::visit`;
- `std::optional`;
- `std::span`;
- generación pseudoaleatoria reproducible;
- separación entre estado, reglas, controladores e interfaz;
- pruebas automáticas sin depender de entrada/salida por consola.

---

## 4. Alcance funcional

Una partida se ejecuta sobre un tablero de **20 × 30** en los escenarios principales.

El agente posee:

- posición actual;
- energía actual;
- energía máxima;
- puntaje;
- cantidad de recursos recogidos;
- estado activo/finalizado.

### Condiciones de término

El motor utiliza:

```cpp
enum class EndReason {
    none,
    goalReached,
    noEnergy,
    turnLimit
};
```

Una partida termina cuando:

1. el agente alcanza la salida manteniendo energía positiva;
2. la energía llega a cero;
3. se alcanza el límite de turnos.

La precedencia aplicada por el motor es:

```text
goalReached
    ↓
noEnergy
    ↓
turnLimit
```

Por ello:

- llegar a la salida con energía positiva produce victoria;
- llegar a la salida con energía cero **no** produce victoria;
- llegar con energía positiva exactamente en el último turno sí produce victoria.

---

## 5. Tipos de celda

Las celdas se representan mediante un `std::variant`:

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
| Agente | 🤖 | `@` | Posición actual del jugador |
| Espacio libre | ⬜ | `.` | Movimiento normal |
| Muro | ⬛ | `#` | Bloquea el movimiento |
| Terreno elevado | 🟫 | `~` | Consume energía adicional |
| Recurso | 💎 | `R` | Suma puntaje una sola vez |
| Batería | ⚡ | `B` | Recupera energía una sola vez |
| Trampa | 💥 | `T` | Penaliza energía y puntaje en cada entrada |
| Salida | 🏁 | `S` | Objetivo del escenario |

### Consumibles

Los recursos y las baterías son consumibles:

- `ResourceCell` mantiene `collected`;
- `Battery` mantiene `consumed`;
- después de activarse, visualmente se comportan como espacio vacío;
- una segunda entrada no vuelve a aplicar su beneficio.

Las trampas **no** son consumibles y vuelven a aplicar su penalización cada vez que el agente entra en ellas.

---

## 6. Reglas y dificultades

Las reglas variables se centralizan en:

```cpp
struct GameRules {
    int initialEnergy;
    int maximumEnergy;
    std::size_t turnLimit;

    int normalCellCost;
    int roughTerrainCost;
    int waitOrInvalidCost;

    int resourcePoints;
    int batteryRecharge;
    int trapEnergyPenalty;
    int trapScorePenalty;
};
```

La función:

```cpp
GameRules rulesFor(Difficulty difficulty);
```

produce uno de los tres perfiles disponibles.

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

El perfil predeterminado de la aplicación es **Standard**.

La ventaja de este diseño es que `NavigationEnvironment` aplica un objeto `GameRules` y no necesita contener condicionales dispersos según el nombre de la dificultad.

---

## 7. Orden de resolución de un turno

`NavigationEnvironment::step(Action)` concentra la lógica de una acción.

El orden utilizado es:

1. incrementar el turno;
2. procesar `wait` o una acción inválida;
3. para un movimiento válido, calcular la celda destino;
4. actualizar la posición;
5. registrar `MovedEvent`;
6. descontar el costo de entrada;
7. registrar el cambio de energía;
8. aplicar el efecto de la celda destino;
9. producir los eventos correspondientes;
10. comprobar las condiciones de término;
11. devolver un `StepResult`.

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

## 8. Arquitectura

El diseño separa el dominio de la presentación:

```text
┌──────────────────────────┐
│        ConsoleUI         │
│ FTXUI + teclado + render │
└────────────┬─────────────┘
             │ UICommand
             ▼
┌──────────────────────────┐
│     GameApplication      │
│ coordina el ciclo        │
└────────────┬─────────────┘
             │ Action
             ▼
┌──────────────────────────┐
│ NavigationEnvironment    │
│ estado + reglas + step() │
└──────────┬───────┬───────┘
           │       │
           │       └──────────────┐
           ▼                      ▼
┌──────────────────┐      ┌───────────────────┐
│ Grid<Cell,R,C>   │      │ NavigationEvent   │
│ tablero genérico │      │ eventos del turno │
└──────────────────┘      └───────────────────┘

             Observation
                  │
                  ▼
        ┌─────────────────┐
        │   IController   │
        │ Random/Heuristic│
        └─────────────────┘
```

### Responsabilidades

| Componente | Responsabilidad | No debe hacer |
|---|---|---|
| `ConsoleUI` | Traducir teclas y renderizar con FTXUI | Aplicar reglas de energía o modificar el tablero |
| `GameApplication` | Coordinar menús, partida, animaciones y llamadas a `step()` | Duplicar reglas del entorno |
| `NavigationEnvironment` | Mantener y modificar el estado; validar y ejecutar acciones | Leer teclado o depender de FTXUI |
| `Grid<Cell, Rows, Columns>` | Almacenar celdas, validar límites y exponer iteradores | Conocer energía, turnos o victoria |
| `IController` | Contrato virtual para elegir acciones | Modificar directamente el entorno |
| `PolicyController<Policy>` | Adaptar una policy genérica a `IController` | Decidir mediante `typeid` o `dynamic_cast` |

---

## 9. Flujo de ejecución

El `main` es deliberadamente pequeño:

```cpp
int main() {
    GameApplication game;
    game.Run();
    return 0;
}
```

Flujo general:

```text
main()
  │
  ▼
GameApplication::Run()
  │
  ├── StartupAnimation
  │
  ├── Menus
  │     ├── Scenario
  │     ├── Difficulty
  │     └── RenderMode
  │
  ├── CreateScenario()
  ├── rulesFor(...)
  ├── CreatePlayer(...)
  │
  ▼
NavigationEnvironment<20,30>
  │
  ▼
ConsoleUI / FTXUI
  │
  ├── W/A/S/D o flechas
  ├── E → wait
  ├── H → ayuda
  └── Q → salir
  │
  ▼
environment.step(Action)
  │
  ├── Observation
  ├── NavigationEvent[]
  ├── finished
  └── EndReason
```

---

## 10. Estado observable y resultado de un paso

### `Observation`

Los controladores reciben una copia del estado observable:

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

Esto evita entregar referencias modificables al estado interno.

### `StepResult`

Cada acción retorna:

```cpp
struct StepResult {
    Observation observation;
    std::vector<NavigationEvent> events;
    bool finished{false};
    EndReason reason{EndReason::none};
};
```

Así, la interfaz puede conocer el nuevo estado y lo ocurrido durante el turno sin inspeccionar texto de consola.

---

## 11. Sistema de eventos

Los eventos se modelan mediante tipos pequeños:

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

| Evento | Significado |
|---|---|
| `MovedEvent` | El movimiento fue aceptado |
| `MovementRejectedEvent` | El movimiento fue rechazado |
| `ResourceCollectedEvent` | Se recogió un recurso |
| `EnergyChangedEvent` | La energía cambió |
| `TrapTriggeredEvent` | Se activó una trampa |
| `GoalReachedEvent` | Se alcanzó la salida |

La UI procesa estos eventos con `std::visit`.

Esto mantiene la separación:

```text
motor
  │
  │ datos tipados
  ▼
NavigationEvent
  │
  ▼
interfaz
```

El motor no imprime mensajes directamente.

---

## 12. Controladores y policies

### `IController`

El polimorfismo dinámico se implementa mediante:

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

### `RandomPolicy`

- utiliza `std::mt19937`;
- recibe una semilla controlable;
- selecciona únicamente una acción legal;
- dos ejecuciones equivalentes con la misma semilla producen la misma secuencia pseudoaleatoria.

### `HeuristicPolicy`

Selecciona la acción legal que reduce la **distancia Manhattan** respecto de la salida:

```text
|row1 - row2| + |column1 - column2|
```

No pretende encontrar un camino global óptimo; es una heurística local simple.

### `PolicyController`

```cpp
template<typename Policy>
requires NavigationPolicy<Policy>
class PolicyController : public IController {
    ...
};
```

Funciona como adaptador entre policies genéricas y la interfaz polimórfica.

---

## 13. Simulación automática reproducible

El archivo:

```text
include/circuit_escape/simulation.h
```

define:

```cpp
template<std::size_t Rows, std::size_t Columns>
SimulationResult runRandomSimulation(
    NavigationEnvironment<Rows, Columns>& environment,
    std::uint32_t seed
);
```

La simulación:

1. reinicia el entorno;
2. crea una `RandomPolicy` con la semilla recibida;
3. obtiene el estado y las acciones legales;
4. selecciona una acción;
5. llama a `step()`;
6. repite hasta terminar;
7. devuelve acciones, motivo de término, turnos, score y energía restante.

Los tests utilizan semillas fijas para verificar reproducibilidad.

---

# 14. Aplicación de los temas obligatorios de C++20

Esta sección indica **dónde se encuentra cada requisito**.

## 14.1 Templates de funciones

### `countMatching`

Archivo:

```text
include/circuit_escape/generic_functions.h
```

```cpp
template<typename Iterator, typename Predicado>
std::size_t countMatching(
    Iterator first,
    Iterator last,
    Predicado predicado
);
```

Recorre un rango mediante iteradores y cuenta los elementos que satisfacen un predicado.

Se utiliza, por ejemplo, para verificar que el entorno tenga exactamente una salida.

### `LinearSearch`

Archivo:

```text
include/circuit_escape/generic_functions.h
```

```cpp
template<typename Iterator, typename Predicado>
Iterator LinearSearch(
    Iterator first,
    Iterator last,
    Predicado predicado
);
```

Busca el primer elemento que satisface un predicado.

Se utiliza para localizar la salida dentro del grid.

### `runRandomSimulation`

Archivo:

```text
include/circuit_escape/simulation.h
```

Es un template de función parametrizado por `Rows` y `Columns`.

### Uso con distintos contenedores

Los algoritmos genéricos se prueban con:

- `std::vector`;
- `std::array`;
- rangos vacíos.

Archivo:

```text
tests/generic_functions_test.cpp
```

---

## 14.2 Templates de clases

### `Grid`

```cpp
template<
    typename CellType,
    std::size_t Rows,
    std::size_t Columns
>
class Grid;
```

Archivo:

```text
include/circuit_escape/grid.h
```

Utiliza:

- un parámetro de tipo;
- dos parámetros no-tipo;
- `std::array<CellType, Rows * Columns>`;
- iteradores const y no const;
- `static_assert` para impedir dimensiones cero.

### Otros templates de clase

También aparecen:

```cpp
ResourceCell<Reward>
NavigationEnvironment<Rows, Columns>
PolicyController<Policy>
CellTraits<CellType>
```

---

## 14.3 Especialización total y parcial

Archivo:

```text
include/circuit_escape/cells.h
```

Caso general:

```cpp
template<typename CellType>
struct CellTraits;
```

### Especialización total

```cpp
template<>
struct CellTraits<Wall>;
```

El muro no es transitable.

### Especialización parcial

```cpp
template<typename Reward>
struct CellTraits<ResourceCell<Reward>>;
```

Reconoce cualquier `ResourceCell<Reward>` independientemente del tipo de recompensa.

---

## 14.4 Templates variádicos

Archivo:

```text
include/circuit_escape/environment.h
```

```cpp
template<class... FLambda>
struct Overloaded : FLambda... {
    using FLambda::operator()...;
};
```

El parameter pack `FLambda...` permite agrupar distintas lambdas y utilizarlas como visitor para `std::visit`.

---

## 14.5 Fold expression

Archivo:

```text
include/circuit_escape/environment.h
```

```cpp
(evaluate(conditions), ...);
```

Se utiliza dentro de:

```cpp
firstSatisfiedTermination(...)
```

para evaluar condiciones de término en un orden definido.

---

## 14.6 Concepts

Archivo:

```text
include/circuit_escape/controllers.h
```

```cpp
template<typename Policy>
concept NavigationPolicy = requires(...) {
    {
        policy.selectAction(observation, actions)
    } -> std::same_as<Action>;
};
```

El concept comprueba en compilación que una policy posea la operación correcta y retorne exactamente `Action`.

---

## 14.7 Polimorfismo dinámico

Archivo:

```text
include/circuit_escape/controllers.h
```

```cpp
class IController
```

define la interfaz virtual común.

`PolicyController<Policy>` implementa esa interfaz y delega la decisión a la policy almacenada.

De esta forma:

- el `concept` valida la policy en compilación;
- `IController` permite despacho dinámico en ejecución.

---

## 14.8 `std::variant` y `std::visit`

Se utilizan para:

### Celdas

```cpp
using Cell = std::variant<...>;
```

Archivo:

```text
include/circuit_escape/cells.h
```

### Eventos

```cpp
using NavigationEvent = std::variant<...>;
```

Archivo:

```text
include/circuit_escape/environment.h
```

### Procesamiento

`std::visit` aparece en:

- `isTraversable`;
- `movementCost`;
- `applyEffectCell`;
- `ConsoleUI::RenderCell`;
- `ConsoleUI::EventMessage`.

---

## 14.9 `std::optional`

Archivo:

```text
include/circuit_escape/types.h
src/types.cpp
```

```cpp
std::optional<Position> neighbor(...);
```

Evita representar posiciones negativas con valores inválidos.

---

## 14.10 Iteradores

`Grid` expone:

```cpp
begin()
end()
cbegin()
cend()
```

Los algoritmos genéricos trabajan sobre iteradores en lugar de depender de un contenedor concreto.

---

## 14.11 Biblioteca estándar utilizada

| Elemento | Uso |
|---|---|
| `std::array` | almacenamiento contiguo y de tamaño fijo del grid |
| `std::vector` | eventos, acciones disponibles y resultados de simulación |
| `std::variant` | celdas y eventos |
| `std::visit` | despacho según alternativa activa del `variant` |
| `std::optional` | posición vecina potencialmente inexistente |
| `std::span` | vista no propietaria de acciones y eventos |
| `std::mt19937` | decisiones pseudoaleatorias reproducibles |
| `std::uniform_int_distribution` | selección de acción aleatoria |
| `std::thread` | actualización de animaciones de la aplicación |
| `std::atomic` | coordinación de estados de animación |
| `std::function` | callback usado por `GameApplication` |
| `std::algorithm` | búsquedas/comprobaciones en tests y lógica auxiliar |

No se utiliza `new` ni `delete` directamente en el diseño principal.

---

## 15. Elección de contenedores

### `std::array`

El tamaño del grid se conoce en compilación:

```cpp
std::array<CellType, Rows * Columns>
```

Ventajas:

- almacenamiento contiguo;
- no requiere reserva dinámica del tablero;
- las dimensiones forman parte del tipo;
- funciona bien con iteradores.

### `std::vector`

Se utiliza cuando la cantidad de elementos depende de la ejecución:

- acciones disponibles;
- eventos producidos en un turno;
- historial de acciones de una simulación.

### `std::variant`

Las alternativas de celdas y eventos se conocen en compilación, por lo que `variant` evita una jerarquía polimórfica innecesaria para estos objetos.

### `std::span`

Permite pasar una vista ligera y no propietaria de un rango contiguo sin copiar el contenedor.

---

## 16. Interfaz de consola

La presentación utiliza **FTXUI v7.0.3**.

FTXUI queda restringido a la capa de presentación y coordinación de la aplicación; el núcleo de `Grid`, celdas, reglas, eventos y `NavigationEnvironment` no depende de entrada estándar.

### Controles

| Tecla | Alternativa | Acción |
|---|---|---|
| `W` | `↑` | mover arriba |
| `S` | `↓` | mover abajo |
| `A` | `←` | mover izquierda |
| `D` | `→` | mover derecha |
| `E` | — | esperar un turno |
| `H` | — | abrir/cerrar ayuda |
| `Q` | — | salir |

Las letras aceptan mayúsculas y minúsculas.

### Render modes

El menú permite seleccionar:

- **Emoji**
- **ASCII**

El modo ASCII existe para terminales que no representan correctamente Unicode o emojis de ancho completo.

---

## 17. Escenarios

El proyecto contiene dos constructores de escenarios de **20 × 30**:

```text
src/scenarios/scenario_1.cpp
src/scenarios/scenario_2.cpp
```

y sus interfaces:

```text
include/circuit_escape/scenarios/scenario_1.h
include/circuit_escape/scenarios/scenario_2.h
```

La posición inicial utilizada por `GameApplication` es:

```cpp
Position{1, 1}
```

### Scenario 1

Incluye:

- muros;
- terreno elevado;
- 2 recursos;
- 2 baterías;
- 2 trampas;
- salida en `{10, 15}`.

### Scenario 2

Incluye:

- muros;
- terreno elevado;
- 3 recursos;
- 2 baterías;
- 3 trampas;
- salida en `{12, 18}`.

### Opción `Random`

La interfaz gráfica del menú contiene una opción visual denominada **Random**, pero en la versión actual revisada no existe todavía un generador procedural asociado a esa opción.

---

# 18. Estructura del repositorio

```text
Maze/
├── app/
│   └── main.cpp
│
├── assets/
│   └── maps/
│
├── docs/
│   ├── design.md
│   └── contributions.md
│
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
│       ├── scenarios/
│       │   ├── scenario_1.h
│       │   └── scenario_2.h
│       └── [headers de animaciones/UI]
│
├── src/
│   ├── agent.cpp
│   ├── console_ui.cpp
│   ├── controllers.cpp
│   ├── environment.cpp
│   ├── game_rules.cpp
│   ├── GameApplication.cpp
│   ├── menus.cpp
│   ├── types.cpp
│   ├── scenarios/
│   │   ├── scenario_1.cpp
│   │   └── scenario_2.cpp
│   └── [implementaciones de animaciones]
│
├── tests/
│   ├── test_main.cpp
│   ├── grid_test.cpp
│   ├── environment_test.cpp
│   ├── interactions_test.cpp
│   ├── controllers_test.cpp
│   ├── simulation_test.cpp
│   ├── scenario_1_test.cpp
│   ├── scenario_2_test.cpp
│   ├── generic_functions_test.cpp
│   ├── console_ui_test.cpp
│   ├── event_test.cpp
│   └── console_render_test.cpp
│
├── .gitignore
├── CMakeLists.txt
└── README.md
```

---

# 19. Requisitos

## Software

- compilador compatible con **C++20**;
- **CMake 3.20** o superior;
- **Git**;
- terminal moderna;
- conexión a Internet durante la primera configuración de CMake para descargar FTXUI.

Compiladores recomendados:

- GCC;
- Clang;
- MSVC.

---

# 20. Compilación

## Linux

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel
```

Ejecución:

```bash
./build/Maze
```

---

## macOS

Con Xcode Command Line Tools y CMake instalados:

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel

./build/Maze
```

---

## Windows — PowerShell

```powershell
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --config Debug
```

Con Visual Studio como generador, el ejecutable normalmente estará en:

```powershell
.\build\Debug\Maze.exe
```

---

## Windows — Command Prompt

```cmd
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --config Debug
```

Luego:

```cmd
build\Debug\Maze.exe
```

---

# 21. Pruebas automáticas

Las suites se encuentran en `tests/` y utilizan principalmente `assert`.

El ejecutable agregado por `tests/test_main.cpp` llama a las suites de:

- Grid;
- Environment;
- interacciones;
- controllers;
- simulación;
- escenario 1;
- escenario 2;
- algoritmos genéricos;
- ConsoleUI;
- eventos;
- renderizado.

## Ejecución esperada mediante CTest

Una vez habilitado el target de pruebas en `CMakeLists.txt`:

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

En Windows con un generador multi-config:

```powershell
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

---

## 22. Cobertura funcional de las pruebas

El código de pruebas del repositorio cubre, entre otros:

- acceso válido e inválido al grid;
- dimensiones del grid;
- movimiento normal;
- movimiento contra muro;
- movimiento fuera del tablero;
- `wait`;
- costos configurables;
- terreno elevado;
- recurso de un solo uso;
- batería consumible;
- límite de recarga por energía máxima;
- trampas repetibles;
- penalizaciones de trampas;
- victoria;
- fin por energía;
- fin por turnos;
- precedencia de condiciones;
- acciones disponibles;
- perfiles `easy`, `standard` y `hard`;
- `RandomPolicy`;
- reproducibilidad con la misma semilla;
- `HeuristicPolicy`;
- `NavigationPolicy`;
- `PolicyController`;
- simulación automática;
- algoritmos genéricos con `vector`, `array` y rangos vacíos;
- traducción de teclado;
- eventos;
- representación ASCII;
- representación emoji;
- dimensiones 20 × 30 del render.

---

# 23. Prueba negativa del `concept`

En `controllers_test.cpp` existe una comprobación positiva/negativa en compilación:

```cpp
static_assert(NavigationPolicy<RandomPolicy>);
static_assert(NavigationPolicy<HeuristicPolicy>);

struct InvalidPolicy {};
static_assert(!NavigationPolicy<InvalidPolicy>);
```

Esto comprueba que `InvalidPolicy` **no** satisface el concept.

Sin embargo, para cumplir literalmente el requisito de **documentar el diagnóstico de una compilación fallida**, se recomienda mantener además un archivo/documento separado con un ejemplo intencionalmente inválido, por ejemplo:

```cpp
struct InvalidPolicy {};

PolicyController<InvalidPolicy> controller{
    InvalidPolicy{}
};
```

Ese archivo no debe formar parte de la compilación normal. Su objetivo es mostrar el mensaje del compilador indicando que `InvalidPolicy` no satisface `NavigationPolicy`.

---

# 24. Decisiones de diseño

## 24.1 Motor independiente de la UI

`NavigationEnvironment` no utiliza `std::cin`, `std::cout` ni FTXUI.

Esto permite:

- pruebas automáticas;
- controladores automáticos;
- simulación reproducible;
- futura incorporación de otro controlador sin modificar el motor.

---

## 24.2 Reglas como datos

Los perfiles de dificultad se representan mediante `GameRules`.

Esto evita una lógica del tipo:

```cpp
if (difficulty == ...)
```

dispersa por todo el motor.

---

## 24.3 `std::variant` para celdas

Los siete tipos de celda son conocidos en compilación.

`std::variant` permite almacenar cualquiera de ellos manteniendo type safety sin una jerarquía de herencia para cada celda.

---

## 24.4 Eventos tipados

Los cambios del juego se comunican mediante `NavigationEvent`, no mediante strings.

La UI decide posteriormente cómo representarlos.

---

## 24.5 `Observation` como frontera

Los controllers no reciben acceso modificable al environment.

Reciben un estado observable y proponen una acción.

---

## 24.6 Templates para dimensiones

```cpp
Grid<Cell, 20, 30>
```

hace que las dimensiones formen parte del tipo.

Los tests pueden crear:

```cpp
Grid<Cell, 1, 3>
Grid<Cell, 2, 2>
Grid<Cell, 3, 4>
```

sin cambiar el motor.

---

## 24.7 Semilla controlable

`RandomPolicy` utiliza un `std::mt19937` inicializado con una semilla explícita.

Esto permite reproducir las simulaciones automáticas en tests.

---

# 25. Demostración sugerida

Para una demostración reproducible del proyecto:

1. iniciar la aplicación;
2. mostrar el menú;
3. seleccionar `Standard`;
4. mostrar los modos Emoji y ASCII;
5. iniciar Scenario 1;
6. demostrar movimiento normal;
7. intentar entrar a un muro;
8. atravesar terreno elevado;
9. recoger un recurso;
10. utilizar una batería;
11. activar una trampa;
12. abrir la ayuda con `H`;
13. alcanzar la salida o mostrar una condición de derrota;
14. ejecutar las pruebas mediante CTest;
15. explicar una simulación automática con semilla fija.

---

# 26. Limitaciones conocidas

- Los escenarios tienen dimensiones fijas en tiempo de compilación.
- No existe movimiento diagonal.
- No hay enemigos móviles.
- No hay juego en red.
- No existe persistencia en base de datos.
- No se implementa reinforcement learning en esta etapa.
- `HeuristicPolicy` usa una heurística local y no garantiza la ruta global óptima.
- No se implementa A*, Dijkstra ni búsqueda avanzada.
- La opción visual `Random` del menú todavía no genera un escenario procedural.
- El modo Emoji depende de que la terminal represente correctamente Unicode de ancho completo.

---

# 27. Estado de la versión de `main` revisada

> **Importante antes de crear el tag `proyecto-1-entrega`:** esta sección describe el estado observado del repositorio durante la preparación de este README. Debe resolverse y actualizarse antes de la entrega final.

### CMake

En el `CMakeLists.txt` revisado:

- `src/scenarios/scenario_2.cpp` existe, pero no está agregado actualmente a `Maze_core`;
- `src/menus.cpp` existe, pero no está agregado actualmente a `Maze_core`;
- el bloque de tests (`enable_testing`, `Maze_test` y `add_test`) está comentado.

Por lo tanto, antes de la entrega se debe verificar que `CMakeLists.txt` incluya **todas** las unidades de traducción necesarias y que CTest esté habilitado.

### Documentación adicional

Los archivos:

```text
docs/design.md
docs/contributions.md
```

existen en el repositorio revisado pero actualmente no contienen documentación.

El enunciado solicita que ambos formen parte de la entrega final.

### README

Antes de entregar:

- completar grupo;
- completar nombres;
- completar códigos UTEC;
- completar usuarios GitHub;
- completar contribuciones;
- registrar el hash completo;
- crear y verificar el tag `proyecto-1-entrega`;
- actualizar esta sección una vez corregidos los pendientes.

---

# 28. Checklist previo a la entrega

## Repositorio

- [ ] `main` compila desde un clon limpio.
- [ ] `CMakeLists.txt` incluye `src/menus.cpp`.
- [ ] `CMakeLists.txt` incluye `src/scenarios/scenario_2.cpp`.
- [ ] `Maze` ejecuta correctamente.
- [ ] Se habilitó el target de pruebas.
- [ ] `ctest` ejecuta y finaliza correctamente.
- [ ] Scenario 1 funciona.
- [ ] Scenario 2 funciona.
- [ ] `easy`, `standard` y `hard` funcionan.
- [ ] Emoji funciona.
- [ ] ASCII funciona.
- [ ] La simulación con semilla fija es reproducible.
- [ ] Se documentó la prueba negativa del concept.
- [ ] `docs/design.md` está completo.
- [ ] `docs/contributions.md` está completo.
- [ ] El README contiene integrantes y códigos.
- [ ] Se documentaron aportes individuales.
- [ ] Se registró el hash completo del commit.
- [ ] Se creó el tag anotado `proyecto-1-entrega`.

## Validación desde cero

Antes de etiquetar la entrega:

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git Maze-verificacion
cd Maze-verificacion

cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/Maze
```

La entrega solo debería etiquetarse después de comprobar este flujo desde una carpeta limpia.

---

# 29. Creación del tag de entrega

Cuando la versión final ya esté verificada:

```bash
git switch main
git pull --ff-only
git status
```

Comprobar el commit:

```bash
git rev-parse HEAD
```

Crear el tag anotado:

```bash
git tag -a proyecto-1-entrega -m "Entrega Proyecto 1 - Circuito de Escape"
```

Publicarlo:

```bash
git push origin proyecto-1-entrega
```

Verificar:

```bash
git show proyecto-1-entrega
```

El hash completo mostrado debe coincidir con el commit registrado en Canvas.

---

# 30. Contribuciones individuales

> Completar esta tabla con información verificable mediante commits, issues y pull requests.

| Integrante | Implementación | Pruebas | Documentación / integración | PRs o issues relevantes |
|---|---|---|---|---|
| **[Integrante 1]** | [detalle] | [detalle] | [detalle] | [#...] |
| **[Integrante 2]** | [detalle] | [detalle] | [detalle] | [#...] |
| **[Integrante 3]** | [detalle] | [detalle] | [detalle] | [#...] |
| **[Integrante 4]** | [detalle] | [detalle] | [detalle] | [#...] |
| **[Integrante 5]** | [detalle] | [detalle] | [detalle] | [#...] |

Cada integrante debe poder explicar:

- su contribución;
- el flujo general del motor;
- las decisiones principales de diseño;
- cómo ejecutar las pruebas;
- dónde aparecen los temas obligatorios de C++20.

---

# 31. Uso de herramientas de IA

> Completar únicamente de acuerdo con la política comunicada por el docente.

Si el uso de IA generativa está autorizado, documentar:

| Herramienta | Uso | Cómo se verificó |
|---|---|---|
| **[Herramienta]** | **[tarea para la que se utilizó]** | **[pruebas/revisión realizada]** |

La responsabilidad sobre el código, pruebas y documentación entregados corresponde al grupo.

---

# 32. Dependencias y referencias

## FTXUI

El proyecto utiliza:

**FTXUI v7.0.3**

Repositorio oficial:

https://github.com/ArthurSonzogni/FTXUI

La dependencia se obtiene mediante CMake `FetchContent`.

## C++20

Referencia general:

https://en.cppreference.com/

## CMake

https://cmake.org/documentation/

---

# 33. Licencia

En la versión revisada del repositorio no se observa un archivo `LICENSE` en la raíz.

Si el grupo desea publicar el proyecto con una licencia open source, debe agregar una licencia explícita y actualizar esta sección.

---

# 34. Resumen rápido

```bash
git clone https://github.com/leonardolandeo-a11y/Maze.git
cd Maze

cmake -S . -B build
cmake --build build --parallel

./build/Maze
```

Pruebas, después de habilitar correctamente el target en CMake:

```bash
ctest --test-dir build --output-on-failure
```

---

**Circuito de Escape** demuestra un motor de navegación por turnos desacoplado de la interfaz, con reglas configurables, tipos genéricos, eventos tipados, controladores intercambiables y simulación reproducible, aplicando de manera integrada los principales contenidos de C++20 exigidos en el Proyecto 1 de CS2013.
