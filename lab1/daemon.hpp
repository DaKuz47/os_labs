#include <fstream>
#include <iostream>
#include <string>
#include <vector>


class DaemonWatcher{
    private:
        static DaemonWatcher* instance;
        std::string log_file_path;
        std::vector<std::string> watch_list{};
        std::vector<int> watch_descriptors{};

        void prepareSystem();
        int process_inotify_events(int fd, std::ofstream& log_file);
        DaemonWatcher() = default;
    public:
        ~DaemonWatcher(){
            for(auto wd : watch_descriptors){
                close(wd);
            }
            closelog();
        }
        DaemonWatcher(const DaemonWatcher &other) = delete;
        DaemonWatcher(DaemonWatcher &&other) = delete;
        DaemonWatcher& operator=(DaemonWatcher &other) = delete;

        static DaemonWatcher* getInstance();
        void configure(const std::string& config_path);
        int startWatching();
};
