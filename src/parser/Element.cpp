#include "pch.h"
#include "Element.h"


Element::Element(Element::Type t)
{
	type = t;
	specified = false;
	req = nullptr;
}


Element::Element(Element::Type t, Requirement::Type t2, int v)
{
	type = t;
	specified = true;
	req = new Int_Requirement(t2, v);
}


Element::Element(Element::Type t, Requirement::Type t2, float v)
{
	type = t;
	specified = true;
	req = new Float_Requirement(t2, v);
}


Element::Element(Element::Type t, Requirement::Type t2, std::string v)
{
	type = t;
	specified = true;
	req = new String_Requirement(t2, v);
}


Element::~Element()
{
	if (specified)
		delete req;
}
