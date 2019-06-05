//
// Created by Karol Rzepka on 01.06.2019.
//

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include <cstdlib>
#include "Proces.h"

Proces::Proces(std::string directory_,int mainPipeSize_, int pipeSize_):
                        nextId(-1), mainFd(0), readFd(0), writeFd(0),
                        directory(std::move(directory_)), processId(0),
                        mainPipePath(std::move(directory + "mainFIFO")),
                        mainPipeSize(mainPipeSize_), pipeSize(pipeSize_)
{

}

void Proces::createPipe(int size)
{
    pipePath = directory +"pipe_" + std::to_string(processId);
    if(mkfifo(pipePath.c_str(), PERM) < 0)
    {
        throw ProcesException("creating main fifo failed: " + pipePath + ", " + strerror(errno));
    }
    if((readFd = open(pipePath.c_str(), O_RDWR)) < 0)
    {
        throw ProcesException("opening main fifo failed: " + pipePath + ", " + strerror(errno));
    }

    if(size) fcntl(readFd, F_SETPIPE_SZ, size);

}

void Proces::connectToMainPipe() {
    mainFd = open(mainPipePath.c_str(), O_RDWR);

    // jeśli główna kolejka nie istnieje
    if(mainFd == -1)
    {
        // utwórz główną kolejkę i wstaw do niej strukturę
        createMainPipe();

        // utwórz własną kolejkę
        createPipe();
    }
    // jeśli istnieje
    else
    {
        // odczytaj strukturę i wstaw nową ze swoim id
        std::vector<int> new_structure = std::move(readMainPipe());
        // obecny proces przybiera ID o 1 większe niż ostatni
        processId = new_structure[new_structure.size() - 1] + 1;
        new_structure.push_back(processId);

        // zapisz nową strukturę do głównego fifo
        writeMainPipe(new_structure);

        // obecny proces wysyła wiadomość do ostatniego w głównej kolejce, aby ten się z nim połączył
        if(processId > 0)
        {
            // utwórz własną kolejkę
            createPipe();

            sendRequestConn(processId - 1, processId); // ostatni ma  id o 1 mniejsze od obecnego procesu

            nextPipePath = directory + "pipe_" + std::to_string(processId - 1);
            nextId = processId - 1;
        }
    }
    writeFd = open(nextPipePath.c_str(), O_RDWR);
}

void Proces::createMainPipe()
{
    // utwórz główną kolejkę
    if(mkfifo(mainPipePath.c_str(), PERM) < 0)
    {
        throw ProcesException("creating main fifo failed: " + mainPipePath + ", " + strerror(errno));
    }
    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
    {
        throw ProcesException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno));
    }

    if(mainPipeSize) fcntl(mainFd, F_SETPIPE_SZ, mainPipeSize); // jeśli 0, nie zmieniaj rozmiaru

    processId = 0;
    writeMainPipe(std::vector<int>(1,0));

}

void Proces::connect() {
    try {
        connectToMainPipe();
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return;
    }
}

std::vector<int> Proces::readMainPipe() {
    std::vector<int> structure_vec;

    if(manager.assemble(mainFd))
    {
        protocol::control_data structure_out = manager.read_data(mainFd);

        int numberOfProcesses = structure_out.read_int();

        std::cout<<"Procesy:"<<std::endl;
        for(int i = 0; i < numberOfProcesses; ++ i)
        {
            int returned = structure_out.read_int();
            std::cout<<returned<<std::endl;
            structure_vec.push_back(returned);
        }
        std::cout<<"Koniec!"<<std::endl;
    }
    return structure_vec;
}

void Proces::writeMainPipe(const std::vector<int>& new_structure) {
    int x;
    if(processId == 0) x = 998;
    else if(processId == 1) x = 997;
    else x = 996;
    protocol::control_data structure(x);
    structure.write_int(new_structure.size());

    for(int it : new_structure)
    {
        structure.write_int(it);
    }
    structure.send_msg(mainFd);
}


void Proces::disconnect()
{
    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
    {
        throw ProcesException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno) + ", cannot disconnect");
    }
    std::vector<int> structure = readMainPipe();

    // jeśli obecny proces jest jedynym, należy wysadzić główną kolejkę w powietrze
    if(structure.size() == 1)
    {
        unlink(mainPipePath.c_str());
    }
    else if(structure.size() > 1)
    {

        unlink(pipePath.c_str());

        auto it = std::find(structure.begin(), structure.end(), processId);
        int index = std::distance(structure.begin(), it);

        // jeśli obecny proces jest pierwszym w strukturze, należy wysłać wiadomość do ostatniego o podłączeniu się do drugiego
        if(index == 0)
        {
            sendRequestConn(structure[structure.size() - 1], structure[1]);
        }
        // jeśli obecny proces jest ostatnim w strukturze, należy wysłać wiadomość do przedostatniego o podłączeniu się do pierwszego
        else if(index == structure.size() - 1)
        {
            sendRequestConn(structure[index - 1], structure[0]);
        }
        else
        {
            sendRequestConn(structure[index - 1], structure[index + 1]);
        }

        // uaktualnij strukturę w głównej kolejce
        structure.erase(it);
        writeMainPipe(structure);
    }

}

