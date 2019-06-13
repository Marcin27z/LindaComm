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
#include <random>
#include <dirent.h>
#include "Proces.h"

void ProcessInterface::manageInput() {
    while(input != "exit") {
        getline(std::cin, input);

        tokens = splitBySpace(input);
        if(tokens.empty()) continue;

        if(tokens[0] == "input") {
            if(tokens.size() < 2) {
                std::cout << "Incorrect command" << std::endl;
                continue;
            }
            std::cout << "trying to get a tuple..." << std::endl;
            Tuple tuple = linda::input_linda(tokens[1], 30);
        }
        else if(tokens[0] == "read") {
            if(tokens.size() < 2) {
                std::cout << "Incorrect command" << std::endl;
                continue;
            }
            std::cout << "trying to get a tuple..." << std::endl;
            Tuple tuple = linda::read_linda(tokens[1], 30);
        }
        else if(tokens[0] == "output") {
            if(tokens.size() < 2) {
                std::cout << "Incorrect command" << std::endl;
                continue;
            }
            Tuple tuple;
            for(int i = 1; i < tokens.size(); i++) {
                if(tokens[i][tokens[i].size()-1] == 'f') {
                    try {
                        std::string text(tokens[i].begin(), tokens[i].end()-1);
                        float value = std::stof(text);
                        tuple.addElement(value);
                    }
                    catch(...) {
                        tuple.addElement(tokens[i].c_str());
                    }
                }
                else {
                    try {
                        int value = std::stoi(tokens[i]);
                        tuple.addElement(value);
                    }
                    catch(...) {
                        tuple.addElement(tokens[i].c_str());
                    }
                }
            }
            linda::output_linda(tuple);
//            if(tokens[1] == "1")
//                linda::output_linda(Tuple(1, 2, 3));
//            else if(tokens[1] == "2")
//                linda::output_linda(Tuple(1.01f, 13.37f, 9.997f));
//            else if(tokens[1] == "3")
//                linda::output_linda(Tuple("tuple", 5, 3.01));
//            else std::cout<<"Incorrect command"<<std::endl;
        }
        else if(input == "help") {
            std::cout << "HELP" << std::endl;
            std::cout << "\tinput tuple - Prośba o zadaną krotkę. Argument tuple określa rodzaj krotki, o którą proces prosi." << std::endl;
            std::cout << "\tread tuple - Prośba o odczyt zadanej krotki. Argument tuple określa rodzaj krotki, o którą proces prosi." << std::endl;
            std::cout << "\toutput tuple - Dodanie krotki do przestrzeni krotek.";
            std::cout << " Argument tuple jest ciągiem elementów składowych rozdzielonych spacjami." << std::endl;
            std::cout << "\thelp - Wyświetlenie tej informacji." << std::endl;
            std::cout << "\tdisplay_requests - Wyświetlenie oczekujących żądań." << std::endl;
            std::cout << "\tdisplay_state - Wyświetlenie stanu, w którym znajduje się proces." << std::endl;
            std::cout << "\tdisplay_tuples - Wyświetlenie posiadanych krotek procesu." << std::endl;
            std::cout << "\tdisconnect - Rozłączenie procesu z przestrzeni krotek." << std::endl;
        }
        else if (input == "display_requests")
            linda::display_requests();
        else if(input == "display_state")
            linda::display_state();
        else if(input == "display_tuples")
            linda::display_tuples();
        else if(input == "disconnect")
            break;
        else std::cout<<"Incorrect command"<<std::endl;
    }
}

std::vector<std::string> ProcessInterface::splitBySpace(const std::string &input) {
    std::string buf;
    std::stringstream ss(input);

    std::vector<std::string> tokens;

    while(ss >> buf)
        tokens.push_back(buf);

    return tokens;
}

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
    if (mkfifo(pipePath.c_str(), PERM) < 0)
        throw ProcessException("creating pipe failed: " + pipePath + ", " + strerror(errno));

    if ((readFd = open(pipePath.c_str(), O_RDWR)) < 0)
        throw ProcessException("opening pipe failed: " + pipePath + ", " + strerror(errno));

    if(size) fcntl(readFd, F_SETPIPE_SZ, size);
}

void Proces::connectToMainPipe() {
    mainFd = open(mainPipePath.c_str(), O_RDWR);

    // jeśli główna kolejka nie istnieje
    if (mainFd == -1) {
        // utwórz główną kolejkę i wstaw do niej strukturę
        createMainPipe();

        // utwórz własną kolejkę
        createPipe();
        nextPipePath = pipePath;
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
            nextId = *(new_structure.end() - 2);
        }
    }
    writeFd = open(nextPipePath.c_str(), O_RDWR);
}

void Proces::createMainPipe() {
    // utwórz główną kolejkę
    if(mkfifo(mainPipePath.c_str(), PERM) < 0)
        throw ProcessException("creating main fifo failed: " + mainPipePath + ", " + strerror(errno));

    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
        throw ProcessException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno));

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

        for (int i = 0; i < numberOfProcesses; ++i)
            structure_vec.push_back(structure_out.read_int());
    }
    return structure_vec;
}

