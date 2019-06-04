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
    std::vector<std::string> requests;
    std::string mainPipePath;
    protocol::manager manager;

    void addRequest(const std::string &request);


public:
    explicit Proces(int id, std::string mainPipePath);
    ~Proces();

    void connect();
    void connectToMainPipe();
    void createMainPipe();
    void createPipe(int processID);

};

class ProcesException : public std::exception
{
    const std::string info;

public:
    explicit ProcesException(const std::string& msg);
    const char* what() const noexcept override;
};


#endif //UXP1A_PROCES_H
