#include <utility>

//
// Created by Marcin on 27.05.2019.
//

#include <iostream>
#include <cstring>
#include "Tuple.h"
#include "../parser/Parser.h"

size_t Tuple::getSize() {
    return elements.size();
}

TupleElement &Tuple::operator[](int position) {
    return elements[position];
}


bool Tuple::matchPattern(std::string pattern) {
    Parser parser;
    std::vector<Element *> result = parser.parse(std::move(pattern));
    if (this->getSize() != result.size()) {
        return false;
    }
    for (auto i = 0; i < elements.size(); ++i) {
        if (elements[i].getType() == TupleElementType::STRING && result[i]->type == Element::Type::STRING) {
            if (result[i]->specified) {
                auto *sr = (String_Requirement *) result[i]->req;
                std::string temp(elements[i].getStringValue());
                if (sr->type == Requirement::GREATER_THAN) {
                    if (temp.compare(sr->value) <= 0) {
                        return false;
                    }
                } else if (sr->type == Requirement::LESS_THAN) {
                    if (temp.compare(sr->value) >= 0) {
                        return false;
                    }
                } else if (sr->type == Requirement::EQUAL) {
                    if (temp != sr->value) {
                        return false;
                    }
                } else if (sr->type == Requirement::GREATER_OR_EQUAL) {
                    if (temp.compare(sr->value) < 0) {
                        return false;
                    }
                } else {
                    if (temp.compare(sr->value) > 0) {
                        return false;
                    }
                }
            }
        } else if (elements[i].getType() == TupleElementType::INT && result[i]->type == Element::Type::INT) {
            if (result[i]->specified)  {
                auto *ir = (Int_Requirement *) result[i]->req;
                if (ir->type == Requirement::GREATER_THAN) {
                    if (elements[i].getIntValue() <= ir->value) {
                        return false;
                    }
                } else if (ir->type == Requirement::LESS_THAN) {
                    if (elements[i].getIntValue() >= ir->value) {
                        return false;
                    }
                } else if (ir->type == Requirement::EQUAL) {
                    if (elements[i].getIntValue() != ir->value) {
                        return false;
                    }
                } else if (ir->type == Requirement::GREATER_OR_EQUAL) {
                    if (elements[i].getIntValue() < ir->value) {
                        return false;
                    }
                } else {
                    if (elements[i].getIntValue() > ir->value) {
                        return false;
                    }
                }
            }
        } else if (elements[i].getType() == TupleElementType::FLOAT && result[i]->type == Element::Type::FLOAT) {
            if (result[i]->specified) {
                auto *fr = (Int_Requirement *) result[i]->req;
                if (fr->type == Requirement::GREATER_THAN) {
                    if (elements[i].getIntValue() <= fr->value) {
                        return false;
                    }
                } else if (fr->type == Requirement::LESS_THAN) {
                    if (elements[i].getIntValue() >= fr->value) {
                        return false;
                    }
                } else if (fr->type == Requirement::GREATER_OR_EQUAL) {
                    if (elements[i].getIntValue() < fr->value) {
                        return false;
                    }
                } else {
                    if (elements[i].getIntValue() > fr->value) {
                        return false;
                    }
                }
            }
        } else {
            return false;
        }
    }
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