void Proces::writeMainPipe(const std::vector<int> &new_structure) {
    protocol::control_data structure(999);
    structure.write_int((int)new_structure.size());

    for(int it : new_structure)
        structure.write_int(it);

    structure.send_msg(mainFd);
}


void Proces::disconnect() {
    std::cout<<"Disconnecting! "<<pipePath<<std::endl;

    if((mainFd = open(mainPipePath.c_str(), O_RDWR)) < 0)
        throw ProcessException("opening main fifo failed: " + mainPipePath + ", " + strerror(errno) + ", cannot disconnect");

    std::vector<int> structure = readMainPipe();

    // jeśli obecny proces jest jedynym, należy wysadzić główną kolejkę w powietrze
    if (structure.size() == 1) {
        unlink(mainPipePath.c_str());
        unlink(pipePath.c_str());
    } else if (structure.size() > 1) {

        std::cout<<"Unlinking "<<pipePath<<std::endl;
        unlink(pipePath.c_str());

        auto it = std::find(structure.begin(), structure.end(), processId);
        int index = (int)std::distance(structure.begin(), it);

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
//    unlink(pipePath.c_str());
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
    while(!quitFlag)
        handleRequests();
}

void Proces::handleRequests() {
    if(manager.assemble(readFd)) {
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
            case 5:
                handleNotAcceptTuple(request);
                break;
            default:
                quitFlag = true;
                break;
        }
    }
}

void Proces::handleRequestTuple(protocol::control_data &request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

    int tuplePatternSize = request.read_int();
    std::string tuplePattern = request.read_string(tuplePatternSize);
    std::cout << tuplePattern << std::endl;
    auto trove = findTupleByPattern(tuplePattern);
    std::cout << "RequestTuple" << std::endl;
    if(trove.first) {
        std::cout << "RequestTuple" << std::endl;

        trove.second.setSerialNumber(serialNumber);
        addRequest(tuplePattern, request.id_sender, serialNumber, request.expirationDate);
        sendOwnTuple(request.id_sender, serialNumber);
        std::cout << "found matching tuple ";
        trove.second.print();
    } else if(request.id_sender != processId) {
        addRequest(tuplePattern, request.id_sender, serialNumber, request.expirationDate);
        request.write_int(serialNumber);
        request.write_int(tuplePatternSize);
        request.write_string(tuplePattern);
        forwardMessage(request);
    }
}



void Proces::handleOwnTuple(protocol::control_data &request) {
    std::cout << "handleOwnTuple, recipient:" << request.id_recipient << std::endl;
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy

    if(request.id_recipient == processId) {
        if (findRequest(serialNumber))  // jeśli żądanie jest nadal aktualne
            sendAcceptTuple(request.id_sender, serialNumber);     // wysłanie informacji do procesu, że nadal chcemy tę krotkę
        else sendNotAcceptTuple(request.id_sender, serialNumber);

    } else{
        std::cout << "Forwarding" << std::endl;
        request.write_int(serialNumber);
        forwardMessage(request);
    }
}

