#include <utility>

#include <utility>

//
// Created by rzaro on 01.06.2019.
//

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include "Proces.h"

Proces::Proces(std::string directory_,int mainPipeSize_, int pipeSize_):
                        directory(std::move(directory_)), processId(0), readFd(0), writeFd(0),
                        mainPipePath(std::move(directory + "mainFIFO")),
                        mainPipeSize(mainPipeSize_), pipeSize(pipeSize_)
{

}

void Proces::addRequest(const std::string &request) {
    requests.push_back(request);
}

void Proces::createPipe(int size)
{
    pipePath = "/home/karol/pipe_" + std::to_string(processId);
    mkfifo(pipePath.c_str(), PERM);          // create fifo

    if(size) fcntl(readFd, F_SETPIPE_SZ, size);

}

void Proces::connectToMainPipe() {
    int mainFd = open(mainPipePath.c_str(), O_RDWR);

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
        std::vector<int> structure_vec = std::move(readMainPipe(mainFd));
        // np. jeśli w strukturze były id 0 i 1, obecny proces dostaje id równe .size(), czyli 2
        processId = structure_vec.size();
        structure_vec.push_back(structure_vec.size());

        writeMainPipe(mainFd, structure_vec);
    }

    close(mainFd);
}

int Proces::createMainPipe()
{
    int mainFd;
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

    return mainFd;

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

        int id1 = structure_out.read_int();
        int id2 = structure_out.read_int();

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
    protocol::control_data structure(999);
    structure.write_int(new_structure.size());

    for(int it : new_structure)
    {
        structure.write_int(it);
    }

    structure.send_msg(mainFd);

}

void Proces::disconnect()
{

}



Proces::~Proces()
{
    disconnect();
    unlink(pipePath.c_str());
}

ProcesException::ProcesException(const std::string &msg)  : info("Process Exception: " + msg)
{

}

const char *ProcesException::what() const noexcept {
    return info.c_str();
}


