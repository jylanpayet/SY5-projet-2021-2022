#include "cassini.h"
#include "timing-text-io.c"

struct timing timing;
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

void send_ls_request(int p, int b) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_LIST_TASKS);
    if (write(p, &opcode, sizeof(opcode)) == -1) {
        perror("La requête n'a pas pu être exécutée.");
        exit(EXIT_FAILURE);
    }
    uint16_t ok;
    if (read(b, &ok, sizeof(ok)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    uint32_t nbtask;
    if (read(b, &nbtask, sizeof(nbtask)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    int a = be32toh(nbtask);
    while (a > 0) {
        uint64_t taskid;
        if (read(b, &taskid, sizeof(uint64_t)) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        printf("%llu: ", be64toh(taskid));
        char *dest = malloc(TIMING_TEXT_MIN_BUFFERSIZE);
        struct timing *time = malloc(sizeof timing);
        uint64_t minutes;
        uint32_t hours;
        uint8_t days;
        if (read(b, &minutes, sizeof(uint64_t)) == -1 || read(b, &hours, sizeof(uint32_t)) == -1 ||
            read(b, &days, sizeof(uint8_t)) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        time->minutes = be64toh(minutes);
        time->hours = be32toh(hours);
        time->daysofweek = days;
        int r = timing_string_from_timing(dest, time);
        if (r == 0) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        printf("%s ", dest);
        uint32_t argc;
        if (read(b, &argc, sizeof(argc)) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        int q = be32toh(argc);
        while (q > 0) {
            uint32_t l;
            if (read(b, &l, sizeof(l)) == -1) {
                perror("Erreur.");
                exit(EXIT_FAILURE);
            }
            int h = be32toh(l);
            char *content = malloc(h);
            if (read(b, content, h) == -1) {
                perror("Erreur.");
                exit(EXIT_FAILURE);
            }
            printf("%s ", content);
            q--;
        }
        printf("\n");
        a--;
    }
    exit(EXIT_SUCCESS);
}

void send_cr_request(int p, int b, char *minutes_str, char *hours_str, char *daysofweek_str, int argc, char **argv) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_CREATE_TASK);
    struct timing *time = malloc(sizeof(timing));
    if (time == NULL) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    int a = timing_from_strings(time, minutes_str, hours_str, daysofweek_str);
    if (a == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    if (write(p, &opcode, sizeof(opcode)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    uint64_t m = htobe64(time->minutes);
    uint32_t h = htobe32(time->hours);
    uint8_t d = (time->daysofweek);
    uint32_t c = htobe32(argc - optind);
    if (write(p, &m, sizeof(m)) == -1 || write(p, &h, sizeof(h)) == -1 || write(p, &d, sizeof(d)) == -1 ||
        write(p, &c, sizeof(c)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    for (int i = optind; i < argc; i++) {
        uint32_t t = htobe32(strlen(argv[i]));
        if (write(p, &t, sizeof(t)) == -1 || write(p, argv[i], strlen(argv[i])) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
    }
    uint16_t ok;
    if (read(b, &ok, sizeof(ok)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    uint64_t taskid;
    if (read(b, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
#ifdef __APPLE__
    printf("%llu",be64toh(taskid));

#else
    printf("%lu", be64toh(taskid));
#endif
    free(time);
    exit(EXIT_SUCCESS);
}

void send_rm_request(int p, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_REMOVE_TASK);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void send_info_request(int p, int b, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_GET_TIMES_AND_EXITCODES);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    uint16_t reptype;
    if (read(b, &reptype, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    if (be16toh(reptype) == SERVER_REPLY_OK) {
        uint32_t nb_runs;
        if (read(b, &nb_runs, sizeof(uint32_t)) == -1) {
            perror("erreur");
            exit(EXIT_FAILURE);
        }
        int runs = be32toh(nb_runs);
        int v = 0;
        while (v < runs) {
            int64_t time;
            if (read(b, &time, sizeof(int64_t)) == -1) {
                perror("erreur");
                exit(EXIT_FAILURE);
            }

            time_t date = be64toh(time);
            struct tm *tm = localtime(&date);
            printf("%4d-%02d-%02d %02d:%02d:%02d ",
                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec);

            int16_t exit_code;
            if (read(b, &exit_code, sizeof(int16_t)) == -1) {
                perror("erreur");
                exit(EXIT_FAILURE);
            }
            printf("%d", be16toh(exit_code));
            printf("\n");
            free(str);
            v++;

        }
    }
}

void send_so_request(int p, int b, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_GET_STDOUT);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        exit(EXIT_FAILURE);
    }
    uint16_t rep;
    if (read(b, &rep, sizeof(int16_t)) == -1) {
        perror("erreur");
        exit(EXIT_FAILURE);
    }
    if (be16toh(rep) == SERVER_REPLY_OK) {
        uint32_t l;
        if (read(b, &l, sizeof(l)) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        int h = be32toh(l);
        char *content = malloc(h);
        if (read(b, content, h) == -1) {
            perror("Erreur.");
            exit(EXIT_FAILURE);
        }
        printf("%s ", content);
    }
    exit(EXIT_SUCCESS);
}

void set_pipe_dir(char *pipe_dir) {
    char *user = getenv("USER");
    strcat(pipe_dir, "/tmp/");
    strcat(pipe_dir, user);
    strcat(pipe_dir, "/saturnd/pipes");
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
    int p = open("./run/pipes/saturnd-request-pipe", O_WRONLY);
    int b = open("./run/pipes/saturnd-reply-pipe", O_RDONLY);
    if (p == -1 || b == -1) {
        errno = 1;
        goto error;
    }

    switch (operation) {
        case CLIENT_REQUEST_LIST_TASKS:
            send_ls_request(p, b);
            break;
        case CLIENT_REQUEST_CREATE_TASK :
            send_cr_request(p, b, minutes_str, hours_str, daysofweek_str, argc, argv);
            break;
        case CLIENT_REQUEST_REMOVE_TASK :
            send_rm_request(p, taskid);
            break;
        case CLIENT_REQUEST_GET_TIMES_AND_EXITCODES :
            send_info_request(p, b, taskid);
            break;
        case CLIENT_REQUEST_GET_STDOUT :
            send_so_request(p, b, taskid);
            break;
        case CLIENT_REQUEST_GET_STDERR :
            break;
        case CLIENT_REQUEST_TERMINATE :
            break;
        default :
            break;
    }

    // --------
    // | TODO |
    // --------

    close(p);
    close(b);
    return EXIT_SUCCESS;

    error:
    if (errno != 0) perror("main");
    free(pipes_directory);
    pipes_directory = NULL;
    return EXIT_FAILURE;
}

