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
        free(directory);
        exit(EXIT_FAILURE);
    }
    if (stat("saturnd", &st) == -1) {
        mkdir("saturnd", 0755);
    }
    asprintf(&directory,"/tmp/%s/saturnd", getenv("USER"));
    if (chdir(directory) != 0){
        perror("/tmp/user/saturnd");
        free(directory);
        exit(EXIT_FAILURE);
    }
    if (stat("pipes", &st) == -1) {
        mkdir("pipes", 0755);
    }
    asprintf(&directory,"/tmp/%s/saturnd/pipes", getenv("USER"));
    if (chdir(directory) != 0){
        perror("/tmp/user/saturnd/pipes");
        free(directory);
        exit(EXIT_FAILURE);
    }

    if(stat(reply,&st)==-1){
        if(mkfifo(reply,S_IRUSR | S_IWUSR)==-1){
            perror("pipes");
            free(directory);
            exit(EXIT_FAILURE);
        }
    }
    if(stat(request,&st)==-1){
        if(mkfifo(request,S_IRUSR | S_IWUSR)==-1){
            perror("pipes");
            free(directory);
            exit(EXIT_FAILURE);
        }
    }
    free(directory);
    return 0;
}

int create_tasks_directory()
{
    int res = mkdir("tasks",0755);
    if(res == -1)
    {
        switch (errno)
        {
            case EACCES :
            {
                exit(EXIT_FAILURE);
            }
            case EEXIST:
            {
                return(0);
            }
            default:
                perror("mkdir");
                return (1);
        }
    }
    return 0;
}

int max_tab(int  tab[], int n)
{
    int max = tab[0];
    for (int i = 1; i < n; ++i)
    {
        if (tab[i] > max) max = tab[i];
    }
    return max;
}

int get_next_task_id()
{
    //int errno = 0;
    DIR *dirp = opendir("tasks");
    if(dirp == NULL)
    {
        switch (errno)
        {
            case EACCES:
                exit(EXIT_FAILURE);
            case ENOENT:
                perror("le répertoire tasks n'existe pas");
                exit(EXIT_FAILURE);
        }
    }
    int ids[256]= {0};
    struct dirent *entry;
    int i = 0;
    while ((entry = readdir(dirp)))
    {
        char name[256];
        strcpy(name,(entry->d_name));
        if((name[0]) != '.' && (entry->d_type) == DT_DIR )
        {
            ids[i] = atoi(name);
            i++;
        }
    }
    int next_id = max_tab(ids,256) + 1;
    closedir(dirp);
    return next_id;
}
