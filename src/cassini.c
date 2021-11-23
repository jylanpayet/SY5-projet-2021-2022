#include "cassini.h"

const char usage_info[] = "\
   usage: cassini [OPTIONS] -l -> list all tasks\n\
      or: cassini [OPTIONS]    -> same\n\
      or: cassini [OPTIONS] -q -> terminate the daemon\n\
      or: cassini [OPTIONS] -c [-m MINUTES] [-H HOURS] [-d DAYSOFWEEK] COMMAND_NAME [ARG_1] ... [ARG_N]\n\
          -> add a new task and print its TASKID\n\
             format & semantics of the \"timing\" fields defined here:\n\
             https://pubs.opengroup.org/onlinepubs/9699919799/utilities/crontab.html\n\
             default value for each field is \"*\"\n\
      or: cassini [OPTIONS] -r TASKID -> remove a task\n\
      or: cassini [OPTIONS] -x TASKID -> get info (time + exit code) on all the past runs of a task\n\
      or: cassini [OPTIONS] -o TASKID -> get the standard output of the last run of a task\n\
      or: cassini [OPTIONS] -e TASKID -> get the standard error\n\
      or: cassini -h -> display this message\n\
\n\
   options:\n\
     -p PIPES_DIR -> look for the pipes in PIPES_DIR (default: /tmp/<USERNAME>/saturnd/pipes)\n\
";

void send_ls_request(char *pipe) {
    int req_pipe = open(pipe, O_WRONLY);
    if (req_pipe == -1) {
        perror("Erreur lors de l'ouverture du tube requête.");
        errno = 1;
        goto error;
    }

    u_int16_t OPCODE = CLIENT_REQUEST_LIST_TASKS;
    if (write(req_pipe, &OPCODE, sizeof(u_init16_t)) < 0) {
        perror("write error");
        errno = 1;
        goto error;
    }
    close(req_pipe);
    exit(EXIT_SUCCESS);
}

void set_pipe_dir(char *pipe_dir) {
    char *user = getenv("USER");
    if (pipe_dir[0] == '\0' || pipe_dir == NULL) {
        strcat(pipe_dir, "/tmp/");
        strcat(pipe_dir, user);
        strcat(pipe_dir, "/saturnd/pipes");
    }
}

int main(int argc, char *argv[]) {
    errno = 0;

    char *minutes_str = "*";
    char *hours_str = "*";
    char *daysofweek_str = "*";
    char *pipes_directory = NULL;

    u_int16_t operation = CLIENT_REQUEST_LIST_TASKS;
    u_int64_t taskid;

    int opt;
    char *strtoull_endp;
    while ((opt = getopt(argc, argv, "hlcqm:H:d:p:r:x:o:e:")) != -1) {
        switch (opt) {
            case 'm':
                minutes_str = optarg;
                break;
            case 'H':
                hours_str = optarg;
                break;
            case 'd':
                daysofweek_str = optarg;
                break;
            case 'p':
                pipes_directory = strdup(optarg);
                if (pipes_directory == NULL) goto error;
                break;
            case 'l':
                operation = CLIENT_REQUEST_LIST_TASKS;
                break;
            case 'c':
                operation = CLIENT_REQUEST_CREATE_TASK;
                break;
            case 'q':
                operation = CLIENT_REQUEST_TERMINATE;
                break;
            case 'r':
                operation = CLIENT_REQUEST_REMOVE_TASK;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'x':
                operation = CLIENT_REQUEST_GET_TIMES_AND_EXITCODES;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'o':
                operation = CLIENT_REQUEST_GET_STDOUT;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'e':
                operation = CLIENT_REQUEST_GET_STDERR;
                taskid = strtoull(optarg, &strtoull_endp, 10);
                if (strtoull_endp == optarg || strtoull_endp[0] != '\0') goto error;
                break;
            case 'h':
                printf("%s", usage_info);
                return 0;
            case '?':
                fprintf(stderr, "%s", usage_info);
                goto error;
        }
    }
    struct stat st1;
    if (stat(strcat(pipes_directory, "saturnd-request-pipe"), &st1) == -1) {
        fprintf(stderr, "le tube requête n'exite pas");
        errno = 1;
        goto error;
    }
    int req_pipe = open(strcat(pipes_directory, "sturnd-request-pipe"), O_WRONLY);
    send_ls_request(req_pipe);
    struct stat st;
    if (stat(strcat(pipes_directory, "saturnd-reply-pipe"), &st) == -1) {
        fprintf(stderr, "le tube réponse n'exite pas");
        errno = 1;
        goto error;
    }
    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS:
            send_ls_request(strcat(pipes_directory, "saturnd-reply-pipe"));
            break;
        case CLIENT_REQUEST_CREATE_TASK :
            break;
        case CLIENT_REQUEST_REMOVE_TASK :
            break;
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
            break;
        case CLIENT_REQUEST_TERMINATE :
            break;
        case CLIENT_REQUEST_GET_STDOUT :
            break;
        default break;
    }
    set_pipe_dir(pipes_directory);
    // --------
    // | TODO |
    // --------

    return EXIT_SUCCESS;

    error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}

