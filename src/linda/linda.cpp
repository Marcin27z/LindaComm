#include <utility>

//
// Created by Marcin on 05.06.2019.
//

#include "linda.h"
#include "../thread/SynchronizedQueue.h"
#include "../architecture/Proces.h"

namespace linda {
    Proces proces("/home/eliot/pipe/");

    void init_linda() {
        proces.start();
    }

    void output_linda(Tuple tuple) {
        proces.put(std::move(tuple));
    }

    Tuple input_linda() {
    }

    Tuple read_linda() {
    }
}
