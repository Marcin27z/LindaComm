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
#include <chrono>
#include "Proces.h"

Proces::Proces(std::string directory_, int mainPipeSize_, int pipeSize_) :
        nextId(-1), mainFd(0), readFd(0), writeFd(0),
        directory(std::move(directory_)), processId(0),
        mainPipePath(std::move(directory + "mainFIFO")),
        mainPipeSize(mainPipeSize_), pipeSize(pipeSize_) {
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
}

void Proces::createPipe(int size) {
    pipePath = directory + "pipe_" + std::to_string(processId);
    if (mkfifo(pipePath.c_str(), PERM) < 0) {
        throw ProcesException("creating pipe failed: " + pipePath + ", " + strerror(errno));
    }
    if ((readFd = open(pipePath.c_str(), O_RDWR)) < 0) {
        throw ProcesException("opening pipe failed: " + pipePath + ", " + strerror(errno));
    }

    if (size) fcntl(readFd, F_SETPIPE_SZ, size);

}

void Proces::connectToMainPipe() {
    mainFd = open(mainPipePath.c_str(), O_RDWR);

    // jeśli główna kolejka nie istnieje
    if (mainFd == -1) {
        // utwórz główną kolejkę i wstaw do niej strukturę
        createMainPipe();

        // utwórz własną kolejkę
        createPipe();
    }
        // jeśli istnieje
    else {
        // odczytaj strukturę i wstaw nową ze swoim id
        std::vector<int> new_structure = std::move(readMainPipe());
        // obecny proces przybiera ID o 1 większe niż ostatni
        processId = new_structure[new_structure.size() - 1] + 1;
        new_structure.push_back(processId);

        // zapisz nową strukturę do głównego fifo
        writeMainPipe(new_structure);

        // obecny proces wysyła wiadomość do ostatniego w głównej kolejce, aby ten się z nim połączył
        if (processId > 0) {
            // utwórz własną kolejkę
            createPipe();

            sendRequestConn(*new_structure.begin(), processId);

            nextPipePath = directory + "pipe_" + std::to_string(*(new_structure.end() - 2));
            nextId = *(new_structure.end() - 2)
                    ;
        }
    }
    writeFd = open(nextPipePath.c_str(), O_RDWR);
}

void Proces::createMainPipe() {
    // utwórz główną kolejkę
    if (mkfifo(mainPipePath.c_str(), PERM) < 0) {
        throw ProcesException("creating main fifo failed: " + mainPipePath + ", " + strerror(errno));
    }
    if ((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0) {
        throw ProcesException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno));
    }

    if (mainPipeSize) fcntl(mainFd, F_SETPIPE_SZ, mainPipeSize); // jeśli 0, nie zmieniaj rozmiaru

    processId = 0;
    writeMainPipe(std::vector<int>(1, 0));

}

void Proces::connect() {
    try {
        connectToMainPipe();
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return;
    }
}

std::vector<int> Proces::readMainPipe() {
    std::vector<int> structure_vec;

    if (manager.assemble(mainFd)) {
        protocol::control_data structure_out = manager.read_data(mainFd);

        int numberOfProcesses = structure_out.read_int();

//        std::cout << "Procesy:" << std::endl;
        for (int i = 0; i < numberOfProcesses; ++i) {
            int returned = structure_out.read_int();
//            std::cout << returned << std::endl;
            structure_vec.push_back(returned);
        }
//        std::cout << "Koniec!" << std::endl;
    }
    return structure_vec;
}

void Proces::writeMainPipe(const std::vector<int> &new_structure) {
    protocol::control_data structure(999);
    structure.write_int(new_structure.size());

    for (int it : new_structure) {
        structure.write_int(it);
    }
    structure.send_msg(mainFd);
}


