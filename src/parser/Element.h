#ifndef ELEMENT_H
#define ELEMENT_H

#include "Requirement.h"

// Klasa reprezentujaca jeden element zadanej krotki
// type - typ danej: int, float, string
// specified - czy dodatkowe wymagania co do wartosci zostaly wprowadzone
// 	false : równoznaczne z '*'
// 	true : wprowadzono wymagania np. > 5, =="ret", ...
// req - wskaznik na obiekt reprezentujacy wymagania co do wartosci(jesli specified jest rowne false: req jest rowne nullptr)

class Element {
public:
	enum Type { INT, FLOAT, STRING };

private:
	Element::Type type;
	bool specified;
	Requirement *req;

public:
	explicit Element(Element::Type t) : type(t), specified(false), req(nullptr) {};
	Element(Element::Type t, Requirement::Type t2, int v) : type(t), specified(true), req(new Int_Requirement(t2, v)) {};
	Element(Element::Type t, Requirement::Type t2, float v) : type(t), specified(true), req(new Float_Requirement(t2, v)) {};
	Element(Element::Type t, Requirement::Type t2, std::string v) : type(t), specified(true), req(new String_Requirement(t2, std::move(v))) {};
	~Element() { if (specified) delete req; };

	const Element::Type getType() const { return type; }
	const bool isSpecified() const { return specified; }
	const Requirement* getReq() const { return req; }
};

#endif