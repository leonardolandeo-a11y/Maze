# Decisiones de diseño

Este documento resume las principales decisiones de diseño de **Circuito de Escape** y señala dónde se aplican los temas obligatorios de Programación III.

## 1. Arquitectura general

El proyecto separa la presentación, la coordinación de la aplicación y el motor de simulación.

La interfaz recibe las acciones del usuario y muestra el estado del juego. La lógica principal se mantiene en `NavigationEnvironment`, mientras que `GameApplication` coordina la ejecución de una partida.

Archivos principales:

- `include/circuit_escape/environment.h`
- `src/environment.cpp`
- `include/circuit_escape/GameApplication.h`
- `src/GameApplication.cpp`
- `include/circuit_escape/console_ui.h`
- `src/console_ui.cpp`

Esta separación permite que la lógica del entorno pueda ejecutarse y probarse sin depender directamente de la interfaz de consola.

## 2. Template de clase

El tablero utiliza la clase genérica:

```cpp
template<typename CellType, std::size_t Rows, std::size_t Columns>
class Grid;
```

Ubicación:

`include/circuit_escape/grid.h`

`Grid` recibe un parámetro de tipo y dos parámetros no-tipo para representar las dimensiones del tablero.

Las dimensiones forman parte del tipo y el almacenamiento interno utiliza:

```cpp
std::array<CellType, Rows * Columns>
```

Esto resulta adecuado porque el tamaño del tablero se conoce en tiempo de compilación.

La clase proporciona además acceso validado, consulta de posiciones válidas e iteradores `const` y no `const`.

## 3. Templates de funciones

El proyecto utiliza funciones genéricas basadas en templates para reutilizar algoritmos con diferentes tipos de datos y contenedores.

La implementación principal se encuentra en:

`include/circuit_escape/generic_functions.h`

También existen funciones genéricas relacionadas con el entorno y con la simulación automática.

Funciones utilizadas:

- `countMatching(Iterator first, Iterator last, Predicado predicado)`, definida en `include/circuit_escape/generic_functions.h`, recorre un rango mediante iteradores y cuenta los elementos que cumplen un predicado.
- `LinearSearch(Iterator first, Iterator last, Predicado predicado)`, definida en `include/circuit_escape/generic_functions.h`, busca mediante iteradores el primer elemento que cumple un predicado.
- `firstSatisfiedTermination(const Conditions&... conditions)`, definida en `include/circuit_escape/environment.h`, evalúa un conjunto variable de condiciones de término mediante una fold expression.
- `runRandomSimulation(NavigationEnvironment<Rows, Columns>& environment, std::uint32_t seed)`, definida en `include/circuit_escape/simulation.h`, ejecuta una simulación automática parametrizada por las dimensiones del entorno.

Estas funciones permiten evitar la duplicación de algoritmos para distintos contenedores.

## 4. Especialización total

El proyecto utiliza una especialización total de:

```cpp
CellTraits<Wall>
```

Ubicación:

`include/circuit_escape/cells.h`

Esta especialización permite representar de manera específica las propiedades correspondientes a una celda de tipo `Wall`.

## 5. Especialización parcial

El proyecto utiliza una especialización parcial de:

```cpp
CellTraits<ResourceCell<Reward>>
```

Ubicación:

`include/circuit_escape/cells.h`

Al estar parametrizada por `Reward`, una misma definición puede utilizarse para distintos tipos de recompensa asociados a recursos.

## 6. Template variádico

El proyecto utiliza el template variádico:

```cpp
template<class... FLambda>
struct Overloaded : FLambda... {
    using FLambda::operator()...;
};
```

Ubicación:

`include/circuit_escape/environment.h`

`Overloaded` permite combinar varias lambdas y utilizarlas conjuntamente con `std::visit` para procesar las alternativas almacenadas en un `std::variant`.

## 7. Fold expression

El proyecto utiliza una fold expression para evaluar condiciones de término.

La operación utilizada es equivalente a:

```cpp
(evaluate(conditions), ...);
```

Función asociada:

`firstSatisfiedTermination()`

Ubicación:

