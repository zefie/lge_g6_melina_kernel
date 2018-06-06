/*
 * Glue code for the SHA1 Secure Hash Algorithm assembler implementation using
 * ARM NEON instructions.
 *
 * Copyright © 2014 Jussi Kivilinna <jussi.kivilinna@iki.fi>
 *
 * This file is based on sha1_generic.c and sha1_ssse3_glue.c:
 *  Copyright (c) Alan Smithee.
 *  Copyright (c) Andrew McDonald <andrew@mcdonald.org.uk>
 *  Copyright (c) Jean-Francois Dive <jef@linuxbe.org>
 *  Copyright (c) Mathias Krause <minipli@googlemail.com>
 *  Copyright (c) Chandramouli Narayanan <mouli@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <asm/byteorder.h>
#include <asm/neon.h>
#include <asm/simd.h>
#include <asm/crypto/sha1.h>

#include "../sha.h"
#include "../../lgecrypto_wrapper.h"
#include "../lgecrypto_fips.h"

asmlinkage void sha1_transform_neon(void *state_h, const char *data,
                    unsigned int rounds);


static int _sha1_init(struct crypto_tfm *tfm)
{
    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    return 0;
}

static int sha1_neon_init(struct shash_desc *desc)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);

    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    *sctx = (struct sha1_state){
        .state = { SHA1_H0, SHA1_H1, SHA1_H2, SHA1_H3, SHA1_H4 },
    };

    return 0;
}

static int __sha1_neon_update(struct shash_desc *desc, const u8 *data,
                   unsigned int len, unsigned int partial)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int done = 0;

    sctx->count += len;

    if (partial) {
        done = SHA1_BLOCK_SIZE - partial;
        memcpy(sctx->buffer + partial, data, done);
        sha1_transform_neon(sctx->state, sctx->buffer, 1);
    }

    if (len - done >= SHA1_BLOCK_SIZE) {
        const unsigned int rounds = (len - done) / SHA1_BLOCK_SIZE;

        sha1_transform_neon(sctx->state, data + done, rounds);
        done += rounds * SHA1_BLOCK_SIZE;
    }

    memcpy(sctx->buffer, data + done, len - done);

    return 0;
}

static int sha1_neon_update(struct shash_desc *desc, const u8 *data,
                 unsigned int len)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int partial = sctx->count % SHA1_BLOCK_SIZE;
    int res;

    /* Handle the fast case right here */
    if (partial + len < SHA1_BLOCK_SIZE) {
        sctx->count += len;
        memcpy(sctx->buffer + partial, data, len);

        return 0;
    }

    if (!may_use_simd()) {
        res = sha1_update_arm(desc, data, len);
    } else {
        kernel_neon_begin();
        res = __sha1_neon_update(desc, data, len, partial);
        kernel_neon_end();
    }

    return res;
}


/* Add padding and return the message digest. */
static int sha1_neon_final(struct shash_desc *desc, u8 *out)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int i, index, padlen;
    __be32 *dst = (__be32 *)out;
    __be64 bits;
    static const u8 padding[SHA1_BLOCK_SIZE] = { 0x80, };

    bits = cpu_to_be64(sctx->count << 3);

    /* Pad out to 56 mod 64 and append length */
    index = sctx->count % SHA1_BLOCK_SIZE;
    padlen = (index < 56) ? (56 - index) : ((SHA1_BLOCK_SIZE+56) - index);
    if (!may_use_simd()) {
        sha1_update_arm(desc, padding, padlen);
        sha1_update_arm(desc, (const u8 *)&bits, sizeof(bits));
    } else {
        kernel_neon_begin();
        /* We need to fill a whole block for __sha1_neon_update() */
        if (padlen <= 56) {
            sctx->count += padlen;
            memcpy(sctx->buffer + index, padding, padlen);
        } else {
            __sha1_neon_update(desc, padding, padlen, index);
        }
        __sha1_neon_update(desc, (const u8 *)&bits, sizeof(bits), 56);
        kernel_neon_end();
    }

    /* Store state in digest */
    for (i = 0; i < 5; i++)
        dst[i] = cpu_to_be32(sctx->state[i]);

    /* Wipe context */
    memset(sctx, 0, sizeof(*sctx));

    return 0;
}

static int sha1_neon_export(struct shash_desc *desc, void *out)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);

    memcpy(out, sctx, sizeof(*sctx));

    return 0;
}

static int sha1_neon_import(struct shash_desc *desc, const void *in)
{
    struct sha1_state *sctx = lgecryptow_shash_desc_ctx(desc);

    memcpy(sctx, in, sizeof(*sctx));

    return 0;
}

static struct shash_alg alg = {
    .digestsize    =    SHA1_DIGEST_SIZE,
    .init        =    sha1_neon_init,
    .update        =    sha1_neon_update,
    .final        =    sha1_neon_final,
    .export        =    sha1_neon_export,
    .import        =    sha1_neon_import,
    .descsize    =    sizeof(struct sha1_state),
    .statesize    =    sizeof(struct sha1_state),
    .base        =    {
        .cra_name        = "lge-sha1",
        .cra_driver_name    = "lge-sha1-neon",
        .cra_priority        = 250,
        .cra_flags        = CRYPTO_ALG_TYPE_SHASH,
        .cra_blocksize        = SHA1_BLOCK_SIZE,
        .cra_module        = THIS_MODULE,
        .cra_init       = _sha1_init,
    }
};

int __init sha1_armv8_neon_32_init(void)
{
    if (!cpu_has_neon())
        return -ENODEV;

    return lgecryptow_crypto_register_shash(&alg);
}

void __exit sha1_armv8_neon_32_exit(void)
{
    lgecryptow_crypto_unregister_shash(&alg);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SHA1 Secure Hash Algorithm, NEON accelerated");
MODULE_ALIAS("sha1");
