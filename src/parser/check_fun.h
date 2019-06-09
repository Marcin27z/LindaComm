#include "..\tuple\Tuple.h"
#include "Parser.h"

bool check(std::string s, Tuple t)
{
	Parser p;
	std::vector<Element*> result = p.parse(s);
	if (t.getSize() != result.size())
		return false;
	for (int i = 0; i < result.size(); ++i)
	{
		if (t[i].getType() == TupleElementType::STRING && result[i]->type == Element::Type::STRING)
		{
			if (!result[i]->specified)
				return true;
			String_Requirement *sr = (String_Requirement*) result[i]->req;
			std::string temp(t[i].getStringValue());
			if (sr->type == Requirement::GREATER_THAN)
			{
				return temp > sr->value;
			}
			if (sr->type == Requirement::LESS_THAN)
			{
				return temp < sr->value;
			}
			if (sr->type == Requirement::EQUAL)
			{
				return temp == sr->value;
			}
			if (sr->type == Requirement::GREATER_OR_EQUAL)
			{
				return temp >= sr->value;
			}
			else
			{
				return temp <= sr->value;
			}
		}
		else if(t[i].getType() == TupleElementType.INT && result[i]->type == Element.INT)
		{
			if (!result[i]->specified)
				return true;
			Int_Requirement *ir = (Int_Requirement*) result[i]->req;
			if (sr->type == Requirement::GREATER_THAN)
			{
				return t[i].getIntValue() > sr->value;
			}
			if (sr->type == Requirement::LESS_THAN)
			{
				return t[i].getIntValue() < sr->value;
			}
			if (sr->type == Requirement::EQUAL)
			{
				return t[i].getIntValue() == sr->value;
			}
			if (sr->type == Requirement::GREATER_OR_EQUAL)
			{
				return t[i].getIntValue() >= sr->value;
			}
			else
			{
				return t[i].getIntValue() <= sr->value;
			}
		}
		else if(t[i].getType() == TupleElementType.FLOAT && result[i]->type == Element.FLOAT)
		{
			if (!result[i]->specified)
				return true;
			Float_Requirement *fr = (Float_Requirement*) result[i]->req;
			if (sr->type == Requirement::GREATER_THAN)
			{
				return t[i].getFloatValue() > sr->value;
			}
			if (sr->type == Requirement::LESS_THAN)
			{
				return t[i].getFloatValue() < sr->value;
			}
			if (sr->type == Requirement::EQUAL)
			{
				return t[i].getFloatValue() == sr->value;
			}
			if (sr->type == Requirement::GREATER_OR_EQUAL)
			{
				return t[i].getFloatValue() >= sr->value;
			}
			else
			{
				return t[i].getFloatValue() <= sr->value;
			}
		}
		else
		{
			return false;
		}
	}
}
