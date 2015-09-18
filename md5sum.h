#ifndef __MD5SUM_H__
#define __MD5SUM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <openssl/md5.h>

#define BUFFER_SIZE 1024

void md5sum(const char * const filename, unsigned char *sum);

#endif /* __MD5SUM_H__ */