void Proces::handleAcceptTuple(protocol::control_data &request) {
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    if(request.id_recipient == processId) {
        if(findRequest(serialNumber)) {
            auto foundRequest = getRequest(serialNumber);
            auto &tuple1 = findTupleBySerial(serialNumber).second;
            sendGiveTuple(request.id_sender, serialNumber, tuple1);
            std::cout << "Tuple sent" << std::endl;
            removeRequest(serialNumber);
        }
    } else{
        request.write_int(serialNumber);
        forwardMessage(request);
    }
}
void Proces::handleNotAcceptTuple(protocol::control_data &request) {
    // jeśli request istnieje to go usuwamy
    int serialNumber = request.read_int(); // losowa liczba przydzielana żądaniu, aby wykluczyć hazardy
    if (request.id_recipient == processId) {
        if (findRequest(serialNumber)) {
            std::string pattern = getRequest(serialNumber).first;

            auto trove = findTupleBySerial(serialNumber);
            if(trove.first) if(trove.second.isBlocked()) trove.second.setSerialNumber(-1);
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

        Tuple tuple1 = readTupleFromRequest(serialNumber, tupleType, request);

        //TODO: tutaj sprawdzenie czy chcieliśmy tę krotkę?
        // usunięcie krotki z requestów,
        // wypisanie krotki użytkownikowi?
        removeRequest(serialNumber);
        std::cout << "got tuple ";
        tuple1.print();
        tupleReady(tuple1);
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

void Proces::put(const Tuple& newTuple) {
    std::cout << "Adding tuple..." << std::endl;
    outTuples.push_back(newTuple);
    int serialNumber;
    refreshRequests();
    for(const auto& i : requests) {
        std::string pattern = i.second.first;
        auto result = findTupleByPattern(pattern);

        if(result.first){
            serialNumber = i.first;
            int recipient = i.second.second.first;
            sendOwnTuple(i.second.second.first, serialNumber);
            std::cout << i.second.second.first << std::endl;
            result.second.setSerialNumber(serialNumber); // block tuple
            break;
        }
    }

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
        if(isTupleReady) {
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

std::pair<bool, Tuple&> Proces::findTupleByPattern(const std::string &tuplePattern) {
    /*   for(Tuple tuple : tuples) {
           if (tuple.matchPattern(tuplePattern)) {
               return tuple;
           }
       }*/
    for (auto &&tuple1: outTuples)
        if (tuple1.matchPattern(tuplePattern))
            return {true, tuple1};

    return {false, nullTuple};
}

std::pair<bool, Tuple&> Proces::findTupleBySerial(int serialNumber) {

    for (auto &&tuple1: outTuples)
        if (tuple1.getSerialNumber() == serialNumber)
            return {true, tuple1};

    return {false, nullTuple};
}

void Proces::sendRequestTuple(int destId, const std::string &tuplePattern, int timeout, bool isRead) {
    protocol::control_data request(0);
    request.id_sender = processId;
    request.id_recipient = destId;
    int serialNumber;
    request.expirationDate = std::chrono::duration_cast<std::chrono::milliseconds>
                    (std::chrono::system_clock::now().time_since_epoch()).count() + timeout * 1000;

    long long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(0, 2147483647/2);

    if(isRead) // jeśli tylko czytamy krotkę, serialNumber będzie parzysty
        serialNumber = distribution(generator)*2;
    else       // jeśli pobieramy krotkę, serialNumber będzie nieparzysty
        serialNumber = distribution(generator)*2 + 1;

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

void Proces::sendNotAcceptTuple(int destId, int serialNumber) {
    protocol::control_data request(5);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    request.send_msg(writeFd);
}


void Proces::sendGiveTuple(int destId, int serialNumber, Tuple &tuple_) {
    protocol::control_data request(3);
    request.id_sender = processId;
    request.id_recipient = destId;

    request.write_int(serialNumber);
    request.write_int((int)tuple_.getType().size());
    request.write_string(tuple_.getType());
    for (int i = 0; i < tuple_.getSize(); ++i) {
        if (tuple_[i].getType() == STRING) {
            request.write_int((int) strlen(tuple_[i].getStringValue()));
            request.write_string(tuple_[i].getStringValue());
        }
        else if (tuple_[i].getType() == INT)
            request.write_int(tuple_[i].getIntValue());
        else
            request.write_float(tuple_[i].getFloatValue());
    }
    request.send_msg(writeFd);

    // jeśli numer seryjny zapytania jest nieparzysty, to znaczy, że proces chce ją pobrać, a więc ją usuwamy
    if(serialNumber%2 != 0)
        outTuples.erase(std::remove(outTuples.begin(), outTuples.end(), tuple_), outTuples.end());
    else
        tuple_.setSerialNumber(-1); // unlock the tuple

    // w przeciwnym wypadku, gdy numer jest parzysty, proces chce ją tylko przeczytać, a więc jej nie usuwamy
}

void Proces::forwardMessage(protocol::control_data &request) {
    request.send_msg(writeFd);
}

void Proces::refreshRequests() {
    for (auto &request: requests)
        if (request.second.second.second < std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::system_clock::now().time_since_epoch()).count())
            requests.erase(request.first);
}

bool Proces::findRequest(int serialNumber) {
    refreshRequests();
    auto request = requests.find(serialNumber);
    return request != requests.end();
}

Tuple Proces::readTupleFromRequest(int number, const std::string &tupleType, protocol::control_data &data) {
    Tuple tuple;
    for (char i : tupleType)
        switch(i) {
            case 'i':
                tuple.addElement(data.read_int());
                break;
            case 'f':
                tuple.addElement(data.read_float());
                break;
            case 's': {
                int size = data.read_int();
                std::string str = data.read_string(size);
                tuple.addElement(str.c_str());
                break;
            }
            default:
                break;
        }

    return tuple;
}

void Proces::addRequest(const std::string &request, int idSender, int serialNumber, long long expirationDate) {
    std::pair<int, long long> metadata({idSender, expirationDate});
    std::pair<std::string, std::pair<int, long long>> requestInfo({request, metadata});
    requests.insert({serialNumber, requestInfo});
    std::cout<<"Process "<<processId<<":-   added request: "<<request<<" from "<<idSender<<std::endl;
}

void Proces::removeRequest(int serialNumber) {
    auto it = requests.find(serialNumber);
    requests.erase(it);
}

void Proces::displayRequests() {
    std::cout<<"Process "<<processId<<":-   requests:"<<std::endl;

    for(const auto& i : requests)
        std::cout<<"Process "<<processId<<":-  "<<i.second.first<<" from "<<i.second.second.first<<std::endl;
}

void Proces::displayRingState() {
    std::vector<int> structure = readMainPipe();
    std::cout<<"Current ring state: ";
    for(auto i : structure)
        std::cout<<i<<", ";

    std::cout<<std::endl;
    writeMainPipe(structure);
}

void Proces::displayTuples() {
    std::cout<<"Displaying tuples:"<<std::endl;
    for(auto tuple_: outTuples)
        tuple_.print(); std::cout<<std::endl;
}