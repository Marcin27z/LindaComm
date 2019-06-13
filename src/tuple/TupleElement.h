//
// Created by Marcin on 27.05.2019.
//

#ifndef UXP1A_TUPLEELEMENT_H
#define UXP1A_TUPLEELEMENT_H

#include "TupleElementType.h"

class TupleElement {
private:
    TupleElementType type;
    union Value {
        char* stringValue;
        int intValue;
        float floatValue;
    } value;

public:
    TupleElement(TupleElementType, const char *);
    TupleElement(TupleElementType, int);
    TupleElement(TupleElementType, float);
    TupleElementType getType() {  return type; }

    const char * getStringValue() const { return value.stringValue; }
    int getIntValue() const { return value.intValue; }
    float getFloatValue() const { return value.floatValue; }
    Value getValue() const { return value; }

    bool operator==(const TupleElement &rhs) const;

    bool operator!=(const TupleElement &rhs) const { return !(rhs == *this); }
};

#endif //UXP1A_TUPLEELEMENT_H
