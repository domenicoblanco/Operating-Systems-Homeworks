/**
 *  Usando la possibilita' di mappare file in memoria, creare un programma che
 *  possa manipolare un file arbitrariamente grande costituito da una sequenza
 *  di record lunghi N byte.
 *  La manipolazione consiste nel riordinare, tramite un algoritmo di ordinamento
 *  a scelta, i record considerando il contenuto dello stesso come chiave:
 *  ovvero, supponendo N=5, il record [4a a4 91 f0 01] precede [4a ff 10 01 a3].
 *  La sintassi da supportare e' la seguente:
 *    $ homework-3 <N> <pathname del file>
 * 
 *  E' possibile testare il programma sul file 'esempio.txt' prodotto dal seguente
 *  comando, utilizzando il parametro N=33:
 *   $ ( for I in `seq 1000`; do echo $I | md5sum | cut -d' ' -f1 ; done ) > esempio.txt
 *
 *   Su tale file, l'output atteso e' il seguente:
 *   $ homework-3 33 esempio.txt
 *   $ head -n5 esempio.txt
 *      000b64c5d808b7ae98718d6a191325b7
 *      0116a06b764c420b8464f2068f2441c8
 *      015b269d0f41db606bd2c724fb66545a
 *      01b2f7c1a89cfe5fe8c89fa0771f0fde
 *      01cdb6561bfb2fa34e4f870c90589125
 *   $ tail -n5 esempio.txt
 *      ff7345a22bc3605271ba122677d31cae
 *      ff7f2c85af133d62c53b36a83edf0fd5
 *      ffbee273c7bb76bb2d279aa9f36a43c5
 *      ffbfc1313c9c855a32f98d7c4374aabd
 *      ffd7e3b3836978b43da5378055843c67
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

u_int16_t buffSize;

int cmp(const void *s1, const void *s2) {
    return memcmp(s1, s2, buffSize);
}

int main(int argc, char *argv[]) {
    int8_t fd;
    struct stat statbuff;
    void *mappedAddress;

    if (argc != 3) {
        printf("Syntax: %s <N> <pathname>\n", argv[0]);
        return 0;
    }

    if ((fd = open(argv[2], O_RDWR)) == -1) {
        perror(argv[2]);
        exit(1);
    }

    // Getting file size
    if (fstat(fd, &statbuff) == -1) {
        perror("stat");
        exit(1);
    }

    // Parsing first argument & checking it's value
    if ((buffSize = strtol(argv[1], NULL, 0)) == 0 || buffSize > statbuff.st_size) {
        perror("strtol");
        exit(1);
    }

    // Memory mapping
    if ((mappedAddress = mmap(NULL, statbuff.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        perror("Failed memory mapping");
        exit(1);
    }

    if (close(fd) == -1) {
        perror("close");
        exit(1);
    }

    long long unsigned fileLength = statbuff.st_size/buffSize;
    qsort(mappedAddress, fileLength, buffSize*sizeof(char), cmp);

    printf("$ head -n5 input.txt\n");
    for (int j=0; j<buffSize*5; j++)
        printf("%c", ((char*)mappedAddress)[j]);

    printf("$ tail -n5 input.txt\n");
    for (int j=statbuff.st_size-5*buffSize; j<statbuff.st_size; j++)
        printf("%c", ((char*)mappedAddress)[j]);

    munmap(mappedAddress, statbuff.st_size);
    return 0;
}
