#pragma once
#include "Requirement.h"

// Klasa reprezentujaca jeden element zadanej krotki
// type - typ danej: int, float, string
// specified - czy dodatkowe wymagania co do wartosci zostaly wprowadzone
// 	false : równoznaczne z '*'
// 	true : wprowadzono wymagania np. > 5, =="ret", ...
// req - wskaznik na obiekt reprezentujacy wymagania co do wartosci(jesli specified jest rowne false: req jest rowne nullptr)
class Element
{
public:
	enum Type { INT, FLOAT, STRING };
	Element::Type type;
	bool specified;
	Requirement *req;
	Element(Element::Type t);
	Element(Element::Type t, Requirement::Type t2, int v);
	Element(Element::Type t, Requirement::Type t2, float v);
	Element(Element::Type t, Requirement::Type t2, std::string v);
	~Element();
};

