#include <csignal>
#include <syslog.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <sys/resource.h>
#include <unistd.h>

#include "daemon.hpp"


const uint32_t watch_mode = IN_CREATE | IN_DELETE;
const std::string WATCHER_PID_PATH = "/tmp/watcher_pid";

DaemonWatcher* DaemonWatcher::instance = nullptr;

DaemonWatcher* DaemonWatcher::getInstance(){
    if (!instance){
        instance = new DaemonWatcher();
    }

    return instance;
}

void DaemonWatcher::configure(const std::string& config_path){
    watch_list.clear();
    std::string item_to_watch;
    std::ifstream config_file(config_path);

    config_file >> log_file_path;
    while (config_file >> item_to_watch) {
        watch_list.push_back(item_to_watch);
    }
}


void termHandler(int signum){
    syslog(LOG_INFO, "Terminating...");
    exit(0);
}


void DaemonWatcher::prepareSystem(){
    if (getppid() != 1){
        std::signal(SIGTTOU, SIG_IGN);
        std::signal(SIGTTIN, SIG_IGN);
        std::signal(SIGTSTP, SIG_IGN);
    }

    if (fork() != 0) {
        exit(0);
    }


    auto pid = setsid();

    int fd;
    struct rlimit file_limit;
    getrlimit(RLIMIT_NOFILE, &file_limit);
    for (fd = 0; fd < file_limit.rlim_max; fd++){
        close(fd);
    }

    chdir("/");

    openlog("DaemonWatcher", LOG_PID | LOG_CONS, LOG_DAEMON);

    pid_t watcher_pid;
    if(
        (std::ifstream(WATCHER_PID_PATH) >> watcher_pid) &&
        !(kill(watcher_pid, 0) == -1 && errno == ESRCH)
    ){
        syslog(LOG_INFO, "Daemon already started");
        exit(-1);
    }

    std::ofstream(WATCHER_PID_PATH) << pid;
    
    signal(SIGTERM, termHandler);

    syslog(LOG_INFO, "Starting daemon...");
}

int DaemonWatcher::process_inotify_events(int fd, std::ofstream& log_file) {
    char buf[512];
    struct inotify_event *event;
    int event_size = sizeof(struct inotify_event);

    int read_len = read(fd, buf, sizeof(buf));
    if (read_len < event_size) {
        syslog(LOG_ERR, "could not get event!\n");
        return -1;
    }
    
    event = (struct inotify_event *) (buf);
    if (event->len) {
        if (event->mask & IN_CREATE) {
            log_file << "create file: " << event->name << std::endl;
        } else {
            log_file << "delete file: " << event->name << std::endl;
        }
        log_file.flush();
    }

    return 0;
}

int DaemonWatcher::startWatching(){
    prepareSystem();

    int InotifyFd = inotify_init1(0);
    if (InotifyFd == -1) {
        syslog(LOG_CRIT, "Failed ot initialize inotify");
        return -1;
    }

    for(auto& path : watch_list){
        int wd = inotify_add_watch(InotifyFd, path.c_str(), watch_mode);
        if (wd < 0) {
            syslog(LOG_ERR, "Failed to add %s for watch list", path.c_str());
            return -1;
        }

        watch_descriptors.push_back(wd);
    }

    std::ofstream log_file(log_file_path);
    while (true){
        process_inotify_events(InotifyFd, log_file);
    }

    syslog(LOG_INFO, "End of work");

    close(InotifyFd);
    log_file.close();
    return 0;
}


int main(int argc, char **argv){
    DaemonWatcher* daemon = DaemonWatcher::getInstance();
    daemon->configure("config.conf");
    daemon->startWatching();

    return 0;
}
