#include <iostream>
#include "Parser.h"

std::vector<Element*> Parser::parse(std::string s) {
	std::string main;
	std::vector<Element*> result;

	std::string temp;
	bool flag = false;
	for(auto i : s) {
		if (i == '"') {
			flag = !flag;
			temp += i;
		}
		else if(flag)
			temp += i;
		else if(!isspace(i))
			temp += i;
	}
	s = temp;

	int position = 0;
	main = parse_main(s, position);
	for(auto i : main) {
		if (i == 'i')
			parse_int(s, position, result);
		else if (i == 'f')
			parse_float(s, position, result);
		else if (i == 's')
			parse_string(s, position, result);
	}
	return result;
}

std::string Parser::parse_main(std::string s, int& position) {
	std::string result;
	for(auto i = 0; i < s.size(); ++i) {
		if(s[i] == 'i' || s[i] == 'f' || s[i] == 's')
			result += s[i];
		else if(s[i] == ':') {
			position = i + 1;
			return result;
		}
		else
			throw Parser_Exception("Nieznany znak w glownej czesci");
	}
	throw Parser_Exception("Nie znaleziono zakonczenia requesta");
}

void Parser::parse_int(std::string s, int &position, std::vector<Element*> &vec) {
	Requirement::Type type;
	int value;

	if(position < s.size() && s[position] == '*') {
		position++;
		if (position < s.size() && s[position] == ';') {
			position++;
			vec.push_back(new Element(Element::INT));
			return;
		}
		else
			throw Parser_Exception("Brak odpowiedniego formatu przy int requirement");
	}
	type = parse_symbol(s, position);

	std::string number;
	for(; position < s.size() && (s[position] == '-' || isdigit(s[position])); position++)
		number += s[position];
	
	try {
		value = std::stoi(number);
	}
	catch(...) {
		throw Parser_Exception("Blad przy probie parsowania int");
	}

	if (position < s.size() && s[position] == ';') {
		position++;
		vec.push_back(new Element(Element::INT, type, value));
		return;
	}
	else
		throw Parser_Exception("Brak odpowiedniego formatu przy int requirement");
}

void Parser::parse_float(std::string s, int &position, std::vector<Element*> &vec) {
	Requirement::Type type;
	float value;

	if(position < s.size() && s[position] == '*') {
		position++;
		if(position < s.size() && s[position] == ';') {
			position++;
			vec.push_back(new Element(Element::FLOAT));
			return;
		}
		else
			throw Parser_Exception("Brak odpowiedniego formatu przy float requirement");
	}
	type = parse_symbol(s, position);

	std::string number;
	for(; position < s.size() && (s[position] == '-' || s[position] == '.' || isdigit(s[position])); position++)
		number += s[position];

	try{
		value = std::stof(number);
	}
	catch(...) {
		throw Parser_Exception("Blad przy probie parsowania float");
	}

	if(position < s.size() && s[position] == ';') {
		position++;
		vec.push_back(new Element(Element::FLOAT, type, value));
		return;
	}
	else
		throw Parser_Exception("Brak odpowiedniego formatu przy float requirement");
}

void Parser::parse_string(std::string s, int &position, std::vector<Element*> &vec) {
	Requirement::Type type;
	std::string value;

	if(position < s.size() && s[position] == '*') {
		position++;
		if(position < s.size() && s[position] == ';') {
			position++;
			vec.push_back(new Element(Element::STRING));
			return;
		}
		else
			throw Parser_Exception("Brak odpowiedniego formatu przy string requirement");
	}
	type = parse_symbol(s, position);

	if(position < s.size() && s[position] == '"') {
		position++;
		for(; position < s.size() && s[position] != '"'; position++)
			value += s[position];

		if(position < s.size() && s[position] == '"')
			position++;
		else
			throw Parser_Exception("Nie znaleziono zakonczenia stringa");

	}
	else
		throw Parser_Exception("Brak odpowiedniego formatu przy string requirement");

	if(position < s.size() && s[position] == ';') {
		position++;
		vec.push_back(new Element(Element::STRING, type, value));
		return;
	}
	else
		throw Parser_Exception("Brak odpowiedniego formatu przy string requirement");
}

Requirement::Type Parser::parse_symbol(std::string s, int &position) {
	if(position < s.size()) {
		switch(s[position]) {
			case '>': {
				position++;
				if(position < s.size() && s[position] == '=') {
					position++;
					return Requirement::GREATER_OR_EQUAL;
				}
				else
					return Requirement::GREATER_THAN;
			}
			case '<': {
				position++;
				if(position < s.size() && s[position] == '=') {
					position++;
					return Requirement::LESS_OR_EQUAL;
				}
				else
					return Requirement::LESS_THAN;
			}
			case '=': {
				position++;
				if(position < s.size() && s[position] == '=') {
					position++;
					return Requirement::EQUAL;
				}
			}
			default:
				throw Parser_Exception("Brak odpowiedniego formatu zadanego symbolu");
		}
	}
	else
		throw Parser_Exception("Pozycja wykracza poza rozmiar sprawdzanego tekstu");
}
