#include "saturnd.h"

int main(){
    int r;
    create_fifo();
    create_task();
    //create_task();
    printf("taskid depuis le main : %d\n",get_next_task_id());
    r = fork();
    switch(r) {
        case -1 :
            perror("fork");
            return 1;
        case 0 : // fils
            if (setsid() < 0)
                exit(EXIT_FAILURE);

            printf("je suis le fils, mon pid est %d, celui de mon père %d\n",getpid(), getppid());
            char *request= NULL;
            char *reply = NULL;
            char *pipes_directory = NULL;
            asprintf(&pipes_directory, "/tmp/%s/saturnd/pipes", getenv("USER"));
            asprintf(&request, "%s/saturnd-request-pipe", pipes_directory);
            asprintf(&reply, "%s/saturnd-reply-pipe", pipes_directory);
            printf("avant create");
            printf("apres create");
            /*int p = open(request, O_RDONLY);
            int b = open(reply,O_WRONLY);
            if (p == -1 || b == -1) {
                free(request);
                free(reply);
                exit(1);
            }*/
            while(1){
                //À Completer
                break;
            }
            break;
        default : // père
            printf("je suis le père, de pid %d, je viens de créer un fils de pid %d\n",getpid(), r);
            return 0;
    }
}

