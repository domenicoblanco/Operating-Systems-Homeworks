/**
 *  Scrivere un programma in linguaggio C che permetta di copiare un numero
 *  arbitrario di file regolari su una directory di destinazione preesistente.
 *  Il programma dovrà accettare una sintassi del tipo:
 *  $ homework-1 file1.txt path/file2.txt "nome con spazi.pdf" directory-destinazione 
 */

#include <stdio.h>
#include <string.h>

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

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Syntax %s: fileToCopy destination\n", argv[0]);
        return(1);
    }

    for (int i=1; i<argc-1; i++)
        copyAndPaste(argv[i], argv[argc-1]);

    return 0;
}
