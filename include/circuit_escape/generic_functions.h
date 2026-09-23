#pragma once

#include <cstddef>
/*
IMplementacion de funciones genericas: requerimientos de la rubrica:

- countMatching: Recorre mediante iteradores un conj de datos 
y cuenta los que cumplen el predicado (funcion lambda)

- LinearSearch: Recorre mediante iteradores un conj de datos
y devuelve el primero que cumpla el predicado (funcion lambda)
*/
template<typename Iterator, typename Predicado>
std::size_t countMatching(Iterator first, Iterator last,Predicado predicado) {
    std::size_t count = 0;

    for (; first != last; ++first) {
        if (predicado(*first)) {
            ++count;
        }
    }

    return count;
}

template<typename Iterator, typename Predicado>
Iterator LinearSearch(Iterator first, Iterator last,Predicado predicado) {
    for (; first != last; ++first) {
        if (predicado(*first)) {
            return first;
        }
    }

    return last;
}