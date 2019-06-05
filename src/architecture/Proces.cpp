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
#include "Proces.h"

Proces::Proces(std::string directory_,int mainPipeSize_, int pipeSize_):
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
        std::vector<int> new_structure = std::move(readMainPipe(mainFd));
        // obecny proces przybiera ID o 1 większe niż ostatni
        processId = new_structure[new_structure.size() - 1] + 1;
        new_structure.push_back(processId);

        // zapisz nową strukturę do głównego fifo
        writeMainPipe(mainFd, new_structure);

        // obecny proces wysyła wiadomość do ostatniego w głównej kolejce, aby ten się z nim połączył
        if(processId > 0)
        {
            // utwórz własną kolejkę
            createPipe();

            sendRequestConn(processId - 1, processId); // ostatni ma  id o 1 mniejsze od obecnego procesu

            nextPipePath = directory + "pipe_" + std::to_string(processId - 1);
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
    writeMainPipe(mainFd, std::vector<int>(1,0));

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

std::vector<int> Proces::readMainPipe(int mainFd) {
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

void Proces::writeMainPipe(int mainFd, const std::vector<int>& new_structure) {
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
    int mainFd;
    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
    {
        throw ProcesException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno) + ", cannot disconnect");
    }
    std::vector<int> structure = readMainPipe(mainFd);

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
        writeMainPipe(mainFd, structure);
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

    //TODO run while()

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
    //TODO: handleRequestTuple
}

void Proces::handleOwnTuple(protocol::control_data& request) {
    //TODO: handleOwnTuple
}

void Proces::handleAcceptTuple(protocol::control_data& request) {
    //TODO: handleAcceptTuple
}

void Proces::handleGiveTuple(protocol::control_data& request) {
    //TODO: handleGiveTuple
}

void Proces::handleRequestConn(protocol::control_data request) {
    int destId = request.read_int();
    close(writeFd); // TODO: czy tutaj close(writefd) czegoś nie zapsuje?
    writeFd = openWrite(destId);
}

int Proces::openWrite(int id) {

    // zmień ścieżkę na zadane fifo
    std::string path = directory + "pipe_" + std::to_string(id);
    int fd = open(path.c_str(), O_RDWR);

    return fd;
}



ProcesException::ProcesException(const std::string &msg)  : info("Process Exception: " + msg)
{

}

const char *ProcesException::what() const noexcept {
    return info.c_str();
}


