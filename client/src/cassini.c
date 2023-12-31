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

    char *request;
    char *reply;
    if (pipes_directory == NULL) {
        asprintf(&pipes_directory, "/tmp/%s/saturnd/pipes", getenv("USER"));
    }
    asprintf(&request, "%s/saturnd-request-pipe", pipes_directory);
    asprintf(&reply, "%s/saturnd-reply-pipe", pipes_directory);

    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS:
            if (send_ls_request(request,reply) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_CREATE_TASK :
            if (send_cr_request(request,reply, minutes_str, hours_str, daysofweek_str, argc, argv) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_REMOVE_TASK :
            if (send_rm_request(request,reply, taskid) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
            if (send_info_request(request,reply, taskid) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_GET_STDOUT :
            if (send_so_request(request,reply, taskid) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_GET_STDERR :
            if (send_se_request(request,reply, taskid) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
        case CLIENT_REQUEST_TERMINATE :
            if (send_tm_request(request,reply) == 1) {
                free(request);
                free(reply);
                errno = 1;
                goto error;
            }
            break;
    }

    // --------
    // | TODO |
    // --------

    free(request);
    free(reply);
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_SUCCESS;

    error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}
