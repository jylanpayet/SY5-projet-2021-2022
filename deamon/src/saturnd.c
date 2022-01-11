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
            struct pollfd fds [1];
            char *request;

            printf("fils %d -> père %d\n",getpid(), getppid());
            asprintf(&request,"/tmp/%s/saturnd/pipes/saturnd-request-pipe", getenv("USER"));

            fds[0].fd = open(request,O_NONBLOCK,O_RDONLY);
            fds[0].events = POLLIN;
            free(request);
            if (fds[0].fd == -1){
                perror("open");
                return 1;
            }
            while(1){
                uint16_t opcode;
                int ret = poll(fds, 1, TIME_OUT);
                if (ret < 0){
                    perror("Le poll à échoué");
                    return 1;
                }
                if(fds[0].revents & POLLIN){
                    if(read(fds[0].fd,&opcode,sizeof (opcode))==-1){
                        perror("Le read à échoué");
                        return 1;
                    }

                    switch (be16toh(opcode)) {
                        case CLIENT_REQUEST_LIST_TASKS:
                            //TODO
                            break;
                        case CLIENT_REQUEST_CREATE_TASK :
                            create_task(fds[0].fd);
                            break;
                        case CLIENT_REQUEST_REMOVE_TASK :
                            rm_task(fds[0].fd);
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
                            terminate_demon(fds[0].fd);
                            close(fds[0].fd);
                            return 0;
                        default:
                            perror("Erreur rencontrée");
                            return 1;
                    }

                }
            }
        default : // père
            printf("père %d -> fils %d\n",getpid(), r);
            return 0;
    }
}