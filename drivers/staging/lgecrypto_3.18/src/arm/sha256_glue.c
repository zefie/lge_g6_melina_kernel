/*
 * Glue code for the SHA256 Secure Hash Algorithm assembly implementation
 * using optimized ARM assembler and NEON instructions.
 *
 * Copyright © 2015 Google Inc.
 *
 * This file is based on sha256_ssse3_glue.c:
 *   Copyright (C) 2013 Intel Corporation
 *   Author: Tim Chen <tim.c.chen@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <crypto/internal/hash.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/cryptohash.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/byteorder.h>
#include <asm/simd.h>
#include <asm/neon.h>

#include "sha256_glue.h"
#include "../sha.h"
#include "../../lgecrypto_wrapper.h"
#include "../lgecrypto_fips.h"

asmlinkage void sha256_block_data_order(u32 *digest, const void *data,
                    unsigned int num_blks);

int sha256_init(struct shash_desc *desc)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    sctx->state[0] = SHA256_H0;
    sctx->state[1] = SHA256_H1;
    sctx->state[2] = SHA256_H2;
    sctx->state[3] = SHA256_H3;
    sctx->state[4] = SHA256_H4;
    sctx->state[5] = SHA256_H5;
    sctx->state[6] = SHA256_H6;
    sctx->state[7] = SHA256_H7;
    sctx->count = 0;

    return 0;
}

int sha224_init(struct shash_desc *desc)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    sctx->state[0] = SHA224_H0;
    sctx->state[1] = SHA224_H1;
    sctx->state[2] = SHA224_H2;
    sctx->state[3] = SHA224_H3;
    sctx->state[4] = SHA224_H4;
    sctx->state[5] = SHA224_H5;
    sctx->state[6] = SHA224_H6;
    sctx->state[7] = SHA224_H7;
    sctx->count = 0;

    return 0;
}

int __sha256_update(struct shash_desc *desc, const u8 *data, unsigned int len,
            unsigned int partial)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int done = 0;

    sctx->count += len;

    if (partial) {
        done = SHA256_BLOCK_SIZE - partial;
        memcpy(sctx->buf + partial, data, done);
        sha256_block_data_order(sctx->state, sctx->buf, 1);
    }

    if (len - done >= SHA256_BLOCK_SIZE) {
        const unsigned int rounds = (len - done) / SHA256_BLOCK_SIZE;

        sha256_block_data_order(sctx->state, data + done, rounds);
        done += rounds * SHA256_BLOCK_SIZE;
    }

    memcpy(sctx->buf, data + done, len - done);

    return 0;
}

int sha256_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int partial = sctx->count % SHA256_BLOCK_SIZE;

    // Handle the fast case right here
    if (partial + len < SHA256_BLOCK_SIZE) {
        sctx->count += len;
        memcpy(sctx->buf + partial, data, len);

        return 0;
    }

    return __sha256_update(desc, data, len, partial);
}

int sha256_export(struct shash_desc *desc, void *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    memcpy(out, sctx, sizeof(*sctx));

    return 0;
}

int sha256_import(struct shash_desc *desc, const void *in)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    memcpy(sctx, in, sizeof(*sctx));

    return 0;
}

int __init sha256_armv8_neon_32_init(void)
{
    int res;

    if (!cpu_has_neon())
        return -ENODEV;
    
    res = lgecryptow_crypto_register_shashes(sha256_neon_algs, ARRAY_SIZE(sha256_neon_algs));

    return res;
}

void __exit sha256_armv8_neon_32_exit(void)
{
    lgecryptow_crypto_unregister_shashes(sha256_neon_algs, ARRAY_SIZE(sha256_neon_algs));
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SHA256 Secure Hash Algorithm (ARM), including NEON");

MODULE_ALIAS_CRYPTO("sha256");
