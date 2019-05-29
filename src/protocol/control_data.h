//
// Created by Fen on 29.05.2019.
//

#ifndef CONTROL_DATA_H
#define CONTROL_DATA_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>

namespace protocol {
    struct control_data {
        enum class Type : int {
            output_linda,
            input_linda,
            read_linda,

            unknown
        };

        static const std::map<int, Type> types;

        int type = -1;
        uint buf_length = 0;
        std::vector<char> buffer;

        Type get_type(int) const;

        int send_msg(int);             // But how? Int?

        int read_int();
        float read_float();
        std::string read_string(int);

        void write_int(int);
        void write_float(float);
        void write_string(std::string);

        control_data() {};
        control_data(int t): type(t) {}
        control_data(int t, char* b, int bl): type(t), buf_length(bl) {
            buffer = std::vector<char>(b, b+bl);
        }
    };

    class manager {
        private:
            std::map<int, std::vector<char>> buffers;
            std::map<int, control_data> msg_buffer;

            ssize_t read_result;

            bool is_cd_ready(int);      // Int?

            size_t expected_cd_size(int);       // Int?
            size_t remaining_cd_size(int);      // Int?

            uint pop_int(int);   // Int?

        public:
            int last_result() const;
            bool assemble(int);

            control_data read_data(int);
    };
}

#endif //CONTROL_DATA_H
