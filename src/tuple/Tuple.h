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
private:
    std::vector<TupleElement> elements;
    std::string type;
    int serialNumber; // jeśli różny od -1, to oznacza że krotka jest zablokowana

public:

    template<typename T, typename... Args>
    void addElement(T first, Args... args);

    template<typename T>
    void addElement(T v);

    Tuple() : serialNumber(-1) {}

    template<typename... Args>
    explicit Tuple(Args... args);

    bool matchPattern(std::string);
    void print();
    std::string toString();

    size_t getSize() { return elements.size(); }

    TupleElement &operator[](int position) { return elements[position]; }

    std::string getType() { return type; }

    void setSerialNumber(int serialNumber_) { serialNumber = serialNumber_; }

    int getSerialNumber() { return serialNumber; }

    bool isBlocked() { return serialNumber != -1; }

    bool operator==(const Tuple &rhs) const { return elements == rhs.elements && type == rhs.type; }

    bool operator!=(const Tuple &rhs) const { return !(rhs == *this); }
};

template<typename... Args>
Tuple::Tuple(Args... args) : serialNumber(-1) {
    addElement(args...);
}

template<typename T>
void Tuple::addElement(T v) {

}

template<>
inline void Tuple::addElement<int>(int v) {
    type.append("i");
    elements.emplace_back(TupleElement(INT, v));
}

template<>
inline void Tuple::addElement<float>(float v) {
    type.append("f");
    elements.emplace_back(TupleElement(FLOAT, v));
}

template<>
inline void Tuple::addElement<const char *>(const char *v) {
    type.append("s");
    elements.emplace_back(TupleElement(STRING, v));
}

template<typename T, typename... Args>
void Tuple::addElement(T first, Args... args) {
    addElement(first);
    addElement(args...);
}

#endif //UXP1A_TUPLE_H