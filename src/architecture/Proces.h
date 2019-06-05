//
// Created by Karol Rzepka on 01.06.2019.
//

#ifndef UXP1A_PROCES_H
#define UXP1A_PROCES_H
#include <vector>
#include <iostream>
#include <string>
#include "../protocol/control_data.h"
#include "../thread/Thread.h"
#include "../tuple/Tuple.h"
#include "../thread/SynchronizedQueue.h"

#define PERM 0777

class Proces: public Thread {
    int nextId;
    int mainFd;
    int readFd;
    int writeFd;
    int processId;
    int pipeSize;
    int mainPipeSize;

    std::string directory;
    std::string pipePath;
    std::string nextPipePath;
    std::string mainPipePath;
    protocol::manager manager;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool isTupleReady = false;
    bool isWantingTuple = false;

    std::vector<std::string> requests; // TODO: przechowywanie requestów

    SynchronizedQueue<Tuple> outTuplesQueue;
    Tuple tuple = Tuple(0);




    // metody do obsługi zapytań od innych procesów
    void handleRequestTuple(protocol::control_data& request);
    void handleOwnTuple(protocol::control_data& request);
    void handleAcceptTuple(protocol::control_data& request);
    void handleGiveTuple(protocol::control_data& request);
    void handleRequestConn(protocol::control_data request);

    // metody do wysyłania zapytań do innych procesów

    void sendOwnTuple(int destId, int serialNumber);
    void sendAcceptTuple(int destId, int serialNumber);
    void sendGiveTuple(int destId, int serialNumber,  Tuple tuple);
    void sendRequestConn(int destId, int newId);   // prośba o połączenie się z daną kolejką

    // metoda do otwierania do zapisu kolejek procesów
    int openWrite(int id);
    void tupleReady(Tuple tuple);

public:
    explicit Proces(std::string directory, int mainPipeSize_ = 0, int pipeSize_ = 0);
    ~Proces();
    void handleRequests();  // pobiera request z kolejki procesu i wywołuje odpowiednią metodę do jego obsługi
    void sendRequestTuple(int destId, const std::string &tuplePattern);

    void run() override;
    void connect();       // podłącza obecny proces do pierścienia
    void disconnect();    // rozłącza obecny proces z pierścienia
    void connectToMainPipe();   // czyta stan pierścienia i aktualizuje go o obecny proces
    void createMainPipe();       // tworzy główną kolejkę
    void createPipe(int size = 0);  // tworzy kolejkę dla obecnego procesu
    std::vector<int> readMainPipe();  // zwraca wektor id procesów obecnych w pierścieniu
    Tuple findTuple(const std::string& tuplePattern);
    int findRequest(int serialNumber);

    void writeMainPipe(const std::vector<int> &new_structure); // zapisuje nowy wektor id procesów do głównej kolejki
    Tuple readTupleFromRequest(int number, const std::string& tupleType, protocol::control_data &data);
    void put(Tuple);
    void addRequest(const std::string& request);
    Tuple getTuple(int timeout);

};

class ProcesException : public std::exception
{
    const std::string info;

public:
    explicit ProcesException(const std::string& msg);
    const char* what() const noexcept override;
};


#endif //UXP1A_PROCES_H
