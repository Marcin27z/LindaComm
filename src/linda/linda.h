//
// Created by Fen on 29.05.2019.
//

#ifndef LINDA_H
#define LINDA_H

#include "../protocol/control_data.h"
#include "../tuple/Tuple.h"


namespace linda {

    void init_linda();

    void output_linda(Tuple tuple);

    Tuple input_linda();

    Tuple read_linda();
}

#endif //LINDA_H
