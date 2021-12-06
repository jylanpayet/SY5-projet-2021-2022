#ifndef CASSINI_H
#define CASSINI_H
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "client-request.h"
#include "server-reply.h"
#include "timing.h"
#include "timing-text-io.h"
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
#endif // CASSINI
