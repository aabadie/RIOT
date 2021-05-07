/*
 * Copyright (C) 2021 Inria
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       lz4 package test application
 *
 * This application is adapted to RIOT from
 * https://github.com/lz4/lz4/blob/dev/examples/simple_buffer.c
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "lz4.h"

#define BUFFER_SIZE 128

char compressed_data[BUFFER_SIZE];
char regen_buffer[BUFFER_SIZE];

static const char* const src =  "Lorem ipsum dolor sit amet, consectetur "
                                "adipiscing elit. Lorem ipsum dolor site amat.";

int main(void)
{
    /* Compression */
    const int src_size = (int)(strlen(src) + 1);

    const int compressed_data_size = LZ4_compress_default(src, compressed_data, src_size, BUFFER_SIZE);
    compressed_data[compressed_data_size] = 0;
    // Check return_value to determine what happened.
    if (compressed_data_size <= 0) {
        puts("Compression failed.");
        return -1;
    }

    if (compressed_data_size > 0) {
        float compression_ratio = (compressed_data_size * 100) / src_size;
        printf("Data compressed with success (ratio: %i.%i)\n",
        (int)compression_ratio / 100, (int)compression_ratio % 100);
    }

    /* Decompression */
    const int decompressed_size = LZ4_decompress_safe(compressed_data, regen_buffer, compressed_data_size, src_size);
    if (decompressed_size < 0) {
        puts("Decompression failed.");
    }
    if (decompressed_size >= 0) {
        puts("Data decompressed with success!");
    }

    if (decompressed_size != src_size) {
        puts("Decompressed data is different from original");
        return -1;
    }

    /* Validation */
    if (memcmp(src, regen_buffer, src_size) != 0) {
        puts("Validation failed");
        return -1;
    }

    printf("Validation done, decompressed string:\n%s\n", regen_buffer);
    return 0;
}
