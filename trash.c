#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <err.h>

void getHostname(char* hostname) {
    FILE* fp;
    if((fp = fopen("/proc/sys/kernel/hostname", "r")) == NULL) {
        printf("could not get hostname. exit...\n");
        exit(EXIT_FAILURE);
    }
    fgets(hostname, 256, fp);
    hostname[strlen(hostname) -1] = '\0';
    fclose(fp);
}

int main(void) {
    char* user;
    char current_path[256];
    char command[256];
    char hostname[256];
    char buf[256];
    int status;
    int parent2child[2], child2parent[2];
    pid_t wpid;
    pid_t child_pid;

    getHostname(hostname);
    user = getenv("USER");
    getcwd(current_path, 256);

    printf("[trash]\x1b[91m%s@%s\x1b[39m:\x1b[94m%s\x1b[39m$ ", hostname, user, current_path); 

    fgets(command, sizeof(command), stdin);
    command[strlen(command)-1] = '\0';
    child_pid = fork();
    if(child_pid == -1) {
        printf("Err could not fork\n");
        exit(1);
    } else if(child_pid == 0) {
//       printf("[trash]\x1b[91m%s@%s\x1b[39m:\x1b[94m%s\x1b[39m$ ", hostname, user, current_path); 
//        printf("entered command = %s\n", command);
        if(strcmp(command,"emacs") == 0 ) {
            printf("you want to use emacs!?\n I guess you want to use vim\n");
            getchar();
            char * argv_vim[] = {
                "vim",
                NULL
            };
            execvp("vim", argv_vim);
        } else {
            char *argv[] = {
                "ls",
                NULL
            };
            execvp(command, argv);
            perror(command);
        }
        _exit(1);
    }

    wait(&status);

    if(-1 == wpid) {
        err(EXIT_FAILURE, "waitpid error");
    }
//    printf("status = %d\n", WEXITSTATUS(status));
//    printf("chiled_process_end\n");

    return 0;
}
