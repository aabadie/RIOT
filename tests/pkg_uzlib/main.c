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
 * @brief       Test application for the uzlib package
 *
 * This application is adapted to RIOT from the uzlib examples:
 * tgzip: https://github.com/pfalcon/uzlib/blob/master/examples/tgzip/tgzip.c
 * tgunzip: https://github.com/pfalcon/uzlib/blob/master/examples/tgunzip/tgunzip.c
 *
 * @author      Alexandre Abadie <alexandre.abadie@inria.fr>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "uzlib.h"

/* The hash table size is computed from the data type (8 bit) multiplied by the
 * number of possible values (1 << hash_bits, with hash_bits set to 6 in
 * this application) */
#define HASH_TABLE_SIZE     (8 * 64)
#define CHUNK_SIZE          (1)
static uzlib_hash_entry_t hash_table[HASH_TABLE_SIZE];

static const char *message =    "Lorem ipsum dolor sit amet, consectetur "
                                "adipiscing elit, sed do eiusmod tempor "
                                "incididunt ut labore et dolore magna aliqua. "
                                "Nisl vel pretium lectus quam id leo. Volutpat "
                                "sed cras ornare arcu dui vivamus arcu felis "
                                "bibendum. Velit scelerisque in dictum non "
                                "consectetur a erat nam. Pretium viverra "
                                "suspendisse potenti nullam ac tortor vitae "
                                "purus faucibus. Tristique et egestas quis "
                                "ipsum suspendisse. At imperdiet dui accumsan "
                                "sit amet nulla facilisi. Pulvinar neque "
                                "laoreet suspendisse interdum consectetur "
                                "libero. Vulputate sapien nec sagittis aliquam "
                                "malesuada bibendum arcu vitae. Sed adipiscing "
                                "diam donec adipiscing tristique risus nec. "
                                "Venenatis tellus in metus vulputate eu "
                                "scelerisque. Id faucibus nisl tincidunt eget "
                                "nullam non nisi est. Integer feugiat "
                                "scelerisque varius morbi enim. Est sit amet "
                                "facilisis magna etiam. Venenatis cras sed "
                                "felis eget velit aliquet sagittis.";
static unsigned char decompressed_output[1024];

int main(void)
{
    /* Compress the message */
    struct uzlib_comp comp = {0};
    comp.dict_size = 32768;
    comp.hash_bits = 6;
    comp.hash_table = hash_table;

    zlib_start_block(&comp.out);
    uzlib_compress(&comp, (uint8_t *)message, strlen(message));
    zlib_finish_block(&comp.out);

    float compression_ratio = (comp.out.outlen * 100.0) / strlen(message);

    printf("Compressing message (len: %u): \"%s\"\n",
           strlen(message), message);
    printf("Message compressed to %u raw bytes (ratio: %i.%i)\n",
           comp.out.outlen,
           (int)compression_ratio / 100, (int)compression_ratio % 100) ;

    /* Decompress */
    uzlib_init();
    unsigned int dlen;

    /* Get decompressed length */
    dlen =            comp.out.outbuf[comp.out.outlen - 1];
    dlen = 256 * dlen + comp.out.outbuf[comp.out.outlen - 2];
    dlen = 256 * dlen + comp.out.outbuf[comp.out.outlen - 3];
    dlen = 256 * dlen + comp.out.outbuf[comp.out.outlen - 4];

    struct uzlib_uncomp d;
    uzlib_uncompress_init(&d, NULL, 0);

    /* all 3 fields below must be initialized by user */
    d.source = comp.out.outbuf;
    d.source_limit = comp.out.outbuf + comp.out.outlen - 1;
    d.source_read_cb = NULL;
    d.dest_start = d.dest = decompressed_output;
    int res = -1;
    while (dlen) {
        unsigned int chunk_len = dlen < CHUNK_SIZE ? dlen : CHUNK_SIZE;
        d.dest_limit = d.dest + chunk_len;
        res = uzlib_uncompress_chksum(&d);
        dlen -= chunk_len;
        if (res != TINF_OK) {
            break;
        }
    }

    uint16_t dsize = d.dest - decompressed_output;
    printf("Decompressed message (len: %i): \"%s\"\n",
           dsize, decompressed_output);

    return 0;
}
