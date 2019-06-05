//
// Created by Karol Rzepka on 01.06.2019.
//

#ifndef UXP1A_PROCES_H
#define UXP1A_PROCES_H
#include <vector>
#include <iostream>
#include <string>
#include "../protocol/control_data.h"

#define PERM 0777

class Proces {
    int processId;
    int pipeSize;
    int mainPipeSize;

    std::string directory;
    std::string pipePath;
    std::string nextPipePath;
    std::string mainPipePath;
    protocol::manager manager;

    std::vector<std::string> requests;


    void sendRequestConn(int destId, int newId);   // prośba o połączenie się z daną kolejką

    // metody do obsługi zapytań od innych procesów
    void handleRequestTuple(protocol::control_data& request);
    void handleOwnTuple(protocol::control_data& request);
    void handleAcceptTuple(protocol::control_data& request);
    void handleGiveTuple(protocol::control_data& request);
    void handleRequestConn(protocol::control_data request);

    // metoda do otwierania do zapisu kolejek procesów
    int openWrite(int id);

public:
    explicit Proces(std::string directory, int mainPipeSize_ = 0, int pipeSize_ = 0);
    ~Proces();
    void handleRequests();  // pobiera request z kolejki procesu i wywołuje odpowiednią metodę do jego obsługi

    void run();
    void connect();       // podłącza obecny proces do pierścienia
    void disconnect();    // rozłącza obecny proces z pierścienia
    void connectToMainPipe();   // czyta stan pierścienia i aktualizuje go o obecny proces
    int createMainPipe();       // tworzy główną kolejkę
    void createPipe(int size = 0);  // tworzy kolejkę dla obecnego procesu
    std::vector<int> readMainPipe(int mainFd);  // zwraca wektor id procesów obecnych w pierścieniu

    void writeMainPipe(int mainFd, const std::vector<int> &new_structure); // zapisuje nowy wektor id procesów do głównej kolejki
};

class ProcesException : public std::exception
{
    const std::string info;

public:
    explicit ProcesException(const std::string& msg);
    const char* what() const noexcept override;
};


#endif //UXP1A_PROCES_H
