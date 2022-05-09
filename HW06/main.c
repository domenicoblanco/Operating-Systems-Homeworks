#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>

#define bool u_int8_t
#define true 1
#define false 0
#define msgMax 1024
#define queuePath "/tmp/myFifo"

typedef struct {
    long type;
    char text[msgMax+1];
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
    } while(!msg.isEof);
}

void child(int fd) {
    msg_t msg;

    do {
        if (msgrcv(fd, &msg, sizeof(msg_t)-sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            msgctl(fd, IPC_RMID, NULL);
            exit(1);
        }

        printf("Received message: %s\n", msg.text);

        if (!msg.isEof && !fork()) {
            printf("New process\n");
            execlp(msg.text, msg.text, NULL);
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