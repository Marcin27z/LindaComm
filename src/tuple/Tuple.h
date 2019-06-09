//
// Created by Marcin on 27.05.2019.
//

#ifndef UXP1A_TUPLE_H
#define UXP1A_TUPLE_H

#include <vector>
#include <cstdio>
#include <string>
#include "TupleElement.h"

class Tuple {
public:

private:
    std::vector<TupleElement> elements;
    std::string type;

public:


    template<typename T, typename... Args>
    void addElement(T first, Args... args);

    template<typename T>
    void addElement(T v);

    Tuple();

    template<typename... Args>
    explicit Tuple(Args... args);

    size_t getSize();

    TupleElement &operator[](int);

    bool matchPattern(std::string);

    std::string getType();

    void print();

    bool operator==(const Tuple &rhs) const;

    bool operator!=(const Tuple &rhs) const;
};

template<typename... Args>
Tuple::Tuple(Args... args) {
    addElement(args...);
}

template<typename T>
void Tuple::addElement(T v) {

}

template<>
inline void Tuple::addElement<int>(int v) {
    type.append("i");
    TupleElement tupleElement(INT, v);
    elements.push_back(tupleElement);
}

template<>
inline void Tuple::addElement<float>(float v) {
    type.append("f");
    TupleElement tupleElement(FLOAT, v);
    elements.push_back(tupleElement);
}

template<>
inline void Tuple::addElement<const char *>(const char *v) {
    type.append("s");
    TupleElement tupleElement(STRING, v);
    elements.push_back(tupleElement);
}

template<typename T, typename... Args>
void Tuple::addElement(T first, Args... args) {
    addElement(first);
    addElement(args...);
}


#endif //UXP1A_TUPLE_H
