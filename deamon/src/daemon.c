#include "daemon.h"

int create_fifo(){
    char *directory;
    char request [] = "saturnd-request-pipe";
    char reply [] = "saturnd-reply-pipe";
    struct stat st;

    if (chdir("/tmp") != 0){
        perror("/tmp");
        exit(EXIT_FAILURE);
    }
    asprintf(&directory,"/%s", getenv("USER"));
    if (stat(getenv("USER"),&st)== -1) {
        mkdir(getenv("USER"), 0755);
    }
    asprintf(&directory,"/tmp/%s", getenv("USER"));
    if (chdir(directory) != 0){
        perror("/tmp/user");
        exit(EXIT_FAILURE);
    }
    if (stat("pipes", &st) == -1) {
        mkdir("pipes", 0755);
    }
    asprintf(&directory,"/tmp/%s/pipes", getenv("USER"));
    if (chdir(directory) != 0){
        perror("/tmp/user/pipes");
        exit(EXIT_FAILURE);
    }

    if(stat(reply,&st)==-1){
        if(mkfifo(reply,S_IRUSR | S_IWUSR)==-1){
            perror("pipes");
            exit(EXIT_FAILURE);
        }
    }
    if(stat(request,&st)==-1){
        if(mkfifo(request,S_IRUSR | S_IWUSR)==-1){
            perror("pipes");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}
