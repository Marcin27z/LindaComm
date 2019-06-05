//
// Created by Marcin on 15.04.2019.
//

#include <iostream>
#include "Thread.h"

void Thread::start() {
    pthread_create(&thread, nullptr, internalThreadEntryFunc, this);
}

void Thread::cancel() {
    pthread_cancel(thread);
}

void Thread::join() {
    pthread_join(thread, nullptr);
}
