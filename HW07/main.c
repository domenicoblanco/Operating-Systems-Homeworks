/**
 * Homework n.7
 * 
 *  Modificare l'homework precedente (n.6) facendo in modo che il figlio che
 *  riceve il comando da eseguire tramite la coda, catturi lo standard output
 *  e lo standard error del figlio nipote usando la redirezione su pipe tra
 *  processi. L'output catturato dovrà essere mandato indietro al padre
 *  tramite un messaggio (per semplicita', assumiamo sufficiente grande).
 *  Tale contenuto sara' poi visualizzato sul terminale dal padre.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>

#define bool u_int8_t
#define true 1
#define false 0
#define msgMax 1024

typedef struct {
    long type;
    char text[msgMax];
    bool isEof;
} msg_t;

void parent(int fd) {
    size_t msgTxtLen;
    msg_t msg;
    
    do {
        printf("🏴‍☠️ %% ");
        fgets(msg.text, msgMax, stdin);
        msgTxtLen = strlen(msg.text);

        if (strncmp(msg.text, "\n", msgMax) == 0) {
            continue;
        }

        if (msg.text[msgTxtLen-1] == '\n') {
            msg.text[msgTxtLen-1] = '\0';
        }

        msg.type = 1;
        msg.isEof = strncmp(msg.text, "exit", msgMax) == 0;

        if (msgsnd(fd, &msg, sizeof(msg_t)-sizeof(long), 0) == -1) {
            perror("msgsnd");
            exit(1);
        }
        printf("Sent message: %s\n", msg.text);

        if (!msg.isEof) {
            if (msgrcv(fd, &msg, sizeof(msg_t)-sizeof(long), 2, 0) == -1) {
                perror("msgrcv father");
            }
            printf("Received from grandson: \n%s\n", msg.text);
        }

    } while(!msg.isEof);
}

void child(int fd) {
    int fildes[2];
    ssize_t readSize;
    msg_t msg;

    do {
        if (pipe(fildes) == -1) {
            perror("pipe");
            return;
        }

        if (msgrcv(fd, &msg, sizeof(msg_t)-sizeof(long), 1, 0) == -1) {
            perror("msgrcv");
            return;
        }

        printf("Received message: %s\n", msg.text);

        if (!msg.isEof && fork()) {
            close(fildes[1]);
            wait(NULL);         

            if ((readSize=read(fildes[0], msg.text, msgMax)) == -1) {
                perror("read");
            }

            for (ssize_t i=0; i<readSize-1; i++) {
                if (msg.text[i] == '\0') {
                    msg.text[i] = '\n';
                }
            }
            msg.text[readSize] = '\0';
            printf("Sending to father process...\n");

            msg.type = 2;
            if (msgsnd(fd, &msg, sizeof(msg_t)-sizeof(long), 0) == -1) {
                perror("msgsnd child");
                return;
            }

            close(fildes[0]);
        } else if (!msg.isEof) {
            printf("New process, msg: %s\n", msg.text);
            close(1);
            dup(fildes[1]);
            
            close(2);
            dup(fildes[1]);
            
            close(0);
            close(fildes[0]);
            
            execlp(msg.text, msg.text, NULL);
            perror("execl");
            exit(1);
        }

    } while(!msg.isEof);
}

int main() {
    int fd;

    if ((fd = msgget(IPC_PRIVATE, IPC_CREAT | 0600)) == -1) {
        perror("msgget");
        exit(1);
    }

    if (fork()) { // Parent
        parent(fd);
    } else { // Child
        child(fd);
        msgctl(fd, IPC_RMID, NULL);
    }
    
    return 0;
}
