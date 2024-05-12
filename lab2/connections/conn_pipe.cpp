#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>
#include <iostream>

#include "conn.hpp"


namespace{
    class ConnPipe : public Conn{
        private:
            int pipe_host_to_client[2];
            int pipe_client_to_host[2];
        public:
            ConnPipe(std::string name, bool is_host);
            ~ConnPipe(){};

            bool read(void *buf, size_t count) override;
            bool write(void *buf, size_t count) override;
    };
}

Conn* Conn::create(std::string name, bool is_host){
    return new ConnPipe(name, is_host);
}

Conn::~Conn(){}

ConnPipe::ConnPipe(std::string name, bool is_host){
    this->is_host = is_host;
    this->name = name;

    if(pipe(pipe_client_to_host) == -1 || pipe(pipe_host_to_client) == -1){
        syslog(LOG_ERR, "Failed to make pipe connection");
        exit(1);
    }

    int flags_to_host = fcntl(pipe_client_to_host[0], F_GETFL, 0);
    int flags_to_client = fcntl(pipe_host_to_client[0], F_GETFL, 0);
    fcntl(pipe_client_to_host[0], F_SETFL, flags_to_host | O_NONBLOCK);
    fcntl(pipe_host_to_client[0], F_SETFL, flags_to_client | O_NONBLOCK);
}


bool ConnPipe::read(void *buf, size_t count){
    int read_fd = is_host ? pipe_client_to_host[0] : pipe_host_to_client[0];
    int n_symbols = ::read(read_fd, buf, count);

    return n_symbols > 0;
}


bool ConnPipe::write(void *buf, size_t count){
    int write_fd = is_host ? pipe_host_to_client[1] : pipe_client_to_host[1];
    int n_symbols = ::write(write_fd, buf, count);

    return n_symbols > 0;
}
