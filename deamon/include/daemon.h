#ifndef DAEMON_H
#define DAEMON_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>

int create_fifo();
int create_tasks_directory();
int get_next_task_id();
int create_task();

#endif
