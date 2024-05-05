#include <cstdlib>
#include <fstream>
#include <semaphore.h>
#include <string>

#include "connections/conn.hpp"


class Client{
    private:
        pid_t host_pid;
        std::string client_out_path;
        std::string client_in_path;
        sem_t *write_sem;
        sem_t *client_read_sem;
        sem_t *host_read_sem;

        void prepareIOSystem();
        void prepareTerminal();
        void prepareSemaphores();

        void waitForOtherClients();

        static void* writeMessage(void *client);
        static void* readMessage(void *client);
    public:
        std::ofstream client_out;
        Conn* connection;
        pid_t client_pid;
        std::string name;

        Client(pid_t host_pid, std::string& name);
        void run();

};
