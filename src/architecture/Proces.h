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
    int processId;
    int readFd;
    int writeFd;
    int pipeSize{};
    std::string directory;
    std::string pipePath{};
    std::vector<std::string> requests;
    std::string mainPipePath;
    int mainPipeSize;
    protocol::manager manager;

    void addRequest(const std::string &request);


public:
    explicit Proces(std::string directory, int mainPipeSize_ = 0, int pipeSize_ = 0);
    ~Proces();

    void connect();
    void disconnect();
    void connectToMainPipe();
    int createMainPipe();
    void createPipe(int size = 0);
    std::vector<int> readMainPipe(int mainFd);

    static void writeMainPipe(int mainFd, const std::vector<int> &new_structure);
};

class ProcesException : public std::exception
{
    const std::string info;

public:
    explicit ProcesException(const std::string& msg);
    const char* what() const noexcept override;
};


#endif //UXP1A_PROCES_H
