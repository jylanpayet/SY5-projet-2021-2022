#include "saturnd.h"
#define TIME_OUT -1

int main(){
    int r;
    create_fifo();
    r = fork();
    switch(r) {
        case -1 :
            perror("fork");
            return 1;
        case 0 : // fils
            if(setsid() < 0){
                perror("setsid");
                exit(1);
            }
            char *request;

            printf("fils %d -> père %d\n",getpid(), getppid());
            asprintf(&request,"/tmp/%s/saturnd/pipes/saturnd-request-pipe", getenv("USER"));

            int fd = open(request,O_RDONLY);
            free(request);
            if (fd == -1){
                perror("open");
                return 1;
            }
            while(1){
                uint16_t opcode;
                if(read(fd,&opcode,sizeof (opcode))==-1){
                        perror("Le read à échoué");
                        return 1;
                    }
                    switch (be16toh(opcode)) {
                        case CLIENT_REQUEST_LIST_TASKS:
                            //TODO
                            break;
                        case CLIENT_REQUEST_CREATE_TASK :
                            //TODO
                            create_task(fd);
                            break;
                        case CLIENT_REQUEST_REMOVE_TASK :
                            //TODO
                            rm_task(fd);
                            break;
                        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
                            //TODO
                            break;
                        case CLIENT_REQUEST_GET_STDOUT :
                            //TODO
                            break;
                        case CLIENT_REQUEST_GET_STDERR :
                            //TODO
                            break;
                        case CLIENT_REQUEST_TERMINATE :
                            terminate_demon(fd);
                            return 0;
                        default:
                            perror("Erreur rencontrée");
                            return 1;
                    }
            }
        default : // père
            printf("père %d -> fils %d\n",getpid(), r);
            return 0;
    }
}

