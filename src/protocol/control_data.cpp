//
// Created by Fen on 29.05.2019.
//

#include "control_data.h"

using namespace protocol;

const std::map<int, control_data::Type> control_data::types = {
        {0, control_data::Type::output_linda},
        {1, control_data::Type::input_linda},
        {2, control_data::Type::read_linda}
};

control_data::Type control_data::get_type(int t) const {
    auto type = types.find(t);
    if(type == types.end()) return control_data::Type::unknown;
    else return type->second;
}

int control_data::read_int() {
    int result;
    std::memcpy(&result, &buffer[0], sizeof(int));
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(int));

    buf_length -= sizeof(int);

    return result;
}

float control_data::read_float() {
    float result;
    std::memcpy(&result, &buffer[0], sizeof(float));
    buffer.erase(buffer.begin(), buffer.begin() + sizeof(float));

    buf_length -= sizeof(float);

    return result;
}

std::string control_data::read_string(int length) {
    std::string result(buffer.begin(), buffer.begin() + length);
    buffer.erase(buffer.begin(), buffer.begin() + length);

    buf_length -= length;

    return result;
}

void control_data::write_int(int i) {
    const char* element = static_cast<char*>(static_cast<void*>(&i));
    buffer.insert(buffer.end(), element, element + sizeof(int));

    buf_length += sizeof(int);
}

void control_data::write_float(float f) {
    const char* element = static_cast<char*>(static_cast<void*>(&f));
    buffer.insert(buffer.end(), element, element + sizeof(float));

    buf_length += sizeof(float);
}

void control_data::write_string(std::string s) {
    buffer.insert(buffer.end(), s.begin(), s.end());

    buf_length += s.length();
}

int control_data::send_msg(int dst) {
    if(write(dst, &buf_length, sizeof(buf_length)) < 0) {
        std::cerr << "Unsuccessful write (body length)" << std::endl;
        return -1;
    }

    if(write(dst, &type, sizeof(type)) < 0) {
        std::cerr << "Unsuccessful wrtie (msg type)" << std::endl;
        return -1;
    }

    if(write(dst, &buffer[0], buf_length) < 0) {
        std::cerr << "Unsuccessful write (body)" << std::endl;
        return -1;
    }

    return buf_length + 2*sizeof(int);
}

uint manager::pop_int(int socket) {
    uint result;
    std::memcpy(&result, &buffers[socket][0], sizeof(int));

    buffers[socket].erase(buffers[socket].begin(), buffers[socket].begin() + sizeof(int));

    return result;
}

bool manager::is_cd_ready(int socket) {
    return buffers[socket].size() >= 2*sizeof(int);
}

size_t manager::expected_cd_size(int socket) {
    if(!is_cd_ready(socket)) return 2* sizeof(int);

    int length;
    std::memcpy(&length, &buffers[socket][0], sizeof(int));
    return 2*sizeof(int) + length;
}

size_t manager::remaining_cd_size(int socket) {
    return expected_cd_size(socket) - buffers[socket].size();
}

bool manager::assemble(int socket) {
    if(!is_cd_ready(socket)) {
        std::vector<char> buffer(2*sizeof(int) - buffers[socket].size());

        read_result = read(socket, &buffer[0], 2*sizeof(int) - buffers[socket].size());
        if(read_result == -1) return false;
        buffers[socket].insert(buffers[socket].end(), buffer.begin(), buffer.begin() + read_result);

        if(!is_cd_ready(socket)) return false;
    }

    std::vector<char> buffer(remaining_cd_size(socket));

    read_result = read(socket, &buffer[0], remaining_cd_size(socket));
    if(read_result == -1) return false;
    buffers[socket].insert(buffers[socket].end(), buffer.begin(), buffer.end());

    if(remaining_cd_size(socket) > 0) return false;

    control_data msg;
    msg.buf_length = pop_int(socket);
    msg.type = pop_int(socket);
    msg.buffer = std::vector<char>(buffers[socket].begin(), buffers[socket].begin() + msg.buf_length);

    buffers[socket].clear();

    msg_buffer.insert({socket, msg});

    return true;
}

control_data manager::read_data(int socket) {
    control_data msg = msg_buffer[socket];
    msg_buffer.erase(socket);
    return msg;
}