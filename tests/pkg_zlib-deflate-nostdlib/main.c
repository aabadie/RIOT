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
 * @brief       Test for the zlib-deflate-nostdlib package
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "inflate.h"

const char * expected_string = "Hello, World? Hello, World!";

static unsigned char zlib_input[] = {
  0x78, 0x9c, 0xf3, 0x48, 0xcd, 0xc9, 0xc9, 0xd7, 0x51, 0x08, 0xcf, 0x2f,
  0xca, 0x49, 0xb1, 0x57, 0xf0, 0x40, 0xe2, 0x29, 0x02, 0x00, 0x80, 0x7d,
  0x09, 0x11
};

unsigned char zlib_output[32];

int main(void)
{
    int16_t out_bytes = inflate_zlib(zlib_input, sizeof(zlib_input),
                                     zlib_output, sizeof(zlib_output));
    if (out_bytes < 0) {
        puts("Zlib error");
        return -1;
    }

    if (memcmp(zlib_output, expected_string, strlen(expected_string)) != 0) {
        puts("Decompressed string doesn't match");
        return -1;
    }

    puts("SUCCESS");
}
