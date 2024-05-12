#include <cstring>
#include <iostream>
#include <signal.h>
#include <syslog.h>
#include <unistd.h>
#include "client.hpp"


const size_t BUFF_SIZE = 1000;


Client::Client(pid_t host_pid, std::string& name){
    this->host_pid = host_pid;
    this->name = name;

    connection = Conn::create(name, true);
}


void Client::prepareSemaphores(){
    write_sem = sem_open("/write", 0);
    if(write_sem == SEM_FAILED){
        syslog(LOG_ERR, "Failed open write semaprhore on %s client side", name.c_str());
        exit(1);
    }
    client_read_sem = sem_open("/client_read", 0);
    if(client_read_sem == SEM_FAILED){
        sem_close(write_sem);
        syslog(LOG_ERR, "Failed open client_read semaprhore on %s client side", name.c_str());
        exit(1);
    }
    host_read_sem = sem_open("/host_read", 0);
    if(host_read_sem == SEM_FAILED){
        sem_close(write_sem);
        sem_close(client_read_sem);
        syslog(LOG_ERR, "Failed open host_read semaprhore on %s client side", name.c_str());
        exit(1);
    }
}


void Client::prepareIOSystem(){
    client_in_path = "/tmp/" + std::to_string(getpid()) + ".in";
    client_out_path = "/tmp/" + std::to_string(getpid()) + ".out";

    client_out.open(client_out_path);
}


void Client::prepareTerminal(){
    std::string listen_client_in_cmd = "cp /dev/stdin " + client_in_path;
    std::string listen_client_out_cmd = "tail -f " + client_out_path;
    std::string new_teminal_cmd = (
        "gnome-terminal -e \"bash -c '" +
        listen_client_in_cmd + " | " +
        listen_client_out_cmd + "; exec bash'\""
    );
    int res = system(new_teminal_cmd.c_str());
    if(res != 0){
        syslog(LOG_ERR, "Can not open terminal for client %s", name.c_str());
        exit(1);
    }
}


void Client::waitForOtherClients(){
    int n_clients_need_for_read;
    do {
        sem_getvalue(client_read_sem, &n_clients_need_for_read);
    } while (n_clients_need_for_read > 0);
}


void* Client::readMessage(void *raw_client){
    Client* client = (Client*)raw_client;

    char buf[BUFF_SIZE];
    while(true){
        sem_wait(client->client_read_sem);
        memset(buf, '\0', BUFF_SIZE);
        if(client->connection->read(buf, BUFF_SIZE)){
            client->client_out << buf << std::endl;
        }
        client->waitForOtherClients();
    }
}


void* Client::writeMessage(void *raw_client){
    Client* client = (Client*)raw_client;
    std::streampos last_pos = 0;

    char buf[BUFF_SIZE];
    while(true){
        std::ifstream input(client->client_in_path);
        input.seekg(last_pos);

        std::string message;
        std::getline(input, message);

        if (input){
            last_pos = input.tellg();
        }

        if (message == ""){
            continue;
        }

        strcpy(buf, message.c_str());

        sem_wait(client->write_sem);
        client->connection->write(buf, BUFF_SIZE);
        sem_post(client->host_read_sem);
        sem_post(client->write_sem);
    }

    pthread_exit(nullptr);
}


void Client::run(){
    syslog(LOG_INFO, "Starting client %s", name.c_str());
    if (getpid() == host_pid){
        pid_t forked_pid = fork();
        if (forked_pid == 0){
            connection->is_host = false;
            client_pid = getpid();
            syslog(LOG_DEBUG, "Preparing IO System for client %s", name.c_str());
            prepareIOSystem();
            syslog(LOG_DEBUG, "Preparing Terminal for client %s", name.c_str());
            prepareTerminal();
            syslog(LOG_DEBUG, "Preparing Semaphores for client %s", name.c_str());
            prepareSemaphores();

            pthread_t writer;
            pthread_t reader;
            pthread_create(&writer, nullptr, Client::writeMessage, this);
            pthread_create(&reader, nullptr, Client::readMessage, this);
            syslog(LOG_INFO, "Client %s success join chat", name.c_str());

            pthread_join(writer, nullptr);
            pthread_join(reader, nullptr);
        } else if (forked_pid > 0){
            client_pid = forked_pid;
        } else {
            syslog(LOG_ERR, "Can not run client %s", name.c_str());
        }
    }
}
