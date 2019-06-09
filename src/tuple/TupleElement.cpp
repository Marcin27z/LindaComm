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

const char * TupleElement::getStringValue() const {
    return value.stringValue;
}

int TupleElement::getIntValue() const {
    return value.intValue;
}

float TupleElement::getFloatValue() const {
    return value.floatValue;
}

bool TupleElement::operator==(const TupleElement &rhs) const {
    if (type == rhs.type) {
        if (type == STRING) {
            return value.stringValue == rhs.getStringValue();
        } else if (type == INT) {
            return value.intValue == rhs.getIntValue();
        } else if (type == FLOAT) {
            return value.floatValue == rhs.getFloatValue();
        }
    }
    return false;
}

bool TupleElement::operator!=(const TupleElement &rhs) const {
    return !(rhs == *this);
}
