#include <utility>

#include <iostream>
#include <unistd.h>
#include <thread>
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

int main() {
    linda::init_linda();
    int i=1;

    while(i) {
        std::cin >> i;
        if (i == 2) {
            std::cout << "trying to get tuple" << std::endl;
            Tuple tuple = linda::input_linda("iii: ==1;==2;==3;", 15);
            std::cout << "linda_input zwrocila";
            tuple.print();
            std::cout << "finished" << std::endl;
        } else if (i == 1) {
            linda::output_linda(Tuple(1, 2, 3));
        } else if (i == 3) {
            linda::display_requests();
        }
        else if(i==4){
            linda::display_state();
        }
        else if(i==5){
            linda::disconnect();
            break;
        }
    }

    linda::terminate_linda();

    return 0;
}
