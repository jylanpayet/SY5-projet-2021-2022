#ifndef SATURND_H
#define SATURND_H
#include "daemon.h"
#include <poll.h>
#include "client-request.h"
#include "server-reply.h"

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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
#endif
