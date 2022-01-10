#include "daemon.h"
struct timing timing;

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
    if (stat("tasks", &st) == -1) {
        mkdir("tasks", 0755);
    }
    asprintf(&directory,"/tmp/%s/saturnd/pipes", getenv("USER"));
    if (chdir(directory) != 0){
        perror("/tmp/user/saturnd/pipes");
        free(directory);
        exit(EXIT_FAILURE);
    }
    if(stat(reply,&st)==-1){
        if(mkfifo(reply,0755)==-1){
            perror("pipes");
            free(directory);
            exit(EXIT_FAILURE);
        }
    }
    if(stat(request,&st)==-1){
        if(mkfifo(request,0755)==-1){
            perror("pipes");
            free(directory);
            exit(EXIT_FAILURE);
        }
    }
    chdir("/");
    free(directory);
    return 0;
}

int get_new_task_id(){
    char *path = NULL;
    asprintf(&path,"/tmp/%s/saturnd/tasks", getenv("USER"));
    DIR *dirp = opendir(path);
    if(dirp == NULL){
        switch (errno){
            case EACCES:
                exit(EXIT_FAILURE);
            case ENOENT:
                perror("le répertoire tasks n'existe pas");
                exit(EXIT_FAILURE);
        }
    }
    int ids[257] = {0};
    struct dirent *entry;
    int i = 1;
    ids[0]= -1;
    while ((entry = readdir(dirp))){
        char name[256];
        strcpy(name,(entry->d_name));
        if((name[0]) != '.' && (entry->d_type) == DT_DIR ){
            ids[i] = atoi(name);
            if(ids[i] > ids[0])
                ids[0] = ids[i];
            i++;
        }
    }
    int next_id = ids[0]+1;
    closedir(dirp);
    return next_id;
}



int get_dates(int fd_req){
    int date_fd = open("date",O_CREAT | O_WRONLY,0600);
    char *dest = malloc(TIMING_TEXT_MIN_BUFFERSIZE);
    struct timing *time = malloc(sizeof timing);
    uint64_t minutes;
    uint32_t hours;
    uint8_t days;
    if (read(fd_req, &minutes, sizeof(uint64_t)) == -1 || read(fd_req, &hours, sizeof(uint32_t)) == -1 ||
        read(fd_req, &days, sizeof(uint8_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    time->minutes = be64toh(minutes);
    time->hours = be32toh(hours);
    time->daysofweek = days;
    int r = timing_string_from_timing(dest, time);
    if (r == 0) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    printf("%s\n",dest);
    // N'écrit pas sur le fichier : donc checker les droits ect ...
    write(date_fd,dest,r);
    close(date_fd);
    return 0;
}

int get_arguments(int fd_req){
    int arguments_fd = open("argument",O_CREAT | O_WRONLY,0600);
    uint32_t c;
    if (read(fd_req, &c, sizeof(c)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    for (int i = 0; i < be32toh(c); i++){
        uint32_t t;
        if (read(fd_req, &t, sizeof(t)) == -1){
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        char *curr_arg = malloc(be32toh(t)+1);
        if (read(fd_req, curr_arg, be32toh(t)) == -1){
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        curr_arg [be32toh(t)] = '\n';
        if(write(arguments_fd,curr_arg, sizeof(curr_arg)) == -1){
            perror("erreur write");
            exit(EXIT_FAILURE);
        }
    }
    close(arguments_fd);
    return (0);
}

int create_task(int req_fd){
    char *path = NULL;
    asprintf(&path,"/tmp/%s/saturnd/tasks", getenv("USER"));
    chdir(path);
    int new_id =get_new_task_id();
    char buff[256];
    sprintf(buff,"%s/%d",path,new_id);
    mkdir(buff,0700);
    chdir(buff);

    //tester chaque retour de fonction.
    get_dates(req_fd);
    //get_arguments(req_fd);

    //TODO -> effectuer les taches pour stocker les résultats dans ces deux fichiers
    open("stdout",O_CREAT,0600);
    open("exitcode",O_CREAT,0600);
    printf("fin de fonction\n");

    /*

* -> Ecrire sur le reply la reponse attendu

    char *reply;
    asprintf(&reply,"/tmp/%s/saturnd/pipes/saturnd-request-pipe",getenv("USER"));
    int p = open(reply,O_NONBLOCK,O_RDONLY);
    */
    return 0;
}
