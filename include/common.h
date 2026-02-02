#ifndef CS450_COMMON_H
#define CS450_COMMON_H
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define DIE(msg) do { fprintf(stderr,"ERROR: %s: %s\n",(msg),strerror(errno)); exit(EXIT_FAILURE);} while(0)
#define DIE_MSG(msg) do { fprintf(stderr,"ERROR: %s\n",(msg)); exit(EXIT_FAILURE);} while(0)
#endif
