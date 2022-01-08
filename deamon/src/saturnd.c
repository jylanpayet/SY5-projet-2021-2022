#include <unistd.h>
#include <stdio.h>


int main(){
    int r;
    r = fork();
    switch(r) {
        case -1 :
            perror("fork");
        case 0 : // fils
            printf("je suis le fils, mon pid est %d, celui de mon père %d\n",getpid(), getppid());
            while(1)
            {
                    //break;
            }
            break;
        default : // père
            printf("je suis le père, de pid %d, je viens de créer un fils de pid %d\n",getpid(), r);
            return 0;
    }
}

