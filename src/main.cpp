#include <iostream>
#include <unistd.h>
#include "tuple/Tuple.h"
#include "architecture/Proces.h"

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

void example_ring(){
    std::string nameFIFO = "/home/karol/mainFIFO";
    unlink(nameFIFO.c_str());  // Add before mknod()
    Proces proces(getpid(), nameFIFO);
    proces.connect();
}

int main() {
    //example("hello", 3, 2, 15.0f, "world");
    example_ring();
    return 0;
}
