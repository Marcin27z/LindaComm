//
// Created by Marcin on 04.04.2019.
//

#ifndef SERVER_SYNCHRONIZEDQUEUE_H
#define SERVER_SYNCHRONIZEDQUEUE_H

#include "monitor.h"
#include <queue>

template<typename T>
class SynchronizedQueue : Monitor {
private:
    std::queue<T> values;
public:
    SynchronizedQueue();

    void put(T);

    T get();

    unsigned long getSize();

    std::queue<T> &getAll();

    Condition isEmpty;
};

template<typename T>
SynchronizedQueue<T>::SynchronizedQueue() {
}

template<typename T>
void SynchronizedQueue<T>::put(T value) {
    enter();
    values.push(std::move(value));
    if (values.size() == 1) {
        signal(isEmpty);
    }
    leave();
}

template<typename T>
T SynchronizedQueue<T>::get() {
    T value;
    enter();
    if (values.size() == 0)
        Monitor::wait(isEmpty);
    value = std::move(values.front());
    values.pop();
    leave();
    return value;
}

template<typename T>
unsigned long SynchronizedQueue<T>::getSize() {
    enter();
    auto size = values.size();
    leave();
    return size;
}

template<typename T>
std::queue<T> &SynchronizedQueue<T>::getAll() {
    enter();
    auto &valuesToReturn = values;
    leave();
    return valuesToReturn;
}

#endif //SERVER_SYNCHRONIZEDQUEUE_H
