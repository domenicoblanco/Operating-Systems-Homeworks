/**
 *  reimplementazione del comando 'mv':
 *  limitato al caso di file sullo stesso file-system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_UNIX_PATH_LEN 4096

/**
 * @brief Finds the index of the filename in a path
 * 
 * @param source the string path
 * 
 * @return The beginning of string or 0
 */

size_t findFileNameIndex(const char* source) {
    size_t fileNameIndex = 0;

    for (size_t index=0; index<strlen(source); index++) {
        if (source[index] == '/') {
            fileNameIndex = index+1;
        }
    }

    return fileNameIndex;
}

/**
 * @brief Copies characters in blocks of 1024 bytes into another position
 * 
 * @param source File to be copied
 * @param dest Destination path
 */

void copyAndPaste(char* source, char* dest) {
    size_t sourceLen = strlen(source), destLen = strlen(dest), fileNameIndex = findFileNameIndex(source);
    int8_t result;
    FILE *fileSource, *fileDest;
    char buffer[1024] = {0};
    char destination[destLen+sourceLen-fileNameIndex];

    // Create destination string
    strcpy(destination, dest);
    if (destLen >= 1 && dest[destLen-1] != '/') {
        strcat(destination, "/");
    }
    strcat(destination, source+fileNameIndex);

    // Open files stream
    fileSource = fopen(source, "r");
    fileDest = fopen(destination, "w+");

    do {
        result = fread(buffer, sizeof(char), 1024, fileSource);
        fwrite(buffer, sizeof(char), result, fileDest);
    } while (result);

    // Closes streams
    fclose(fileSource);
    fclose(fileDest);
}

int main(int argc, char *argv[]) {
    uint8_t sourceFd;
    struct stat statbuf;

    if (argc != 3) {
        fprintf(stderr, "Syntax: %s <file> <new-name-or-position>\n", argv[0]);
        exit(1);
    }

    lstat(argv[1], &statbuf);

    if (S_ISLNK(statbuf.st_mode)) { // This is a symbolic link
        printf("This is a symlink\n");
        uint16_t symlinkLength;
        char symlinkPath[MAX_UNIX_PATH_LEN];

        symlinkLength = readlink(argv[1], symlinkPath, MAX_UNIX_PATH_LEN);

        if (symlink(symlinkPath, argv[2]) == -1) {
            perror("symlink");
            exit(1);
        }

    } else {
        printf("This is a regular file\n");
        /* crea un hard-link a partire dal file esistente */
        if (link(argv[1], argv[2]) == -1) {
            printf("Copy & paste\n");
            copyAndPaste(argv[1], argv[2]);
        }
    }

    /* rimuove il vecchio riferimento al file */
    if (unlink(argv[1]) == -1) {
        perror(argv[1]);
        exit(1);
    }

    /* il file è stato spostato e/o rinominato istantaneamente (a prescindere dalla sua dimensione) */
    return 0;
}
