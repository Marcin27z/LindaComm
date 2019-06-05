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
    TupleElementType getType();


    const char *getStringValue();
    int getIntValue();
    float getFloatValue();
};






#endif //UXP1A_TUPLEELEMENT_H
