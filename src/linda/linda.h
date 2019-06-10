//
// Created by Fen on 29.05.2019.
//

#ifndef LINDA_H
#define LINDA_H

#include "../protocol/control_data.h"
#include "../tuple/Tuple.h"
#include "../architecture/Proces.h"

#define LINDA_READ 1
#define LINDA_INPUT 0

namespace linda {

    void init_linda();

    void terminate_linda();

    void output_linda(Tuple tuple);

    Tuple input_linda(std::string pattern, int timeout);

    Tuple read_linda(std::string pattern, int timeout);

    void display_requests();
    void display_state();
    void display_tuples();
    void disconnect();
}

#endif //LINDA_H
