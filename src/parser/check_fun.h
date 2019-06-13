#ifndef CHECK_FUN_H
#define CHECK_FUN_H

#include "..\tuple\Tuple.h"
#include "Parser.h"

bool check(std::string s, Tuple t) {
	Parser p;
	std::vector<Element*> result = p.parse(std::move(s));
	if (t.getSize() != result.size())
		return false;
	for (int i = 0; i < result.size(); i++) {
		if (t[i].getType() == TupleElementType::STRING && result[i]->getType() == Element::Type::STRING) {
			if (!result[i]->isSpecified())
				return true;
			std::string temp(t[i].getStringValue());
			auto *sr = (String_Requirement*) result[i]->getReq();
			if (sr->getType() == Requirement::GREATER_THAN)
				return temp > sr->getValue();
			else if (sr->getType() == Requirement::LESS_THAN)
				return temp < sr->getValue();
			else if (sr->getType() == Requirement::EQUAL)
				return temp == sr->getValue();
			else if (sr->getType() == Requirement::GREATER_OR_EQUAL)
				return temp >= sr->getValue();
			else
				return temp <= sr->getValue();
		}
		else if(t[i].getType() == TupleElementType::INT && result[i]->getType() == Element::INT) {
			if (!result[i]->isSpecified())
				return true;
			auto *ir = (Int_Requirement*) result[i]->getReq();
			if (ir->getType() == Requirement::GREATER_THAN)
				return t[i].getIntValue() > ir->getValue();
			if (ir->getType() == Requirement::LESS_THAN)
				return t[i].getIntValue() < ir->getValue();
			if (ir->getType() == Requirement::EQUAL)
				return t[i].getIntValue() == ir->getValue();
			if (ir->getType() == Requirement::GREATER_OR_EQUAL)
				return t[i].getIntValue() >= ir->getValue();
			else
				return t[i].getIntValue() <= ir->getValue();
		}
		else if(t[i].getType() == TupleElementType::FLOAT && result[i]->getType() == Element::FLOAT) {
			if (!result[i]->isSpecified())
				return true;
			auto *fr = (Float_Requirement*) result[i]->getReq();
			if (fr->getType() == Requirement::GREATER_THAN)
				return t[i].getFloatValue() > fr->getValue();
			if (fr->getType() == Requirement::LESS_THAN)
				return t[i].getFloatValue() < fr->getValue();
			if (fr->getType() == Requirement::EQUAL)
				return t[i].getFloatValue() == fr->getValue();
			if (fr->getType() == Requirement::GREATER_OR_EQUAL)
				return t[i].getFloatValue() >= fr->getValue();
			else
				return t[i].getFloatValue() <= fr->getValue();
		}
		else
			return false;
	}
	return true;
}

#endif