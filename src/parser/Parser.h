#pragma once
#include "Element.h"
#include <vector>
#include <string>

// Parser dla zadan krotek
// Format:
// 1. Czesc glowna zakonczona dwukropkiem zawierajaca informacje o typach zadanej kroki
// 2. Ograniczenia oddzielone srednikiem, moze byc *
// Przyklady:
// iifs: >4; ==7; <-0.45; =="Ala ma kota";
// ffs: *; *; *;
// 
// Ograniczenia: <, >, <=, >=, ==

class Parser
{
public:
	std::vector<Element*> parse(std::string s);
private:
	std::string parse_main(std::string s, int& position);
	void parse_int(std::string s, int &position, std::vector<Element*> &vec);
	void parse_float(std::string s, int &position, std::vector<Element*> &vec);
	void parse_string(std::string s, int &position, std::vector<Element*> &vec);
	Requirement::Type parse_symbol(std::string s, int &position);
};

class Parser_Exception
{
public:
	std::string message;
	Parser_Exception(std::string m) : message(m) {};
};