`include/circuit_escape/environment.h`

La fold expression permite evaluar de manera genérica varias condiciones sin duplicar la misma estructura de control.

## 8. Concepts

El proyecto define el concept:

```cpp
NavigationPolicy
```

Ubicación principal:

`include/circuit_escape/controllers.h`

El objetivo de `NavigationPolicy` es comprobar durante la compilación que una policy proporcione la operación requerida para seleccionar una acción.

El contrato es equivalente a:

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

De esta manera, una policy que no respete la interfaz esperada no puede utilizarse mediante `PolicyController`.

También se incluye una prueba negativa de compilación para comprobar que una política que no satisface `NavigationPolicy` no puede utilizarse con `PolicyController`.

La prueba se encuentra documentada en: `docs/negative_concept_test.md`

y utiliza una `InvalidPolicy` cuyo método `selectAction` devuelve `int` en lugar de `Action`. El compilador rechaza la instanciación al no satisfacerse el requisito:

```cpp
std::same_as<Action>
```

## 9. Polimorfismo dinámico

El proyecto utiliza la interfaz:

```cpp
IController
```

para representar controladores automáticos mediante un contrato común.

La interfaz define una operación virtual equivalente a:

```cpp
virtual Action selectAction(
    const Observation& observation,
    std::span<const Action> legalActions
) = 0;
```

PolicyController<Policy> implementa esta interfaz y adapta cualquier policy que satisfaga el concept NavigationPolicy.

Los controladores polimórficos pueden administrarse mediante:

```cpp
std::unique_ptr<IController>
```
Por ejemplo, la simulación automática crea un controller concreto y lo maneja mediante la interfaz base:

```cpp
std::unique_ptr<IController> controller =
    std::make_unique<PolicyController<RandomPolicy>>(
        RandomPolicy{seed}
    );
```
La llamada:


```cpp
controller->selectAction(observation, legalActions);
```

utiliza despacho dinámico para ejecutar la implementación correspondiente al objeto concreto.

El uso de `std::unique_ptr` nos permite evitar el uso directo de new y delete (los cuales estan prohibidos para los fines de este proyecto)

De esta forma pueden utilizarse diferentes controladores a través del mismo contrato sin modificar `NavigationEnvironment`.

## 10. PolicyController

El adaptador genérico:

```cpp
PolicyController<Policy>
```

permite utilizar una policy que satisface `NavigationPolicy` mediante la interfaz polimórfica `IController`.

Su estructura es equivalente a:

```cpp
template<typename Policy>
requires NavigationPolicy<Policy>
class PolicyController : public IController {
    // ...
};
```

De esta forma se combinan tres mecanismos:

- templates;
- concepts;
- polimorfismo dinámico.

## 11. Controladores

El proyecto incluye como mínimo dos estrategias automáticas.

### RandomPolicy

`RandomPolicy` selecciona una acción legal utilizando generación pseudoaleatoria.

Utiliza:

```cpp
std::mt19937
```

y recibe una semilla controlable, permitiendo reproducir la misma secuencia de decisiones durante las pruebas.

### HeuristicPolicy

`HeuristicPolicy` utiliza una estrategia basada en la distancia Manhattan respecto de la salida:

```text
|row1 - row2| + |column1 - column2|
```

Su objetivo es seleccionar una acción legal que acerque al agente a la salida.

No se pretende garantizar una ruta global óptima.

## 12. Estado observable

Los controladores no acceden directamente al estado interno de `NavigationEnvironment`.

En su lugar utilizan una representación observable mediante:

```cpp
Observation
```

que contiene información relevante como:

- posición del agente;
- posición de la salida;
- energía;
- puntaje;
- recursos recogidos;
- turno actual;
- límite de turnos;
- acciones disponibles.

Esto mantiene desacoplado al controlador de la implementación interna del entorno.

## 13. Resultado de una acción

Cada acción ejecutada sobre el entorno produce un resultado mediante:

```cpp
StepResult
```

que contiene información equivalente a:

```cpp
struct StepResult {
    Observation observation;
    std::vector<NavigationEvent> events;
    bool finished;
    EndReason reason;
};
```

