//
// Created by rzaro on 01.06.2019.
//

#ifndef UXP1A_PROCES_H
#define UXP1A_PROCES_H
#include <vector>
#include <iostream>
#include <string>
#include "../protocol/control_data.h"

#define PERM 0777

class Proces {
    int id;
    int readFd;
    int writeFd;
    int mainFd;
    int pipeSize;
    std::vector<std::string> requests;
    std::string mainPipePath;
    int mainPipeSize;
    protocol::manager manager;

    void addRequest(const std::string &request);


public:
    explicit Proces(int id, std::string mainPipePath, int mainPipeSize_ = 0, int pipeSize_ = 0);
    ~Proces();

    void connect();
    void connectToMainPipe();
    void createMainPipe();
    void createPipe(int processID, int size);

};

class ProcesException : public std::exception
{
    const std::string info;

public:
    explicit ProcesException(const std::string& msg);
    const char* what() const noexcept override;
};


#endif //UXP1A_PROCES_H
