#include "pch.h"
#include <iostream>
#include "Parser.h"


std::vector<Element*> Parser::parse(std::string s)
{
	std::string main;
	std::vector<Element*> result;

	std::string temp = "";
	bool flag = false;
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == '"')
		{
			flag = !flag;
			temp += s[i];
		}
		else if (flag == true)
		{
			temp += s[i];
		}
		else if (flag == false && !isspace(s[i]))
		{
			temp += s[i];
		}
	}
	s = temp;

	int position = 0;
	main = parse_main(s, position);
	for (int i = 0; i < main.size(); i++)
	{
		if (main[i] == 'i')
			parse_int(s, position, result);
		else if (main[i] == 'f')
			parse_float(s, position, result);
		else if (main[i] == 's')
			parse_string(s, position, result);
	}
	return result;
}

std::string Parser::parse_main(std::string s, int& position)
{
	std::string result = "";
	for (int i = 0; i < s.size(); i++)
	{
		if (s[i] == 'i' || s[i] == 'f' || s[i] == 's')
			result += s[i];
		else if (s[i] == ':')
		{
			position = i + 1;
			return result;
		}
		else
			throw new Parser_Exception("Nieznany znak w glownej czesci");
	}
	throw new Parser_Exception("Nieznaleziono zakonczenia requesta");
}

void Parser::parse_int(std::string s, int &position, std::vector<Element*> &vec)
{
	Requirement::Type type;
	int value;

	if (position < s.size() && s[position] == '*')
	{
		position++;
		if (position < s.size() && s[position] == ';')
		{
			position++;
			vec.push_back(new Element(Element::INT));
			return;
		}
		else
			throw new Parser_Exception("Brak odpowiedniego formatu przy int requirement");
	}
	type = parse_symbol(s, position);

	std::string number = "";
	while (position < s.size() && (s[position] == '-' || isdigit(s[position])))
	{
		number += s[position];
		position++;
	}
	
	try
	{
		value = std::stoi(number.c_str());
	}
	catch (...)
	{
		throw new Parser_Exception("Blad przy probie parsowania int");
	}

	if (position < s.size() && s[position] == ';')
	{
		position++;
		vec.push_back(new Element(Element::INT, type, value));
		return;
	}
	else
		throw new Parser_Exception("Brak odpowiedniego formatu przy int requirement");
}

void Parser::parse_float(std::string s, int &position, std::vector<Element*> &vec)
{
	Requirement::Type type;
	float value;

	if (position < s.size() && s[position] == '*')
	{
		position++;
		if (position < s.size() && s[position] == ';')
		{
			position++;
			vec.push_back(new Element(Element::FLOAT));
			return;
		}
		else
			throw new Parser_Exception("Brak odpowiedniego formatu przy float requirement");
	}
	type = parse_symbol(s, position);

	std::string number = "";
	while (position < s.size() && (s[position] == '-' || s[position] == '.' || isdigit(s[position])))
	{
		number += s[position];
		position++;
	}

	try
	{
		value = std::stof(number.c_str());
	}
	catch (...)
	{
		throw new Parser_Exception("Blad przy probie parsowania float");
	}

	if (position < s.size() && s[position] == ';')
	{
		position++;
		vec.push_back(new Element(Element::FLOAT, type, value));
		return;
	}
	else
		throw new Parser_Exception("Brak odpowiedniego formatu przy float requirement");
}

void Parser::parse_string(std::string s, int &position, std::vector<Element*> &vec)
{
	Requirement::Type type;
	std::string value;

	if (position < s.size() && s[position] == '*')
	{
		position++;
		if (position < s.size() && s[position] == ';')
		{
			position++;
			vec.push_back(new Element(Element::STRING));
			return;
		}
		else
			throw new Parser_Exception("Brak odpowiedniego formatu przy float requirement");
	}
	type = parse_symbol(s, position);

	if (position < s.size() && s[position] == '"')
	{
		position++;
		while (position < s.size() && s[position] != '"')
		{
			value += s[position];
			position++;
		}

		if (position < s.size() && s[position] == '"')
			position++;
		else
			throw new Parser_Exception("Nie znaleziono zakonczenia stringu");

	}
	else
		throw new Parser_Exception("Brak odpowiedniego formatu przy string requirement");

	if (position < s.size() && s[position] == ';')
	{
		position++;
		vec.push_back(new Element(Element::STRING, type, value));
		return;
	}
	else
		throw new Parser_Exception("Brak odpowiedniego formatu przy string requirement");
}

Requirement::Type Parser::parse_symbol(std::string s, int &position)
{
	Requirement::Type type;
	if (position < s.size() && s[position] == '>')
	{
		position++;
		if (position < s.size() && s[position] == '=')
		{
			position++;
			type = Requirement::GREATER_OR_EQUAL;
		}
		else
			type = Requirement::GREATER_THAN;
	}
	else if (position < s.size() && s[position] == '<')
	{
		position++;
		if (position < s.size() && (s[position] == '='))
		{
			position++;
			type = Requirement::LESS_OR_EQUAL;
		}
		else
			type = Requirement::LESS_THAN;
	}
	else if (position < s.size() && s[position] == '=')
	{
		position++;
		if (position < s.size() && s[position] == '=')
		{
			position++;
			type = Requirement::EQUAL;
		}
	}
	else
	{
		throw new Parser_Exception("Brak odpowiedniego formatu przy int requirement");
	}
	return type;
}
