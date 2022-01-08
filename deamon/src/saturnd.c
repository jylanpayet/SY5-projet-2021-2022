#include "saturnd.h"

int main(){
    int r;
    create_fifo();
    create_tasks_directory();
    r = fork();
    switch(r) {
        case -1 :
            perror("fork");
        case 0 : // fils
            printf("je suis le fils, mon pid est %d, celui de mon père %d\n",getpid(), getppid());
            char *request;
            char *reply;
            char *pipes_directory;
            if (pipes_directory == NULL) {
                asprintf(&pipes_directory, "/tmp/%s/saturnd/pipes", getenv("USER"));
            }

            asprintf(&request, "%s/saturnd-request-pipe", pipes_directory);
            asprintf(&reply, "%s/saturnd-reply-pipe", pipes_directory);
            int p = open(request, O_RDONLY);
            int b = open(reply,O_WRONLY);
            if (p == -1 || b == -1) {
                free(request);
                free(reply);
                exit(1);
            }
            while(1)
            {
                char *req = malloc(10);
                printf("j'essaye de lire...");
                if (read(p,req,1) ==-1)
                {
                    perror("jarrive pas a lire");
                    //exit(1);
                }
                printf("j'ai lu %s",req);
                free(req);
            }
            break;
        default : // père
            printf("je suis le père, de pid %d, je viens de créer un fils de pid %d\n",getpid(), r);
            return 0;
    }
}

