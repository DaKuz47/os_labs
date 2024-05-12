#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <signal.h>
#include <semaphore.h>
#include <syslog.h>
#include <unistd.h>

#include "client.hpp"
#include "host.hpp"

const char* PARTICIPANTS_FILE = "participants.conf";
const size_t BUFF_SIZE = 1000;


Host* Host::instance = nullptr;

Host* Host::getInstance(){
    if (!instance) {
        instance = new Host();
    }

    return instance;
}


void Host::prepareSemaphores(){
    sem_unlink("/write");
    write_sem = sem_open("/write", O_CREAT, 0777, 1);
    if(write_sem == SEM_FAILED){
        syslog(LOG_ERR, "Failed open write semaprhore on host side");
        exit(1);
    }
    sem_unlink("/client_read");
    client_read_sem = sem_open("/client_read", O_CREAT, 0777, 0);
    if(client_read_sem == SEM_FAILED){
        sem_close(write_sem);
        syslog(LOG_ERR, "Failed open client_read semaprhore on host side");
        exit(1);
    }
    sem_unlink("/host_read");
    host_read_sem = sem_open("/host_read", O_CREAT, 0777, 0);
    if(client_read_sem == SEM_FAILED){
        sem_close(write_sem);
        sem_close(client_read_sem);
        syslog(LOG_ERR, "Failed open host_read semaprhore on host side");
        exit(1);
    }
}


void Host::broadcast(char* buf, Client* from_client){
    // Message to all chat participants

    sem_wait(write_sem);
    char msg[BUFF_SIZE];
    for (auto client : clients){
        if (client.second == from_client){
            continue;
        }
        memset(msg, '\0', BUFF_SIZE);
        strcat(msg, from_client ? from_client->name.c_str() : "Host");
        strcat(msg, ": ");
        strcat(msg, buf);
        client.second->connection->write(msg, BUFF_SIZE);
    }
    // Write to host console
    if (from_client){
        std::cout << msg << std::endl;
    }

    for (int i = 0; i < static_cast<int>(clients.size()); i++){
        sem_post(client_read_sem);
    }
    sem_post(write_sem);
}


void Host::tell(char* buf, Client* from_client){
    // Look for personal message via /tell command

    auto str_buf = std::string(buf);
    std::stringstream ss(str_buf);
    std::string dummy;
    ss >> dummy;
    std::string to_client_name;
    ss >> to_client_name;

    std::string msg;
    std::getline(ss, msg);

    char final_msg[BUFF_SIZE];
    memset(final_msg, '\0', BUFF_SIZE);
    strcat(final_msg, "FROM ");
    strcat(final_msg, from_client->name.c_str());
    strcat(final_msg, ": ");
    strcat(final_msg, msg.c_str());

    sem_wait(write_sem);
    Client* to_client = clients[to_client_name];
    to_client->connection->write(final_msg, BUFF_SIZE);
    for (int i = 0; i < static_cast<int>(clients.size()); i++){
        sem_post(client_read_sem);
    }
    sem_post(write_sem);
}


void* Host::read(void *raw_host){
    // Look for clients inputs

    Host* host = (Host*)raw_host;
    std::map<Client*, std::chrono::steady_clock::time_point> clients_last_msg_time;

    auto start = std::chrono::steady_clock::now();
    for(auto client : host->clients){
        clients_last_msg_time[client.second] = start;
    }

    char buf[BUFF_SIZE];
    while (true){
        sem_wait(host->host_read_sem);
        for(auto client : host->clients){
            if(client.second->connection->read(buf, BUFF_SIZE)){
                clients_last_msg_time[client.second] = std::chrono::steady_clock::now();
                auto str_buf = std::string(buf);
                if (str_buf.compare(0, host->tell_cmd.size(), host->tell_cmd) == 0){
                    host->tell(buf, client.second);
                } else {
                    host->broadcast(buf, client.second);
                }

                break;
            }
        }
    }
}


void* Host::write(void *raw_host){
    // Look for consloe input and broadcast message to all participants

    Host* host = (Host*)raw_host;

    char buf[BUFF_SIZE];
    while(true){
        std::string message;
        std::getline(std::cin, message);
        if(message != ""){
            strcpy(buf, message.c_str());
            host->broadcast(buf, nullptr);
        }
    }
}


void Host::run(const std::vector<std::string>& clients_names){
    syslog(LOG_INFO, "Running host...");
    pid_t host_pid = getpid();
    for(auto name : clients_names){
        Client* client = new Client(host_pid, name);
        clients[name] = client;
        client->run();
    }

    pthread_t reader;
    pthread_t writer;
    pthread_create(&reader, nullptr, Host::read, this);
    pthread_create(&writer, nullptr, Host::write, this);
    pthread_join(reader, nullptr);
    pthread_join(writer, nullptr);
}


int main(){
    openlog("LOCAL_CHAT", LOG_PID | LOG_NDELAY, LOG_LOCAL0);

    std::vector<std::string> participants;
    std::ifstream p_file(PARTICIPANTS_FILE);
    std::string participant;
    while(std::getline(p_file, participant)){
        participants.push_back(participant);
    }

    Host* host = Host::getInstance();
    host->prepareSemaphores();
    host->run(participants);

    closelog();
    return 0;
}
