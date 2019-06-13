#ifndef REQUIREMENT_H
#define REQUIREMENT_H

#include <string>

// Klasa reprezentujaca ograniczenia dotyczace skladnika krotki
class Requirement {
public:
	enum Type {LESS_THAN, GREATER_THAN, EQUAL, LESS_OR_EQUAL, GREATER_OR_EQUAL};

private:
	Requirement::Type type;

public:
	explicit Requirement(Requirement::Type t) : type(t) {};
	const Requirement::Type getType() const { return type; }
};

class Int_Requirement : public Requirement {
private:
	int value;

public:
	Int_Requirement(Requirement::Type t, int v) : Requirement(t), value(v) {};
	const int getValue() const { return value; }
};

class Float_Requirement : public Requirement {
private:
	float value;

public:
	Float_Requirement(Requirement::Type t, float f) : Requirement(t), value(f) {};
	const float getValue() const { return value; }
};

class String_Requirement : public Requirement {
private:
	std::string value;

public:
	String_Requirement(Requirement::Type t, std::string s) : Requirement(t), value(std::move(s)) {};
	std::string getValue() { return value; }
};

#endif