//
// Created by Marcin on 15.04.2019.
//

#ifndef SERVER_THREAD_H
#define SERVER_THREAD_H

#include <pthread.h>

class Thread {
private:
    pthread_t thread;

    static void *internalThreadEntryFunc(void *This) {
        ((Thread *) This)->run();
        return nullptr;
    }

protected:
    virtual void run() = 0;

public:
    void start();

    void cancel();

    void join();

};


#endif //SERVER_THREAD_H
