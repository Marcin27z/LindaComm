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
            request_tuple,
            own_tuple,
            accept_tuple,
            give_tuple,
            request_conn,
            main_fifo_structure,

            unknown
        };

        static const std::map<int, Type> types;

        int type = -1;
        uint buf_length = 0;
        uint id_sender;
        int id_recipient;
        std::vector<char> buffer;

        Type get_type(int) const;

        // Functions used to read values from message
        int read_int();
        float read_float();
        std::string read_string(int);

        // Functions used to write values to message
        void write_int(int);
        void write_float(float);
        void write_string(std::string);

        int send_msg(int);

        control_data() = default;
        explicit control_data(int t): type(t), id_sender(0), id_recipient(0) {}

        control_data(int t, char* b, uint bl, uint id_s, int id_r = -1):
        type(t), buf_length(bl), id_sender(id_s), id_recipient(id_r) {
            buffer = std::vector<char>(b, b+bl);
        }
    };

    class manager {
        private:
            std::map<int, std::vector<char>> buffers;
            std::map<int, control_data> msg_buffer;

            ssize_t read_result;

            bool is_cd_ready(int);

            size_t expected_cd_size(int);
            size_t remaining_cd_size(int);

            uint pop_int(int);   // Int?

        public:
            int last_result() const;
            bool assemble(int);

            control_data read_data(int);
    };
}

#endif //CONTROL_DATA_H
