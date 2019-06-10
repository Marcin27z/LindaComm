#include <utility>
#include <future>

//
// Created by Marcin on 05.06.2019.
//

#include "linda.h"
#include "../thread/SynchronizedQueue.h"
#include "../architecture/Proces.h"

namespace linda {
    Proces proces("/home/karol/");

    void init_linda() {
            proces.start();
            sleep(1);
    }

    void terminate_linda() {
//        proces.disconnect();
        proces.cancel();
        proces.join();
    }

    Tuple read_linda(std::string pattern, int timeout) {
        sleep(1);
        proces.sendRequestTuple(-1, pattern, timeout, LINDA_READ);
        return proces.getTuple(timeout);
    }

    void output_linda(Tuple tuple) {
        proces.put(std::move(tuple));
    }

    Tuple input_linda(std::string pattern, int timeout) {
        proces.sendRequestTuple(-1, pattern, timeout, LINDA_INPUT);
        return proces.getTuple(timeout);
    }

    void display_state(){
        proces.displayRingState();
    }
    void display_tuples(){
        proces.displayTuples();
    }

    void display_requests(){
        proces.displayRequests();
    }

}
