# Prueba negativa del concept `NavigationPolicy`

## Objetivo

El proyecto utiliza el concept `NavigationPolicy` para verificar en tiempo de compilación que una política proporcione un método `selectAction` con la firma requerida.

El concept se encuentra definido en el archivo:

`include/circuit_escape/controllers.h`

```cpp
template<typename Policy>
concept NavigationPolicy = requires(
    Policy& policy,
    const Observation& observation,
    std::span<const Action> actions
) {
    { policy.selectAction(observation, actions) }
        -> std::same_as<Action>;
};
```

## Política invalida

Para comprobar el funcionamiento del concept se utiliza el archivo:

```cpp
#include "circuit_escape/controllers.h"

struct InvalidPolicy {
    int selectAction(
        const Observation&,
        std::span<const Action>
    ) {
        return 0;
    }
};

int main() {
    PolicyController<InvalidPolicy> controller{
        InvalidPolicy{}
    };

    return 0;
}
```

`InvalidPolicy` proporciona un método selectAction, pero su tipo de retorno es int en lugar de Action.

Para compilar la prueba se uso el siguiente comando:

```bash
g++ -std=c++20 -Iinclude tests/invalid_test.cpp -c
```

El diagnóstico del compilador fue el siguiente:

```bash
error: template constraint failure
note: constraints not satisfied
required for the satisfaction of ‘NavigationPolicy<Policy>’
‘policy.selectAction(observation, actions)’ does not satisfy return-type-requirement
```
El diagnóstico muestra que `InvalidPolicy` no satisface el requisito de retorno `std::same_as<Action>`.

La falla de compilación es intencional y demuestra que NavigationPolicy detecta en tiempo de compilación una política que no cumple el contrato requerido.