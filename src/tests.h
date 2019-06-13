#ifndef TESTS
#define TESTS
#include "linda/linda.h"
#include "tuple/Tuple.h"
#include <unistd.h>
#include <iostream>

// Tworzy dwa procesy, z których jeden wysy³a krotki, a drugi je odbiera. Typy s¹ ze sob¹ zgodne, oraz nie ma
// dodatkowych ograniczeñ
void test1()
{
	std::cout << "\nTEST1" << std::endl;
	if (fork() == 0)
	{
		linda::init_linda();
		for (int i = 0; i < 100; i++)
		{
			linda::output_linda(new Tuple(i, "krotka" + i));
		}
		linda::terminate_linda();
	}
	else
	{
		linda::init_linda();
		int counter = 0;
		for (int i = 0; i < 100; i++)
		{
			Tuple t = linda::input_linda("is:*;*;", 1000);
			std::cout << "Krotka " << t.print() << std::endl;
			counter++;
		}
		std::cout << "Odebrano: " << counter << "krotek" << std::endl;
		linda::terminate_linda();
	}
}

// Tworzymy dwa procesy, z czego jeden wysy³a 3 krotki, a drugi je odbiera. Krotki zosta³y tak stworzone, by
// wszystkie wstawiane krotki odpowiadaly ¿¹daniom
void test2()
{
	std::cout << "\nTEST2" << std::endl;
	if (fork() == 0)
	{
		linda::init_linda();
		linda::output_linda(new Tuple(5, "Test"));
		linda::output_linda(new Tuple(7.9, "Test", 9));
		linda::output_linda(new Tuple(0));
		linda::terminate_linda();
	}
	else
	{
		linda::init_linda();
		Tuple t = linda::input_linda("is:>0;=="Test";", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		Tuple t = linda::input_linda("fsi:>0.0;=="Test";==9", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		Tuple t = linda::input_linda("i:<=0;", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		linda::terminate_linda();
	}
}

// Tworzymy dwa procesy, z czego jeden wysy³a 3 krotki, a drugi je odbiera. Krotki zosta³y tak stworzone, by
// krotki nie odpowiadaly ¿¹daniom, czy to z powodu wartoœci, czy typu
void test3()
{
	std::cout << "\nTEST3" << std::endl;
	if (fork() == 0)
	{
		linda::init_linda();
		linda::output_linda(new Tuple(5, "Test"));
		linda::output_linda(new Tuple(7.9, "Test", 9));
		linda::output_linda(new Tuple(0));
		linda::terminate_linda();
	}
	else
	{
		linda::init_linda();
		Tuple t = linda::input_linda("is:<0;=="Test";", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		Tuple t = linda::input_linda("fsi:>0.0;=="Ala ma kota";==9", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		Tuple t = linda::input_linda("ii:<=0;== -9", 1000);
		std::cout << "Krotka " << t.print() << std::endl;
		linda::terminate_linda();
	}
}

// Test ten ma na celu sprawdzenie co siê dzieje w przypadku, gdy wype³niamy przesprzeñ krotkami, ale zadnej nie
// wyciagamy. Jak oprogramowanie poradzi sobie z przeci¹¿eniem.
void test4(int nrTuples)
{
	linda::init_linda();
	std::cout << "\nTEST4" << std::endl;
	for (int i = 0; i < nrTuples; i++)
	{
		counter++;
		linda::output_linda(new Tuple(i, "krotka" + i));
		std::cout << "Krotka numer " +  i + " wstawiona" << std::endl;
	}
	linda::terminate_linda();
}

// Wykonywane testy
void tests()
{
	test1();
	test2();
	test3();
	test4(10000);
}
#endif
