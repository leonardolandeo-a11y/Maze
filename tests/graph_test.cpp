
#include "circuit_escape/graph.h"

#include <cassert>
#include <iostream>
#include <vector>

void run_graph_tests() {
    // Mapa de 3 filas y 3 columnas
    // 0 1 2
    // 3 4 #
    // 6 7 8

    std::vector<bool> blocked = {
        false, false, false,
        false, false, true,
        false, false, false
    };

    std::vector<int> costs(9, 1);

    GraphData graph = build_graph(3, 3, blocked, costs);

    // Verificar dimensiones
    assert(graph.rows == 3);
    assert(graph.columns == 3);

    // Verificar vecinos del nodo central
    std::vector<std::size_t> expected = {1, 7, 3};

    assert(graph_neighbors(graph, 4) == expected);

    // El nodo 5 es un muro
    assert(graph_neighbors(graph, 5).empty());

    // Comprobar conexiones de otro nodo
    std::vector<std::size_t> expected0 = {1, 3};
    assert(graph_neighbors(graph, 0) == expected0);

    std::cout << "All graph tests passed!\n";
}
