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

int max_tab(const int  tab[], int n)
{
    int max = tab[0];
    for (int i = 1; i < n; ++i)
    {
        if (tab[i] > max) max = tab[i];
    }
    return max;
}

int get_new_task_id()
{
    //Mettre le path dans opendir
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



int get_dates(int fd_req)
{
    int date_fd = open("date",O_CREAT,0600);
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
    time -> minutes = be64toh(minutes);
    time->hours = be32toh(hours);
    time->daysofweek = days;
    int r = timing_string_from_timing(dest, time);
    if (r == 0) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    write(date_fd,dest,TIMING_TEXT_MIN_BUFFERSIZE);
    return 0;
}

int get_arguments(int fd_req){
    int arguments_fd = open("argument",O_CREAT,0600);
    uint32_t c;
    char *curr_arg = malloc(10);
    if (read(fd_req, &c, sizeof(c)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    for (int i = 0; i < be32toh(c); i++)
    {
        uint32_t t;
        if (read(fd_req, &t, sizeof(t)) == -1)
        {
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        int t1 = be32toh(t);
        curr_arg = realloc(curr_arg,t);
        if (read(fd_req, curr_arg, t1) == -1)
        {
            perror("read error!");
            exit(EXIT_FAILURE);
        }
        if(write(arguments_fd,curr_arg,t) == -1)
        {
            perror("erreur write");
            exit(EXIT_FAILURE);
        }
        /*
        if(write(arguments_fd,'\n',1) == -1)
        {
            perror("erreur write");
            exit(EXIT_FAILURE);
        }
         */
    }
    return (0);
}

int create_task(int req_fd){
    char *path = NULL;
    asprintf(&path,"/tmp/%s/saturnd/tasks", getenv("USER"));
    printf("path pour create task : %s\n",path);
    chdir(path);
    printf("juste avant malloc\n");
    int new_id = get_new_task_id();
    printf("%d\n",new_id);
    char buff[256];


    sprintf(buff,"%d",new_id);

    mkdir(buff,0700);
    chdir(buff);
/*if(get_dates(req_fd) != 0)
{
    perror("la date na pas été écrite");
    errno = 1;
}*/
    open("stdout",O_CREAT,0600);
    open("arguments",O_CREAT,0600);
    open("exitcode",O_CREAT,0600);// fin de la creation des fichiers vides
    printf("fin de fonction\n");
    return 0;
    //uint64 uint32 uint8  time à ecrire dans date
    //unit32 nb args
    //uint32 longeur
    //suite de longeur
}
