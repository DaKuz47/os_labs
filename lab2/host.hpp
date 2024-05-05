#include <map>
#include <semaphore.h>
#include <string>
#include <vector>


class Host{
    private:
        const std::string tell_cmd = "/tell";

        static Host* instance;
        std::map<std::string, Client*> clients;
        sem_t* write_sem;
        sem_t* client_read_sem;
        sem_t* host_read_sem;


        Host() = default;
        static void* read(void* host);
        static void* write(void* host);
        void broadcast(char* buf, Client* from_client);
        void tell(char* buf, Client* from_client);
    public:
        ~Host(){std::cout << "bye!" << std::endl;};
        Host(const Host& ohter) = delete;
        Host& operator=(const Host& other) = delete;
        Host(Host&& other) = delete;

        static Host* getInstance();
        void run(const std::vector<std::string>& client_names);
        void prepareSemaphores();
};
