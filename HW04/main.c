/**
 *  Estendere l'esercizio 'homework n.1' affinche' operi correttamente
 *  anche nel caso in cui tra le sorgenti e' indicata una directory, copiandone
 *  il contenuto ricorsivamente. Eventuali link simbolici incontrati dovranno
 *  essere replicati come tali (dovrà essere creato un link e si dovranno
 *  preservare tutti permessi di accesso originali dei file e directory).
 *
 *  Una ipotetica invocazione potrebbe essere la seguente:
 *  $ homework-4 directory-di-esempio file-semplice.txt path/altra-dir/ "nome con spazi.pdf" directory-destinazione
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>


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
 * @brief Adds trailing slash if not already in string
 * 
 * @param destination Path to fix
 */
void fixDestination(char *destination) {
    u_int16_t destLen = strlen(destination);
    if (destLen >= 1 && destination[destLen-1] != '/') {
        strncat(destination, "/", PATH_MAX);
    }
}

/**
 * @brief Converts a relative path to an absolute path
 * 
 * @param path The path to fix
 */

void relativeToAbsolute(char *pathToFix) {
    if (pathToFix[0] == '/') {
        return;
    }

    size_t pathIndexRelativeSymlink = findFileNameIndex(pathToFix);
    char currentDirectory[PATH_MAX];
    char buffer[PATH_MAX];
    char relativePathSymlink[pathIndexRelativeSymlink];
    relativePathSymlink[pathIndexRelativeSymlink] = '\0';

    strncpy(relativePathSymlink, pathToFix, pathIndexRelativeSymlink);
    if (!getcwd(currentDirectory, PATH_MAX)) {
        perror("getcwd");
        exit(1);
    }

    printf("Origin: %s\n", pathToFix);
    printf("Relative path: %s\n", relativePathSymlink);
    if (strcmp(relativePathSymlink, "") == 0) {
        strcpy(relativePathSymlink, ".");
    }
    
    if (chdir(relativePathSymlink) == -1) {
        perror("chdir");
        exit(1);
    }

    if (!getcwd(buffer, PATH_MAX)) {
        perror("getcwd");
        exit(1);
    }
    
    sprintf(pathToFix, "%s%c%s", buffer, '/', pathToFix+pathIndexRelativeSymlink);
    // strcat(buffer, "/");
    // strcat(buffer, pathToFix+pathIndexRelativeSymlink);
    // strcpy(pathToFix, buffer);

    if (chdir(currentDirectory) == -1) {
        perror("chdir");
        exit(1);
    }
}

/**
 * @brief Copies characters in blocks of 1024 bytes into another position
 * 
 * @param source File to be copied
 * @param dest Destination path
 */

void copyAndPaste(char* source, char* dest, mode_t st_mode) {
    size_t sourceLen = strlen(source), destLen = strlen(dest), fileNameIndex = findFileNameIndex(source);
    int8_t result;
    FILE *fileSource, *fileDest;
    char buffer[1024] = {0};
    char destination[destLen+sourceLen-fileNameIndex];

    // Create destination string
    strncpy(destination, dest, PATH_MAX);
    fixDestination(destination);
    strncat(destination, source+fileNameIndex, PATH_MAX);

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

    if (chmod(destination, st_mode) == -1) {
        perror("chmod");
        exit(1);
    }
}

void manageCopy(char *src, char *dst) {
    char newFileDirPath[PATH_MAX];
    struct stat statbuff;

    if (lstat(src, &statbuff) == -1) {
        perror("lstat");
        exit(1);
    }

    switch(statbuff.st_mode & S_IFMT) {
        case S_IFLNK: {
            u_int16_t symlinkLength;
            size_t symlinkName = findFileNameIndex(src);
            char symlinkPath[PATH_MAX];

            if (!(symlinkLength=readlink(src, symlinkPath, PATH_MAX))) {
                perror("symlink");
                exit(1);
            }
            symlinkPath[symlinkLength] = '\0';
            
            strncpy(newFileDirPath, dst, PATH_MAX);
            fixDestination(newFileDirPath);
            strncat(newFileDirPath, src+symlinkName, PATH_MAX);
            relativeToAbsolute(symlinkPath);

            printf("Symlink: %s → %s\n", newFileDirPath, symlinkPath);
            
            if (symlink(symlinkPath, newFileDirPath) == -1) {
                perror("symlink");
                exit(1);
            }

            break;
        }
        case S_IFREG:
            printf("This is a regular file\n");
            copyAndPaste(src, dst, statbuff.st_mode);
            break;
        case S_IFDIR: {
            size_t pathNameIndex = findFileNameIndex(src);
            DIR* newDirectory;
            char currentViewingContent[PATH_MAX];
            struct dirent *dirEntity;

            strncpy(newFileDirPath, dst, PATH_MAX);
            fixDestination(newFileDirPath);
            strncat(newFileDirPath, src+pathNameIndex, PATH_MAX);

            if (mkdir(newFileDirPath, statbuff.st_mode) == -1) {
                perror("mkdir");
                exit(1);
            }

            if (!(newDirectory=opendir(src))) {
                perror("opendir");
                exit(1);
            }

            while ((dirEntity=readdir(newDirectory))) {
                if (strcmp(dirEntity->d_name, ".") && strcmp(dirEntity->d_name, "..")) {
                    printf("Inside this dir there is %s\n", dirEntity->d_name);
                    strncpy(currentViewingContent, src, PATH_MAX);
                    strncat(currentViewingContent, "/", PATH_MAX);
                    strncat(currentViewingContent, dirEntity->d_name, PATH_MAX);
                    manageCopy(currentViewingContent, newFileDirPath);
                }
            }

            if (closedir(newDirectory) == -1) {
                perror("closedir");
                exit(1);
            }
        }
            
    }

}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        printf("Syntax %s: fileToCopy destination\n", argv[0]);
        return 1;
    }

    for (int inputParam=1; inputParam<argc-1; inputParam++)
        manageCopy(argv[inputParam], argv[argc-1]);

    return 0;
}
