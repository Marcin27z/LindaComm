//
// Created by Marcin on 27.05.2019.
//

#include <iostream>
#include "Tuple.h"

size_t Tuple::getSize() {
    return elements.size();
}

TupleElement &Tuple::operator[](int position) {
    return elements[position];
}


bool Tuple::matchPattern(std::string pattern) {
//    size_t last = 0;
//    size_t next = 0;
//    std::string delimiter = ", ";
//    while ((next = pattern.find(delimiter, last)) != std::string::npos) {
//        std::cout << pattern.substr(last, next - last) << std::endl;
//        last = next + 1;
//    }
//    std::cout << pattern.substr(last) << std::endl;
    return true;
}

std::string Tuple::getType() {
    return type;
}

void Tuple::print() {
    for (int i = 0; i < getSize(); ++i) {
        if (elements[i].getType() == STRING) {
            std::cout << elements[i].getStringValue() << " ";
        } else if (elements[i].getType() == INT) {
            std::cout << elements[i].getIntValue() << " ";
        } else {
            std::cout << elements[i].getFloatValue() << " ";
        }
    }
    std::cout << std::endl;
}

Tuple::Tuple() {

}

