//
// Created by Marcin on 27.05.2019.
//

#include "Tuple.h"

size_t Tuple::getSize() {
    return elements.size();
}

TupleElement &Tuple::operator[](int position) {
    return elements[position];
}


bool Tuple::matchPattern(std::string) {
    return false;
}

std::string Tuple::getType() {
    return type;
}