Proces::~Proces()
{
    disconnect();
    unlink(pipePath.c_str());
}

void Proces::sendRequestConn(int destId, int newId){
    // otwórz dolecową kolejkę
    std::string destPipePath = directory + "pipe_" + std::to_string(destId);
    int destFd = open(destPipePath.c_str(), O_RDWR);


    protocol::control_data msg(4);
    msg.write_int(newId);
    msg.send_msg(destFd);

    //close(destFd);
}

void Proces::run() {
    connect();

    while (true) {
        handleRequests();
    }

}

void Proces::handleRequests() {
    if(manager.assemble(readFd)) {
        protocol::control_data request = manager.read_data(readFd);

        switch (request.type) {
            case 0:
                handleRequestTuple(request);
            case 1:
                handleOwnTuple(request);
            case 2:
                handleAcceptTuple(request);
            case 3:
                handleGiveTuple(request);
            case 4:
                handleRequestConn(request);
        }
    }
}

void Proces::handleRequestTuple(protocol::control_data& request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    (void) serialNumber;

    int tuplePatternSize = request.read_int();
    std::string tuplePattern = request.read_string(tuplePatternSize);

    findTuple(tuplePattern);
}

void Proces::handleOwnTuple(protocol::control_data& request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

    if(findRequest(serialNumber) >= 0)  // jeśli żądanie jest nadal aktualne
        sendAcceptTuple(request.id_sender, serialNumber);     // wysłanie informacji do procesu, że nadal chcemy tę krotkę
}

void Proces::handleAcceptTuple(protocol::control_data& request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

    if(findRequest(serialNumber) >= 0)  // szukamy krotki
        sendGiveTuple(nextId, serialNumber, Tuple(nullptr));     // wysłanie informacji do procesu, że nadal chcemy tę krotkę
}

void Proces::handleGiveTuple(protocol::control_data& request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    int tupleTypeSize = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    std::string tupleType = request.read_string(tupleTypeSize); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

    Tuple tuple = readTupleFromRequest(serialNumber, tupleType, request);

    //TODO: tutaj sprawdzenie czy chcieliśmy tę krotkę?
    // usunięcie krotki z requestów,
    // wypisanie krotki użytkownikowi?
}

void Proces::handleRequestConn(protocol::control_data request) {
    int destId = request.read_int();
    close(writeFd); // TODO: czy tutaj close(writefd) czegoś nie zapsuje?
    writeFd = openWrite(destId);
}

int Proces::openWrite(int id)
{
    // zmień ścieżkę na zadane fifo
    std::string path = directory + "pipe_" + std::to_string(id);
    int fd = open(path.c_str(), O_RDWR);

    return fd;
}

void Proces::put(Tuple tuple) {
    outTuplesQueue.put(tuple);
}

Tuple Proces::getTuple() {
//    return intTuplesQueue.get();
}

Tuple Proces::findTuple(const std::string& tuplePattern) {
 /*   for(Tuple tuple : tuples) {
        if (tuple.matchPattern(tuplePattern)) {
            return tuple;
        }
    }*/
    return Tuple(nullptr);
}

void Proces::sendRequestTuple(int destId, const std::string& tuplePattern) {
    protocol::control_data request(0);
    request.id_sender = processId;
    request.id_recipient = destId;

    int serialNumber = rand();
    request.write_int(tuplePattern.size());
    request.write_string(tuplePattern);

    request.send_msg(writeFd);

    addRequest(std::string("")); // TODO
}

void Proces::sendOwnTuple(int destId, int serialNumber) {
    protocol::control_data request(1);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    request.send_msg(writeFd);
}

void Proces::sendAcceptTuple(int destId, int serialNumber) {
    protocol::control_data request(2);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    request.send_msg(writeFd);
}

void Proces::sendGiveTuple(int destId, int serialNumber, Tuple tuple) {
    protocol::control_data request(2);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    // TODO: typ krotki i krotka
    request.send_msg(writeFd);
}

int Proces::findRequest(int serialNumber) {
    // TODO: jak przechowywać requesty i je znajdywać?
    return -1;
}

Tuple Proces::readTupleFromRequest(int number, const std::string& tupleType, protocol::control_data &data) {
    return Tuple(nullptr);
}

void Proces::addRequest(const std::string& request) {

}

ProcesException::ProcesException(const std::string &msg)  : info("Process Exception: " + msg)
{

}

const char *ProcesException::what() const noexcept {
    return info.c_str();
}


