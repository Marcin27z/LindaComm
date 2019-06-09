#pragma once
#include <string>

// Klasa reprezentujaca ograniczenia dotyczace skladnika krotki
class Requirement
{
public:
	enum Type {LESS_THAN, GREATER_THAN, EQUAL, LESS_OR_EQUAL, GREATER_OR_EQUAL};
	Requirement::Type type;
	Requirement(Requirement::Type t) : type(t) {};
};

class Int_Requirement : public Requirement
{
public:
	Int_Requirement(Requirement::Type t, int v) : Requirement(t), value(v) {};
	int value;
};

class Float_Requirement : public Requirement
{
public:
	Float_Requirement(Requirement::Type t, float f) : Requirement(t), value(f) {};
	float value;
};

class String_Requirement : public Requirement
{
public:
	String_Requirement(Requirement::Type t, std::string s) : Requirement(t), value(s) {};
	std::string value;
};