Esto permite que la interfaz y los controladores trabajen con datos estructurados en lugar de analizar texto generado por consola.

## 14. Eventos mediante std::variant

Los eventos de navegación se representan mediante:

```cpp
std::variant
```

Los tipos de evento incluyen, entre otros:

- movimiento realizado;
- movimiento rechazado;
- recurso recogido;
- cambio de energía;
- activación de trampa;
- llegada a la salida.

La interfaz procesa estos eventos utilizando:

```cpp
std::visit
```

Esto evita crear una jerarquía de clases polimórficas separada para cada tipo de evento.

## 15. Biblioteca estándar y elección de contenedores

### std::array

Se utiliza como almacenamiento interno de `Grid`.

Su elección se debe a que las dimensiones del tablero son conocidas en tiempo de compilación y no necesitan cambiar durante la ejecución.

### std::vector

Se utiliza para colecciones cuyo tamaño depende de la ejecución, por ejemplo:

- acciones disponibles;
- eventos de un turno;
- resultados o secuencias de simulación.

### std::variant

Se utiliza para representar diferentes tipos de celda y diferentes tipos de eventos de forma segura.

### std::visit

Se utiliza para procesar los valores almacenados dentro de un `std::variant`.

### std::optional

Se utiliza cuando una operación puede no producir un resultado válido, por ejemplo al interpretar determinadas entradas o búsquedas.

### std::span

Se utiliza para proporcionar una vista no propietaria sobre una secuencia de elementos, evitando copias innecesarias.

### std::unique_ptr

Se utiliza para administrar controladores polimórficos mediante la interfaz `IController`.

### std::mt19937

Se utiliza para generar decisiones pseudoaleatorias reproducibles mediante una semilla controlable.

### Algoritmos de <algorithm>

Los algoritmos estándar se utilizan junto con contenedores e iteradores cuando corresponde, evitando implementar manualmente operaciones ya disponibles en la biblioteca estándar.

## 16. Separación entre motor e interfaz

La lógica del entorno puede utilizarse sin leer directamente desde `std::cin` ni escribir directamente en `std::cout`.

Esto permite:

- ejecutar pruebas automáticas;
- realizar simulaciones sin interfaz;
- utilizar controladores diferentes sobre el mismo entorno;
- mantener FTXUI limitado a la capa de presentación.

## 17. Simulación reproducible

El proyecto permite realizar simulaciones automáticas utilizando una semilla explícita.

Una interfaz disponible es:

```cpp
template<std::size_t Rows, std::size_t Columns>
SimulationResult runRandomSimulation(
    NavigationEnvironment<Rows, Columns>& environment,
    std::uint32_t seed
);
```

El uso de una semilla fija permite comprobar que una misma configuración inicial produzca una secuencia reproducible durante las pruebas.

## 18. Escenarios

El proyecto incluye dos escenarios principales de:

```text
20 × 30
```

implementados en:

- `src/scenarios/scenario_1.cpp`
- `src/scenarios/scenario_2.cpp`

con sus respectivas interfaces en:

- `include/circuit_escape/scenarios/scenario_1.h`
- `include/circuit_escape/scenarios/scenario_2.h`

Ambos escenarios deben mantener una ruta alcanzable desde la posición inicial hasta la salida.

## 19. Perfiles de dificultad

Las reglas configurables se agrupan mediante `GameRules`.

El proyecto proporciona los perfiles:

- `Difficulty::easy`;
- `Difficulty::standard`;
- `Difficulty::hard`.

`NavigationEnvironment` recibe las reglas correspondientes al perfil seleccionado, evitando distribuir condicionales de dificultad por distintas partes del motor.

## 20. Conclusión

El diseño de Circuito de Escape busca mantener separadas la lógica del entorno, los controladores y la interfaz.

Los templates, concepts, especializaciones, iteradores, contenedores de la biblioteca estándar y el polimorfismo dinámico se utilizan dentro de la arquitectura del proyecto para resolver necesidades concretas del dominio y permitir que el motor pueda reutilizarse tanto en partidas interactivas como en simulaciones automáticas.