void Proces::disconnect() {
    std::cout<<"Disconnecting! "<<pipePath<<std::endl;

    if ((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0) {
        throw ProcesException(
                "opening main fifo failed: " + mainPipePath + ", " + strerror(errno) + ", cannot disconnect");
    }
    std::vector<int> structure = readMainPipe();

    // jeśli obecny proces jest jedynym, należy wysadzić główną kolejkę w powietrze
    if (structure.size() == 1) {
        unlink(mainPipePath.c_str());
    } else if (structure.size() > 1) {

        std::cout<<"Unlinking "<<pipePath<<std::endl;
        unlink(pipePath.c_str());

        auto it = std::find(structure.begin(), structure.end(), processId);
        int index = std::distance(structure.begin(), it);

        // jeśli obecny proces jest pierwszym w strukturze, należy wysłać wiadomość do ostatniego o podłączeniu się do drugiego
        if (index == 0) {
            std::cout<<"Send "<<structure[1]<<" to connect with "<<structure[structure.size() - 1]<<std::endl;

            sendRequestConn(structure[1], structure[structure.size() - 1]);
        }
            // jeśli obecny proces jest ostatnim w strukturze, należy wysłać wiadomość do przedostatniego o podłączeniu się do pierwszego
        else if (index == structure.size() - 1) {
            std::cout<<"Send "<<structure[0]<<" to connect with "<<structure[index - 1]<<std::endl;

            sendRequestConn(structure[0],structure[index - 1]);
        } else {
            std::cout<<"Send "<<structure[index + 1]<<" to connect with "<<structure[index - 1]<<std::endl;

            sendRequestConn(structure[index + 1], structure[index - 1]);
        }

        // uaktualnij strukturę w głównej kolejce
        structure.erase(it);
        writeMainPipe(structure);

        close(writeFd);
        close(readFd);
        close(mainFd);
    }

}

Proces::~Proces() {
    disconnect();
    unlink(pipePath.c_str());
}

void Proces::sendRequestConn(int destId, int newId) {
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
    std::cout << "id: " << processId << std::endl;
    while (!quitFlag) {
        handleRequests();
    }

}

void Proces::handleRequests() {
    if (manager.assemble(readFd)) {
        protocol::control_data request = manager.read_data(readFd);

        switch (request.type) {
            case 0:
                handleRequestTuple(request);
                break;
            case 1:
                handleOwnTuple(request);
                break;
            case 2:
                handleAcceptTuple(request);
                break;
            case 3:
                handleGiveTuple(request);
                break;
            case 4:
                handleRequestConn(request);
                break;
            default:
                quitFlag = true;
                break;
        }
    }
}

void Proces::handleRequestTuple(protocol::control_data &request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    (void) serialNumber;

    int tuplePatternSize = request.read_int();
    std::string tuplePattern = request.read_string(tuplePatternSize);
    std::cout << tuplePattern << std::endl;
    auto trove = findTuple(tuplePattern);
    if (trove.first) {
        addRequest(tuplePattern, request.id_sender, serialNumber, request.expirationDate);
        sendOwnTuple(request.id_sender, serialNumber);
        std::cout << "found matching tuple ";
        trove.second.print();
    } else if (request.id_sender != processId) {
        addRequest(tuplePattern, request.id_sender, serialNumber, request.expirationDate);
        request.write_int(serialNumber);
        request.write_int(tuplePatternSize);
        request.write_string(tuplePattern);
        forwardMessage(request);
    }
}

void Proces::handleOwnTuple(protocol::control_data &request) {
    if (request.id_recipient == processId) {
        int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
        if (findRequest(serialNumber))  // jeśli żądanie jest nadal aktualne
            sendAcceptTuple(request.id_sender,
                            serialNumber);     // wysłanie informacji do procesu, że nadal chcemy tę krotkę
    } else {
        forwardMessage(request);
    }
}

void Proces::handleAcceptTuple(protocol::control_data &request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    if (request.id_recipient == processId) {
        if (findRequest(serialNumber)) {
            auto foundRequest = getRequest(serialNumber);
            auto tuple = findTuple(foundRequest.first).second;
            sendGiveTuple(request.id_sender, serialNumber,
                          tuple);
            std::cout<<"Tuple sent"<<std::endl;
            removeRequest(serialNumber);
        }
    } else {
        request.write_int(serialNumber);
        forwardMessage(request);
    }
}

void Proces::handleGiveTuple(protocol::control_data &request) {
    if (request.id_recipient == processId) {
        int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
        int tupleTypeSize = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
        std::string tupleType = request.read_string(
                tupleTypeSize); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

        Tuple tuple = readTupleFromRequest(serialNumber, tupleType, request);

        //TODO: tutaj sprawdzenie czy chcieliśmy tę krotkę?
        // usunięcie krotki z requestów,
        // wypisanie krotki użytkownikowi?
        removeRequest(serialNumber);
        std::cout << "got tuple ";
        tuple.print();
        tupleReady(tuple);
    } else
        forwardMessage(request);
}

void Proces::handleRequestConn(protocol::control_data request) {
    int destId = request.read_int();
    nextId = destId;
    close(writeFd); // TODO: czy tutaj close(writefd) czegoś nie zapsuje?
    writeFd = openWrite(destId);
}

int Proces::openWrite(int id) {
    // zmień ścieżkę na zadane fifo
    std::string path = directory + "pipe_" + std::to_string(id);
    int fd = open(path.c_str(), O_RDWR);

    return fd;
}

void Proces::put(Tuple tuple) {
    std::cout<<"Adding tuple..."<<std::endl;
    outTuples.push_back(tuple);
    std::pair<bool, Tuple> result;
    int serialNumber = -1;
    for(const auto& i : requests)
    {
        std::string pattern = i.second.first;
        result = findTuple(pattern);

        if(result.first){
            serialNumber = i.first;
            sendOwnTuple(i.second.second.first, serialNumber);
            std::cout << i.second.second.first << std::endl;
            break;
        }
    }
//    if(result.first)
//    {
//        sendOwnTuple(nextId, serialNumber);
//    }

}

Tuple Proces::getTuple(int timeout) {
    pthread_mutex_lock(&mutex);
    isWantingTuple = true;
    if (isTupleReady) {
        isTupleReady = false;
        isWantingTuple = false;
        pthread_mutex_unlock(&mutex);
        return tuple;
    } else {
        timespec time;
        timespec_get(&time, TIME_UTC);
        time.tv_sec += timeout;
        time.tv_nsec = 0;
        int status = pthread_cond_timedwait(&cond, &mutex, &time);
        if (isTupleReady) {
            isWantingTuple = false;
            isTupleReady = false;
            pthread_mutex_unlock(&mutex);
            return tuple;
        } else {
            isWantingTuple = false;
            pthread_mutex_unlock(&mutex);
            return Tuple(0);
        }
    }
}


void Proces::tupleReady(Tuple tuple) {
    pthread_mutex_lock(&mutex);
    if (isWantingTuple) {
        this->tuple = std::move(tuple);
        isTupleReady = true;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);
}

std::pair<bool, Tuple> Proces::findTuple(const std::string &tuplePattern) {
    /*   for(Tuple tuple : tuples) {
           if (tuple.matchPattern(tuplePattern)) {
               return tuple;
           }
       }*/
    for (auto &&tuple: outTuples) {
        if (tuple.matchPattern(tuplePattern)) {
            return std::pair<bool, Tuple>({true, tuple});
        }
    }
    return std::pair<bool, Tuple>({false, Tuple(nullptr)});
}

void Proces::sendRequestTuple(int destId, const std::string &tuplePattern, int timeout) {
    protocol::control_data request(0);
    request.id_sender = processId;
    request.id_recipient = destId;
    request.expirationDate = std::chrono::duration_cast<std::chrono::milliseconds>
                    (std::chrono::system_clock::now().time_since_epoch()).count() + timeout * 1000;

    long long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, 2147483647);
    int serialNumber = distribution(generator);
    request.write_int(serialNumber);
    request.write_int(tuplePattern.size());
    request.write_string(tuplePattern);

    request.send_msg(writeFd);

    addRequest(tuplePattern, processId, serialNumber, request.expirationDate); // TODO
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
    protocol::control_data request(3);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    request.write_int(tuple.getType().size());
    request.write_string(tuple.getType());
    for (int i = 0; i < tuple.getSize(); ++i) {
        if (tuple[i].getType() == STRING) {
            request.write_int(strlen(tuple[i].getStringValue()));
            request.write_string(tuple[i].getStringValue());
        } else if (tuple[i].getType() == INT) {
            request.write_int(tuple[i].getIntValue());
        } else {
            request.write_float(tuple[i].getFloatValue());
        }
    }
    request.send_msg(writeFd);
    outTuples.erase(std::remove(outTuples.begin(), outTuples.end(), tuple), outTuples.end());
}

