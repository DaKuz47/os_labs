#pragma once
#include <cstdlib>
#include <string>


class Conn{
    public:
        static Conn* create(std::string name, bool is_host);
        virtual bool read(void *buf, size_t count) = 0;
        virtual bool write(void *bug, size_t count) = 0;
        virtual ~Conn() = 0;

        bool is_host;
        std::string name;
};
