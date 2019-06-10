#include <utility>

#include <iostream>
#include <unistd.h>
#include <thread>
#include <sstream>
#include "tuple/Tuple.h"
#include "architecture/Proces.h"
#include "linda/linda.h"

template<typename... Args>
void example(Args... args) {
    Tuple tuple(args...);
    for (int i = 0; i < tuple.getSize(); ++i) {
        if (tuple[i].getType() == STRING) {
            std::cout << tuple[i].getStringValue() << std::endl;
        } else if (tuple[i].getType() == INT) {
            std::cout << tuple[i].getIntValue() << std::endl;
        } else {
            std::cout << tuple[i].getFloatValue() << std::endl;
        }
    }
    std::cout << tuple.getType() << std::endl;
}

void one_process(std::string directory) {
    Proces process(std::move(directory));
    process.connect();

}

void example_ring() {
    std::string directory = "/home/karol/";
    unlink((directory + "mainFIFO").c_str());

    one_process(directory);
}

void parserTest() {
    Tuple tuple("ala ma kota", 12, 5.0f);
    std::cout << (tuple.matchPattern("sif: *; >10; *;") ? "true" : "false") << std::endl;
}

std::vector<std::string> splitBySpace(const std::string &input)
{
    std::string buf;
    std::stringstream ss(input);

    std::vector<std::string> tokens;

    while (ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

int main() {
    linda::init_linda();
    int i=1;
    std::string input;
    std::vector<std::string> tokens;

    while(input != "exit") {
        getline(std::cin, input);

        tokens = splitBySpace(input);

        if (tokens[0] == "input") {
            if (tokens.size() < 2) {
                std::cout << "Incorrect command" << std::endl;
            }
            std::cout << "trying to get a tuple..." << std::endl;
            Tuple tuple = linda::input_linda(tokens[1], 30);
        } else if (tokens[0] == "read") {
            if(tokens.size() < 2) {
                std::cout<<"Incorrect command"<<std::endl;
            }
            std::cout << "trying to get a tuple..." << std::endl;
            Tuple tuple = linda::read_linda(tokens[1], 30);
        } else if (tokens[0] == "output") {
            if(tokens[1] == "1")
                linda::output_linda(Tuple(1, 2, 3));
            else if(tokens[1] == "2")
                linda::output_linda(Tuple(1.01f, 13.37f, 9.997f));
            else if(tokens[1] == "3")
                linda::output_linda(Tuple("tuple", 5, 3.01));
            else std::cout<<"Incorrect command"<<std::endl;
        } else if (input == "display_requests") {
            linda::display_requests();
        }
        else if(input == "display_state"){
            linda::display_state();
        }
        else if(input == "display_tuples"){
            linda::display_tuples();
        }
        else if(input == "disconnect"){
            break;
        }
        else std::cout<<"Incorrect command"<<std::endl;
    }

    linda::terminate_linda();

    return 0;
}
