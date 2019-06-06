#include <utility>
#include <future>

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

    void terminate_linda() {
        proces.join();
    }

    void output_linda(Tuple tuple) {
        proces.put(std::move(tuple));
    }

    Tuple input_linda(std::string pattern, int timeout) {
        sleep(1);
        proces.sendRequestTuple(-1, pattern);
        return proces.getTuple(timeout);
    }

    Tuple read_linda(std::string pattern, int timeout) {
    }

    void display_requests(){
        proces.displayRequests();
    }

}
