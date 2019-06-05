//
// Created by Marcin on 27.05.2019.
//

#include <cstring>
#include "TupleElement.h"

TupleElement::TupleElement(TupleElementType type, const char *value) {
    this->type = type;
    this->value.stringValue = new char[strlen(value)];
    memcpy(this->value.stringValue, value, strlen(value));
}

TupleElement::TupleElement(TupleElementType type, int value) {
    this->type = type;
    this->value.intValue = value;
}

TupleElement::TupleElement(TupleElementType type, float value) {
    this->type = type;
    this->value.floatValue = value;
}

TupleElementType TupleElement::getType() {
    return type;
}

const char *TupleElement::getStringValue() {
    return value.stringValue;
}

int TupleElement::getIntValue() {
    return value.intValue;
}

float TupleElement::getFloatValue() {
    return value.floatValue;
}
