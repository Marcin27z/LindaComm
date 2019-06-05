//
// Created by Fen on 29.05.2019.
//

#ifndef LINDA_H
#define LINDA_H

#include "../protocol/control_data.h"
#include "../tuple/Tuple.h"
#include "../architecture/Proces.h"


namespace linda {

    void init_linda();

    void terminate_linda();

    void output_linda(Tuple tuple);

    Tuple input_linda(std::string pattern, int timeout);

    Tuple read_linda(std::string pattern, int timeout);
}

#endif //LINDA_H
