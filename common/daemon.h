#ifndef DAEMON_H
#define DAEMON_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "client-request.h"
#include "timing-text-io.h"
#include "timing.h"
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>

#ifdef __APPLE__
#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#else
#include <endian.h>
#endif
#include "server-reply.h"
int rm_task(int fd_req);
int terminate_demon(int fd_req);
int create_fifo();
int get_new_task_id();
int get_dates(int fd_req);
int get_arguments(int fd_req);
int create_task(int req_fd);

#endif