void Proces::forwardMessage(protocol::control_data &request) {
    request.send_msg(writeFd);
}

bool Proces::findRequest(int serialNumber) {
    for (auto &request: requests) {
        if (request.second.second.second < std::chrono::duration_cast<std::chrono::milliseconds>
                            (std::chrono::system_clock::now().time_since_epoch()).count()) {
            requests.erase(request.first);
        }
    }
    auto request = requests.find(serialNumber);
    return request != requests.end();
}

std::pair<std::string, std::pair<int, long long>> Proces::getRequest(int serialNumber) {
    return requests.find(serialNumber)->second;
}

Tuple Proces::readTupleFromRequest(int number, const std::string &tupleType, protocol::control_data &data) {
    Tuple tuple;
    for (char i : tupleType) {
        if (i == 'i') {
            tuple.addElement(data.read_int());
        } else if (i == 'f') {
            tuple.addElement(data.read_float());
        } else if (i == 's') {
            int size = data.read_int();
            std::string str = data.read_string(size);
            tuple.addElement(str.c_str());
        }
    }
    return tuple;
}

void Proces::addRequest(const std::string &request, int idSender, int serialNumber, long long expirationDate) {
    std::pair<int, long long> metadata({idSender, expirationDate});
    std::pair<std::string, std::pair<int, long long>> requestInfo({request, metadata});
    requests.insert({serialNumber, requestInfo});
    std::cout<<"Process "<<processId<<":-   added request: "<<request<<" from "<<idSender<<std::endl;
}


void Proces::removeRequest(int serialNumber)
{
    auto it = requests.find(serialNumber);
    requests.erase(it);
}

void Proces::displayRequests() {
    std::cout<<"Process "<<processId<<":-   requests:"<<std::endl;

    for(const auto& i : requests){
        std::cout<<"Process "<<processId<<":-  "<<i.second.first<<" from "<<i.second.second.first<<std::endl;
    }
}

void Proces::displayRingState() {
    std::vector<int> structure = readMainPipe();
    std::cout<<"Current ring state: ";
    for(auto i : structure){
        std::cout<<i<<", ";
    }
    std::cout<<std::endl;
    writeMainPipe(structure);
}


ProcesException::ProcesException(const std::string &msg) : info("Process Exception: " + msg) {

}

const char *ProcesException::what() const noexcept {
    return info.c_str();
}


