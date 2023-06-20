/*
	Naveed Manzoor Afridi	| 5149575 |
	Abdul Rauf		| 5226212 |

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define LOG_MASTER_FILE "log/master.log"
#define LOG_PROCESS_A_FILE "log/processA.log"
#define LOG_PROCESS_B_FILE "log/processB.log"

pid_t pid_procA;
pid_t pid_procB;
int log_fd;

void log_message(const char *message) {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[100];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", timeinfo);

    write(log_fd, buffer, strlen(buffer));
    write(log_fd, message, strlen(message));
    write(log_fd, "\n", 1);
}

void spawn_process(const char *process_name, const char *process_path, const char *arg_list[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork error");
        exit(1);
    } else if (pid == 0) {
        execvp(process_path, (char**)arg_list);
        perror("Exec error");
        exit(1);
    } else {
        if (strcmp(process_name, "ProcessA") == 0) {
            pid_procA = pid;
        } else if (strcmp(process_name, "ProcessB") == 0) {
            pid_procB = pid;
        }
    }
}

void start_processes() {
    const char *process_A_path = "./bin/processA";
    const char *arg_list_A[] = {"konsole", "-e", process_A_path, NULL};

    const char *process_B_path = "./bin/processB";
    const char *arg_list_B[] = {"konsole", "-e", process_B_path, NULL};

    spawn_process("ProcessA", "/usr/bin/konsole", arg_list_A);
    spawn_process("ProcessB", "/usr/bin/konsole", arg_list_B);

}


time_t get_last_modified_time(const char *filename) {
    struct stat attr;
    if (stat(filename, &attr) == -1) {
        perror("Stat error");
        exit(1);
    }
    return attr.st_mtime;
}

void watchdog() {
    const char *log_files[] = {LOG_PROCESS_A_FILE, LOG_PROCESS_B_FILE};
    pid_t pids[] = {pid_procA, pid_procB};

    int modified = 0;
    int counter = 0;

    while (1) {
        time_t current_time = time(NULL);
        modified = 0;

        for (int i = 0; i < 2; i++) {
            time_t last_modified = get_last_modified_time(log_files[i]);
            if (current_time - last_modified <= 3) {
                modified = 1;
                counter = 0;
                break;
            }
        }

        if (modified == 0) {
            counter += 3;
        }

        if (counter > 60) {
            kill(pid_procA, SIGKILL);
            kill(pid_procB, SIGKILL);
            break;
        }

        sleep(2);
    }
}

void open_log()
{
    // Open the log file
    log_fd = open(LOG_MASTER_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if (log_fd == -1)
    {
        perror("Error opening log file");
        exit(1);
    }
    log_message("---- Master Process Started---- ");
}

int main() {

    // open the log file
    open_log();
    
    // Initialize Process A & B in konsole window
    start_processes();
    
    // watchdog for time out
    watchdog();
    
    // Variable to store the status of the child processes
    int status;

    // Wait for the child processes to terminate
    waitpid(pid_procA, &status, 0);
    waitpid(pid_procB, &status, 0);
  
    log_message("---- Master process terminated---- ");

    close(log_fd);
    return 0;
}

