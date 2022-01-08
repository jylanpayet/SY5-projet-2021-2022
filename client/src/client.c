#include "client.h"
#include "cassini.h"

struct timing timing;

int send_ls_request(int p, int b) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_LIST_TASKS);
    if (write(p, &opcode, sizeof(opcode)) == -1) {
        perror("La requête n'a pas pu être exécutée.");
        return (EXIT_FAILURE);
    }
    uint16_t ok;
    if (read(b, &ok, sizeof(ok)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint32_t nbtask;
    if (read(b, &nbtask, sizeof(nbtask)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    int a = be32toh(nbtask);
    while (a > 0) {
        uint64_t taskid;
        if (read(b, &taskid, sizeof(uint64_t)) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
        printf("%llu: ", (long long) be64toh(taskid));
        char *dest = malloc(TIMING_TEXT_MIN_BUFFERSIZE);
        struct timing *time = malloc(sizeof timing);
        uint64_t minutes;
        uint32_t hours;
        uint8_t days;
        if (read(b, &minutes, sizeof(uint64_t)) == -1 || read(b, &hours, sizeof(uint32_t)) == -1 ||
            read(b, &days, sizeof(uint8_t)) == -1) {
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
        printf("%s ", dest);
        uint32_t argc;
        if (read(b, &argc, sizeof(argc)) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
        int q = be32toh(argc);
        free(dest);
        free(time);
        while (q > 0) {
            uint32_t l;
            if (read(b, &l, sizeof(l)) == -1) {
                perror("Erreur.");
                return (EXIT_FAILURE);
            }
            int h = be32toh(l);
            char *content = malloc(h + 1);
            if (read(b, content, h) == -1) {
                perror("Erreur.");
                return (EXIT_FAILURE);
            }
            content[h] = '\0';
            printf("%s ", content);
            free(content);
            q--;
        }
        printf("\n");
        a--;
    }
    return (EXIT_SUCCESS);
}

int send_cr_request(int p, int b, char *minutes_str, char *hours_str, char *daysofweek_str, int argc, char **argv) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_CREATE_TASK);
    struct timing *time = malloc(sizeof(timing));
    if (time == NULL) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    int a = timing_from_strings(time, minutes_str, hours_str, daysofweek_str);
    if (a == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    if (write(p, &opcode, sizeof(opcode)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint64_t m = htobe64(time->minutes);
    uint32_t h = htobe32(time->hours);
    uint8_t d = (time->daysofweek);
    uint32_t c = htobe32(argc - optind);
    if (write(p, &m, sizeof(m)) == -1 || write(p, &h, sizeof(h)) == -1 || write(p, &d, sizeof(d)) == -1 ||
        write(p, &c, sizeof(c)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    for (int i = optind; i < argc; i++) {
        uint32_t t = htobe32(strlen(argv[i]));
        if (write(p, &t, sizeof(t)) == -1 || write(p, argv[i], strlen(argv[i])) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
    }
    uint16_t ok;
    if (read(b, &ok, sizeof(ok)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint64_t taskid;
    if (read(b, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    printf("%llu", (long long) be64toh(taskid));
    free(time);
    return (EXIT_SUCCESS);
}

int send_rm_request(int p, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_REMOVE_TASK);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

int send_info_request(int p, int b, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_GET_TIMES_AND_EXITCODES);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint16_t reptype;
    if (read(b, &reptype, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    if (be16toh(reptype) == SERVER_REPLY_OK) {
        uint32_t nb_runs;
        if (read(b, &nb_runs, sizeof(uint32_t)) == -1) {
            perror("erreur");
            return (EXIT_FAILURE);
        }
        int runs = be32toh(nb_runs);
        int v = 0;
        while (v < runs) {
            int64_t time;
            if (read(b, &time, sizeof(int64_t)) == -1) {
                perror("erreur");
                return (EXIT_FAILURE);
            }

            time_t date = be64toh(time);
            struct tm *tm = localtime(&date);
            printf("%4d-%02d-%02d %02d:%02d:%02d ",
                   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                   tm->tm_hour, tm->tm_min, tm->tm_sec);

            int16_t exit_code;
            if (read(b, &exit_code, sizeof(int16_t)) == -1) {
                perror("erreur");
                return (EXIT_FAILURE);
            }
            printf("%d", be16toh(exit_code));
            printf("\n");
            v++;

        }
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }
}

int send_so_request(int p, int b, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_GET_STDOUT);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint16_t rep;
    if (read(b, &rep, sizeof(int16_t)) == -1) {
        perror("erreur");
        return (EXIT_FAILURE);
    }
    if (be16toh(rep) == SERVER_REPLY_OK) {
        uint32_t l;
        if (read(b, &l, sizeof(l)) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
        int h = be32toh(l);
        char *content = malloc(h + 1);
        if (read(b, content, h) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
        content[h] = '\0';
        printf("%s ", content);
        free(content);
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }
}

int send_se_request(int p, int b, uint64_t taskid) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_GET_STDERR);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    taskid = htobe64(taskid);
    if (write(p, &taskid, sizeof(uint64_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    uint16_t rep;
    if (read(b, &rep, sizeof(int16_t)) == -1) {
        perror("erreur");
        return (EXIT_FAILURE);
    }
    if (be16toh(rep) == SERVER_REPLY_OK) {
        uint32_t l;
        if (read(b, &l, sizeof(l)) == -1) {
            perror("Erreur.");
            return (EXIT_FAILURE);
        }
        int h = be32toh(l);
        char *content = malloc(h + 1);
        if (read(b, content, h) == -1) {
            perror("Erreur.");
            free(content);
            return (EXIT_FAILURE);
        }
        content[h] = '\0';
        printf("%s ", content);
        free(content);
        return (EXIT_SUCCESS);
    } else {
        return (EXIT_FAILURE);
    }
}

int send_tm_request(int p) {
    uint16_t opcode = htobe16(CLIENT_REQUEST_TERMINATE);
    if (write(p, &opcode, sizeof(uint16_t)) == -1) {
        perror("Erreur.");
        return (EXIT_FAILURE);
    }
    return (EXIT_SUCCESS);
}

