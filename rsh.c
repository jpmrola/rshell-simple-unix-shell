#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// TODO: built-in path and cd commands
// TODO: check more errors
// TODO: better batch mode
// TODO: history file

int main(int argc, char *argv[]) {

    const char prompt[] = "rsh> ";
    const char *path[] = {"./","/bin/","/usr/bin/","/usr/games/"};
    char *program_path;
    char *buff = NULL;
    char **command_args = (char**)malloc(20*sizeof(char*));
    char *saveptr = NULL;
    size_t n = 0;
    int i = 0, interactive = 1, valid_path = 0, rc, rc_wait;
    FILE *fd = stdin; // defaults to stdin (interactive mode)

    if(argc > 2) {
        fprintf(stderr, "Error: Do not supply more than one argument\n");
        exit(0);
    } else if(argc == 2) {
        fd = fopen(argv[1],"r");
        if(fd != NULL) {
        interactive = 0;
        } else {
            fprintf(stderr, "Error: Could not open file\n");
            exit(0);
        }
    }

    if(interactive == 1){
    fprintf(stdout, prompt);
    }
    while(getline(&buff, &n, fd) != -1) {

        /* command parser starts here */
        
        for(command_args[i] = strtok_r(buff, " ", &saveptr); command_args[i] != NULL; command_args[i] = strtok_r(NULL, " ", &saveptr)){
            i++;
        }

        strtok_r(command_args[i-1], "\n", &saveptr);
        i = 0;

            /* built-in commands start here */    

        if(strcmp(command_args[0], "exit") == 0) {
            free(buff);
            free(command_args);
            exit(0);
        }

            /* built-in commands end here*/    

        // Can be turned into a separate function

        if (command_args[0][0] == '/' || command_args[0][0] == '.') {   // Not using PATH variable
            if (access(command_args[0], X_OK) == 0) {
                #ifdef DEBUG
                fprintf(stdout, "access successful %s\n", command_args[0]);
                #endif
                valid_path = 1;
            } else {
            valid_path = -1;
            fprintf(stderr, "Error: command not found\n");
            }
        } else {                                                        // Using PATH variable
            for(int c = 0; sizeof(path)/sizeof(path[0]) > c; c++) {
                program_path = strdup(path[c]);
                program_path = realloc(program_path, sizeof(1 + strlen(path[c]) + strlen(command_args[0])));
                strcat(program_path, command_args[0]);
                if (access(program_path, X_OK) == 0) {
                    command_args[0] = strdup(program_path);
                    free(program_path);
                    #ifdef DEBUG
                    fprintf(stdout, "access successful %s\nargs: %s\n", command_args[0], command_args[1]);
                    #endif
                    valid_path = 1;
                    break;
                } else {
                    valid_path = -1;
                }
                free(program_path);
            }
            if(valid_path != 1) {
                fprintf(stderr, "Error: command not found\n");
            }
        }

        /* command parser ends here */

        /* process management starts here*/

        if(valid_path == 1) {
            rc = fork();
            if(rc < 0) {
                fprintf(stderr, "Error: fork failed\n");
                exit(0);
            } else if(rc == 0) {
                printf("child process pid:%d\n", (int)getpid());
                if(execv(command_args[0], command_args) < 0) {
                    kill(getpid(), SIGINT);
                    fprintf(stderr, "Error: call to exec failed\n");
                }
            } else {
                rc_wait = wait(NULL);
                if(rc_wait < 0) {
                    fprintf(stderr, "Error: wait child process failed");
                }
            }
        } 

        /* process management ends here*/

        free(buff);
        n = 0;
        buff = NULL;
        if(interactive == 1) {
            fprintf(stdout, prompt);
        }
    }
    return 0;
}
