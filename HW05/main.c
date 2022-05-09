/*
    Homework n.5
    Estendere l'esempio 'nanoshell.c' ad una shell piu' realistica in cui
    si possono:
    - passare argomenti al comando invocato (per semplicita', assumiamo
    che questi non contengano spazi);
    - usare la redirezione dei canali di input/output/error;
    - mandare un comando in esecuzione in background (con la '&' finale).
    Esempi di invocazione che la shell dovrebbe supportare sono:
    $ cal 3 2015
    $ cp /etc/passwd /etc/hosts /tmp
    $ cat </dev/passwd >/tmp/passwd
    $ cat filechenonesiste 2>/dev/null
    $ ls -R /etc/ &
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

#define bool u_int8_t
#define true 1
#define false 0
#define MAX_ARG 30
#define ARG_DELIMIT " "

int main() {
    char command[PATH_MAX];
    char *arguments[MAX_ARG];
    char *singleElement, *childStdin, *childStdout, *childStderr;
    size_t commandsSupplied, commandLen;
    int pid, fd;
    bool shouldRunInBackground;

    while (1) {
        childStdin = NULL;
        childStdout = NULL;
        childStderr = NULL;
        shouldRunInBackground = false;
        commandsSupplied = 0;

        printf("🏴‍☠️ %% ");
        fgets(command, PATH_MAX, stdin);
        commandLen = strlen(command);
        if (command[commandLen - 1] == '\n')
            command[commandLen - 1] = '\0';
        
        if (!strncmp(command, "exit", PATH_MAX))
            break;
        
        for (singleElement=strtok(command, ARG_DELIMIT); singleElement && commandsSupplied<MAX_ARG; singleElement=strtok(NULL, ARG_DELIMIT)) {
            size_t elementLen = strlen(singleElement);

            // Run in background
            if (elementLen == 0 || !strcmp(singleElement, "&")) {
                shouldRunInBackground = true;
                printf("Background execution enabled\n");
                continue;
            }

            if (elementLen > 0) {
                // Check stdout redirect
                switch (singleElement[0]) {
                    case '>':
                        childStdout = (char*)malloc(elementLen*sizeof(char));
                        strncpy(childStdout, singleElement+1, elementLen);
                        printf("STDOUT: %s\n", childStdout);
                        continue;
                    case '<':
                        childStdin = (char*)malloc(elementLen*sizeof(char));
                        strncpy(childStdin, singleElement+1, elementLen);
                        printf("STDIN: %s\n", childStdin);
                        continue;
                }
                if (!strncmp(singleElement, "2>", 2)) {
                    childStderr = (char*)malloc(elementLen*sizeof(char));
                    strncpy(childStderr, singleElement+2, elementLen);
                    continue;
                }
            }

            arguments[commandsSupplied] = (char*)malloc((elementLen+1)*sizeof(char));
            strncpy(arguments[commandsSupplied], singleElement, elementLen+1);
            printf("DEBUG: %s\n", arguments[commandsSupplied++]);
        }
        arguments[commandsSupplied] = NULL;

        if ((pid=fork()) == -1) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) {
            if (childStdin) {
                fd = open(childStdin, O_RDONLY, 0660);
                close(0);
                dup(fd);
            }

            if (childStdout) {
                fd = open(childStdout, O_WRONLY | O_CREAT | O_TRUNC, 0660);
                close(1);
                dup(fd);
            }

            if (childStderr) {
                fd = open(childStderr, O_WRONLY | O_CREAT | O_TRUNC, 0660);
                close(2);
                dup(fd);
            }

            if (commandsSupplied > 0) {
              execvp(arguments[0], arguments);
              printf("There was an error during the execution of %s\n", command);
            }
        } else {
            if (!shouldRunInBackground) {
                wait(NULL);
            }
            /* Memory cleanup */
            // Deleting allocated commands
            while (commandsSupplied > 0) {
                if (arguments[--commandsSupplied]) {
                    free(arguments[commandsSupplied]);
                    arguments[commandsSupplied] = NULL;
                }
            }
            // Deleting allocated streams
            if (childStdin)
                free(childStdin);
            if (childStdout)
                free(childStdout);
            if (childStderr)
                free(childStderr);
        }
    }

    return 0;
}
