#include "md5sum.h"

/*
 * @brief Computes the md5sum of a file
 *        Returns the md5sum
 *
 * @param filename File on which md5sum has to be computed
 */
void md5sum(const char * const filename, unsigned char *sum)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char data[BUFFER_SIZE+1];
    MD5_CTX ctx;
    FILE *fp = NULL;
    size_t bytes;

    fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "fopen error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    MD5_Init(&ctx);
    while ((bytes = fread(data, 1, BUFFER_SIZE, fp)) != 0)
        MD5_Update(&ctx, data, bytes);
    MD5_Final(c, &ctx);

    fclose(fp);

    memcpy(sum, c, MD5_DIGEST_LENGTH * sizeof(unsigned char));
    return;
}
