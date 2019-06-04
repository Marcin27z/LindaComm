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

Proces::Proces(int id_, std::string mainPipePath_):id(id_), readFd(0), writeFd(0), mainFd(0),
                                                    mainPipePath(std::move(mainPipePath_))
{

}

void Proces::addRequest(const std::string &request) {
    requests.push_back(request);
}

void Proces::createPipe(int processID)
{
    std::string pipePath = "./pipe" + std::to_string(processID);
    mkfifo(pipePath.c_str(), S_IFIFO);          // create fifo
    readFd = open(pipePath.c_str(), O_RDONLY);    // get fifo's read fd
}

void Proces::connectToMainPipe() {
    mainFd = open(mainPipePath.c_str(), O_RDWR);

    // jeśli nie istnieje, utwórz główną kolejkę
    if(mainFd == -1)
    {
        createMainPipe();
    }


}

void Proces::createMainPipe()
{
    // utwórz główną kolejkę
    if(mknod(mainPipePath.c_str(), S_IFIFO | PERM, 0) < 0) // NOLINT(hicpp-signed-bitwise)
    {
        char cwd[200];
        getcwd(cwd, sizeof(cwd));
        throw ProcesException("creating main fifo failed: " + std::string(cwd) +mainPipePath + ", " + strerror(errno));
    }
    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
    {
        char cwd[200];
        getcwd(cwd, sizeof(cwd));
        throw ProcesException("opening main fifo failed: " + std::string(cwd) +mainPipePath + ", " + strerror(errno));
    }
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

    protocol::control_data structure(5);

    structure.write_int(10); // liczba intów, żeby było wiadomo ile odczytać
    structure.write_int(1);
    structure.write_int(2);
    structure.write_int(3);
    structure.write_int(4);
    structure.write_int(5);
    structure.write_int(6);
    structure.write_int(7);
    structure.write_int(8);
    structure.write_int(9);
    structure.write_int(1337);

    structure.send_msg(mainFd);

    if(manager.assemble(mainFd))
    {
        protocol::control_data structure_out = manager.read_data(mainFd);

        if(structure_out.type != 5) throw ProcesException("incorrect protocol type in main fifo: " +
                                                          std::to_string(structure_out.type)) ;

        int numberOfProcesses = structure_out.read_int();

        std::cout<<"Procesy:"<<std::endl;
        for(int i = 0; i < numberOfProcesses; ++ i)
        {
            int returned = structure_out.read_int();
            std::cout<<structure_out.read_int();
        }
        std::cout<<"Koniec!"<<std::endl;
    }


}


Proces::~Proces()
= default;

ProcesException::ProcesException(const std::string &msg)  : info("Process Exception: " + msg)
{

}

const char *ProcesException::what() const noexcept {
    return info.c_str();
}


