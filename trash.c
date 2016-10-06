#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <err.h>

#define MAX_ARGS 50

struct c_pair {
    char first[256];
    char second[256];
};

char* str_concat(char* str1, char* str2) {
    char *top = str1;

    while(*(str1++) != '\0');
    str1 -= 1;

    do {
        *(str1++) = *str2;
    } while (*(str2++) != '\0');

    return str1;
}

char* str_replace(char *src, char *target, char *replace) {
    char *temp = (char*)malloc(sizeof(char) * 1000);
    if(temp == NULL) {
        printf("could not allocate memory\n");
        exit(1);
    }
    char *p;
    if((p = strstr(src, target)) == NULL) 
        return src;
    *p = '\0';
    p += strlen(target);

    strcpy(temp, p);

    str_concat(src, replace);
    str_concat(src, temp);

    free(temp);

    return src;
}



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

void load_alias_file(struct c_pair *aliases, int *num_of_aliases) {
    FILE *fp;
    char alias[256];
    char *argv[5];
    int argc;
    if((fp = fopen("./.trash_alias", "r")) == NULL) {
        return;
    } else {
        while(fgets(alias, 256, fp) != NULL) {
            char *line = alias;
            if((argv[0] = strtok(line, " \t")) == NULL) 
                    break;
            argv[1] = strtok(NULL, "");

            if(strcmp(argv[0], "alias") == 0) {
                strcpy(aliases[*num_of_aliases].first, strtok(argv[1], "="));
                strcpy(aliases[*num_of_aliases].second, strtok(NULL, "\n"));
                char* tmp = aliases[*num_of_aliases].second;
                tmp += 1;
                *(strchr(tmp, '\"')) = '\0';
                strcpy(aliases[*num_of_aliases].second, tmp);
//                printf("%d %s %s\n", *num_of_aliases, aliases[*num_of_aliases].first, aliases[*num_of_aliases].second);

                *num_of_aliases = *num_of_aliases + 1;
            }
        }
    }
}

void change_directory(char *path) {
    if(path == NULL) { 
        path = getenv("HOME");
    }
    chdir(path);
}

int main(void) {
    char* user;
    char current_path[256];
    char input[1024];
    char hostname[256];
    char buf[256];
    char *argv[256];
    char *command;
//    char *aliases[256];
    struct c_pair aliases[128];
    int num_of_alias = 0;
    int argc;
    int status;
    int parent2child[2], child2parent[2];
    pid_t wpid;
    pid_t child_pid;

    getHostname(hostname);
    user = getenv("USER");

    load_alias_file(aliases, &num_of_alias);
    printf("\nregistered aliases %d\n", num_of_alias);
    for(int i = 0; i < num_of_alias; i++) {
        printf("%s=%s\n", aliases[i].first, aliases[i].second);
    }

    printf("\n##########################\n");
    printf("# welcome to trash shell #\n");
    printf("##########################\n\n\n");

    while(1) {
        getcwd(current_path, 256);
        printf("[trash]\x1b[91m%s@%s\x1b[39m:\x1b[94m%s\x1b[39m$ ", hostname, user, current_path); 
        memset(argv,0,sizeof(argv));
    
        fgets(input, sizeof(input), stdin);
        input[strlen(input)-1] = '\0';

        for(int i = 0; i < num_of_alias; i++) {
            strcpy(input ,str_replace(input, aliases[i].first, aliases[i].second));  
        }

        command = input;

        for(argc = 0; argc < MAX_ARGS; argc++) {
            if((argv[argc] = strtok(command, " \t")) == NULL) { 
                break;
            }
            command = NULL;
        }

        if(strcmp(argv[0],"exit") == 0) {
            exit(0);
        } 

        if(strcmp(argv[0], "cd") == 0) {
            change_directory(argv[1]);
            continue;
        }

        child_pid = fork();
        if(child_pid == -1) {
            printf("Err could not fork\n");
            exit(1);
        } else if(child_pid == 0) {
    //       printf("[trash]\x1b[91m%s@%s\x1b[39m:\x1b[94m%s\x1b[39m$ ", hostname, user, current_path); 
            if(strcmp(argv[0],"emacs") == 0 ) {
                printf("you want to use emacs!?\n I guess you want to use vim\n");
                getchar();
                char * argv_vim[3];
                argv_vim[0] = "vim";
                argv_vim[1] = argv[1];
                execvp("vim", argv_vim);
            } else {
                execvp(argv[0], argv);
                perror(argv[0]);
            }
            _exit(1);
        }
    
        wait(&status);
    
        if(-1 == wpid) {
            err(EXIT_FAILURE, "waitpid error");
        }
    }
//    printf("status = %d\n", WEXITSTATUS(status));
//    printf("chiled_process_end\n");

    return 0;
}
