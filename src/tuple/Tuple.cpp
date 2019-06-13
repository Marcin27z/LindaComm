#include <utility>

//
// Created by Marcin on 27.05.2019.
//

#include <iostream>
#include <cstring>
#include "Tuple.h"
#include "../parser/Parser.h"

bool Tuple::matchPattern(std::string pattern) {
    if(isBlocked()) return false;

    Parser parser;
    std::vector<Element *> result = parser.parse(std::move(pattern));
    if(this->getSize() != result.size())
        return false;

    for(auto i = 0; i < elements.size(); ++i) {
        if(elements[i].getType() == TupleElementType::STRING && result[i]->getType() == Element::Type::STRING) {
            if(result[i]->isSpecified()) {
                auto *sr = (String_Requirement *) result[i]->getReq();
                std::string temp(elements[i].getStringValue());
                switch(sr->getType()) {
                    case Requirement::GREATER_THAN:
                        if(temp.compare(sr->getValue()) <= 0) return false;
                        break;
                    case Requirement::LESS_THAN:
                        if(temp.compare(sr->getValue()) >= 0) return false;
                        break;
                    case Requirement::EQUAL:
                        if(temp != sr->getValue()) return false;
                        break;
                    case Requirement::GREATER_OR_EQUAL:
                        if(temp.compare(sr->getValue()) < 0) return false;
                        break;
                    default:
                        if(temp.compare(sr->getValue()) > 0) return false;
                        break;
                }
            }
        } else if (elements[i].getType() == TupleElementType::INT && result[i]->getType() == Element::Type::INT) {
            if(result[i]->isSpecified()) {
                auto *ir = (Int_Requirement *) result[i]->getReq();
                switch(ir->getType()) {
                    case Requirement::GREATER_THAN:
                        if (elements[i].getIntValue() <= ir->getValue()) return false;
                        break;
                    case Requirement::LESS_THAN:
                        if (elements[i].getIntValue() >= ir->getValue()) return false;
                        break;
                    case Requirement::EQUAL:
                        if (elements[i].getIntValue() != ir->getValue()) return false;
                        break;
                    case Requirement::GREATER_OR_EQUAL:
                        if (elements[i].getIntValue() < ir->getValue()) return false;
                        break;
                    default:
                        if (elements[i].getIntValue() > ir->getValue()) return false;
                        break;
                }
            }
        } else if (elements[i].getType() == TupleElementType::FLOAT && result[i]->getType() == Element::Type::FLOAT) {
            if (result[i]->isSpecified()) {
                auto *fr = (Int_Requirement *) result[i]->getReq();
                switch(fr->getType()) {
                    case Requirement::GREATER_THAN:
                        if (elements[i].getIntValue() <= fr->getValue()) return false;
                        break;
                    case Requirement::LESS_THAN:
                        if (elements[i].getIntValue() >= fr->getValue()) return false;
                        break;
                    // TODO: case Requirement::EQUAL: ?
                    case Requirement::GREATER_OR_EQUAL:
                        if (elements[i].getIntValue() < fr->getValue()) return false;
                        break;
                    default:
                        if (elements[i].getIntValue() > fr->getValue()) return false;
                        break;
                }
            }
        } else
            return false;
    }
    return true;
}

void Tuple::print() {
    for(int i = 0; i < getSize(); ++i) {
        if (elements[i].getType() == STRING)
            std::cout << elements[i].getStringValue() << " ";
        else if (elements[i].getType() == INT)
            std::cout << elements[i].getIntValue() << " ";
        else
            std::cout << elements[i].getFloatValue() << " ";
    }
    std::cout << std::endl;
}

std::string Tuple::toString() {
    std::string s;
    for(int i = 0; i < getSize(); ++i) {
        if (elements[i].getType() == STRING)
            s += elements[i].getStringValue();
        else if (elements[i].getType() == INT)
            s += std::to_string(elements[i].getIntValue());
        else
            s += std::to_string(elements[i].getFloatValue());
        s += " ";
    }
    return s;
}
