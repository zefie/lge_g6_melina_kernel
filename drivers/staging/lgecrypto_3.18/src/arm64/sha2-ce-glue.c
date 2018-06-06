/*
 * sha2-ce-glue.c - SHA-224/SHA-256 using ARMv8 Crypto Extensions
 *
 * Copyright (C) 2014 Linaro Ltd <ard.biesheuvel@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/neon.h>
#include <asm/unaligned.h>
#include <linux/types.h>
#include <asm/cpu.h>
#include <linux/cpufeature.h>
#include <linux/module.h>

#include "../sha.h"
#include "../../lgecrypto_wrapper.h"
#include "../lgecrypto_fips.h"

MODULE_DESCRIPTION("SHA-224/SHA-256 secure hash using ARMv8 Crypto Extensions");
MODULE_AUTHOR("Ard Biesheuvel <ard.biesheuvel@linaro.org>");
MODULE_LICENSE("GPL v2");

asmlinkage int sha2_ce_transform(int blocks, u8 const *src, u32 *state,
                 u8 *head, long bytes);

static int _sha2_init(struct crypto_tfm *tfm)
{
    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    return 0;
}

static int sha224_init(struct shash_desc *desc)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    *sctx = (struct sha256_state){
        .state = {
            SHA224_H0, SHA224_H1, SHA224_H2, SHA224_H3,
            SHA224_H4, SHA224_H5, SHA224_H6, SHA224_H7,
        }
    };
    return 0;
}

static int sha256_init(struct shash_desc *desc)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);

    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    *sctx = (struct sha256_state){
        .state = {
            SHA256_H0, SHA256_H1, SHA256_H2, SHA256_H3,
            SHA256_H4, SHA256_H5, SHA256_H6, SHA256_H7,
        }
    };
    return 0;
}

static int sha2_update(struct shash_desc *desc, const u8 *data,
               unsigned int len)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    unsigned int partial = sctx->count % SHA256_BLOCK_SIZE;

    sctx->count += len;

    if ((partial + len) >= SHA256_BLOCK_SIZE) {
        int blocks;

        if (partial) {
            int p = SHA256_BLOCK_SIZE - partial;

            memcpy(sctx->buf + partial, data, p);
            data += p;
            len -= p;
        }

        blocks = len / SHA256_BLOCK_SIZE;
        len %= SHA256_BLOCK_SIZE;

        kernel_neon_begin_partial(28);
        sha2_ce_transform(blocks, data, sctx->state,
                  partial ? sctx->buf : NULL, 0);
        kernel_neon_end();

        data += blocks * SHA256_BLOCK_SIZE;
        partial = 0;
    }
    if (len)
        memcpy(sctx->buf + partial, data, len);
    return 0;
}

static void sha2_final(struct shash_desc *desc)
{
    static const u8 padding[SHA256_BLOCK_SIZE] = { 0x80, };

    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    __be64 bits = cpu_to_be64(sctx->count << 3);
    u32 padlen = SHA256_BLOCK_SIZE
             - ((sctx->count + sizeof(bits)) % SHA256_BLOCK_SIZE);

    sha2_update(desc, padding, padlen);
    sha2_update(desc, (const u8 *)&bits, sizeof(bits));
}

static int sha224_final(struct shash_desc *desc, u8 *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    __be32 *dst = (__be32 *)out;
    int i;

    sha2_final(desc);

    for (i = 0; i < SHA224_DIGEST_SIZE / sizeof(__be32); i++)
        put_unaligned_be32(sctx->state[i], dst++);

    *sctx = (struct sha256_state){};
    return 0;
}

static int sha256_final(struct shash_desc *desc, u8 *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    __be32 *dst = (__be32 *)out;
    int i;

    sha2_final(desc);

    for (i = 0; i < SHA256_DIGEST_SIZE / sizeof(__be32); i++)
        put_unaligned_be32(sctx->state[i], dst++);

    *sctx = (struct sha256_state){};
    return 0;
}

static void sha2_finup(struct shash_desc *desc, const u8 *data,
               unsigned int len)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    int blocks;

    if (sctx->count || !len || (len % SHA256_BLOCK_SIZE)) {
        sha2_update(desc, data, len);
        sha2_final(desc);
        return;
    }

    /*
     * Use a fast path if the input is a multiple of 64 bytes. In
     * this case, there is no need to copy data around, and we can
     * perform the entire digest calculation in a single invocation
     * of sha2_ce_transform()
     */
    blocks = len / SHA256_BLOCK_SIZE;

    kernel_neon_begin_partial(28);
    sha2_ce_transform(blocks, data, sctx->state, NULL, len);
    kernel_neon_end();
    data += blocks * SHA256_BLOCK_SIZE;
}

static int sha224_finup(struct shash_desc *desc, const u8 *data,
            unsigned int len, u8 *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    __be32 *dst = (__be32 *)out;
    int i;

    sha2_finup(desc, data, len);

    for (i = 0; i < SHA224_DIGEST_SIZE / sizeof(__be32); i++)
        put_unaligned_be32(sctx->state[i], dst++);

    *sctx = (struct sha256_state){};
    return 0;
}

static int sha256_finup(struct shash_desc *desc, const u8 *data,
            unsigned int len, u8 *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    __be32 *dst = (__be32 *)out;
    int i;

    sha2_finup(desc, data, len);

    for (i = 0; i < SHA256_DIGEST_SIZE / sizeof(__be32); i++)
        put_unaligned_be32(sctx->state[i], dst++);

    *sctx = (struct sha256_state){};
    return 0;
}

static int sha2_export(struct shash_desc *desc, void *out)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    struct sha256_state *dst = out;

    *dst = *sctx;
    return 0;
}

static int sha2_import(struct shash_desc *desc, const void *in)
{
    struct sha256_state *sctx = lgecryptow_shash_desc_ctx(desc);
    struct sha256_state const *src = in;

    *sctx = *src;
    return 0;
}

static struct shash_alg algs[] = { {
    .init            = sha224_init,
    .update            = sha2_update,
    .final            = sha224_final,
    .finup            = sha224_finup,
    .export            = sha2_export,
    .import            = sha2_import,
    .descsize        = sizeof(struct sha256_state),
    .digestsize        = SHA224_DIGEST_SIZE,
    .statesize        = sizeof(struct sha256_state),
    .base            = {
        .cra_name        = "lge-sha224",
        .cra_driver_name    = "lge-sha224-ce",
        .cra_priority        = 200,
        .cra_flags        = CRYPTO_ALG_TYPE_SHASH,
        .cra_blocksize        = SHA256_BLOCK_SIZE,
        .cra_module        = THIS_MODULE,
        .cra_init       = _sha2_init,
    }
}, {
    .init            = sha256_init,
    .update            = sha2_update,
    .final            = sha256_final,
    .finup            = sha256_finup,
    .export            = sha2_export,
    .import            = sha2_import,
    .descsize        = sizeof(struct sha256_state),
    .digestsize        = SHA256_DIGEST_SIZE,
    .statesize        = sizeof(struct sha256_state),
    .base            = {
        .cra_name        = "lge-sha256",
        .cra_driver_name    = "lge-sha256-ce",
        .cra_priority        = 200,
        .cra_flags        = CRYPTO_ALG_TYPE_SHASH,
        .cra_blocksize        = SHA256_BLOCK_SIZE,
        .cra_module        = THIS_MODULE,
        .cra_init       = _sha2_init,
    }
} };

int __init sha2_armv8_ce_64_init(void)
{
    if(!cpu_have_feature(cpu_feature(SHA2)))
        return -ENODEV;

    return lgecryptow_crypto_register_shashes(algs, ARRAY_SIZE(algs));
}

void __exit sha2_armv8_ce_64_exit(void)
{
    lgecryptow_crypto_unregister_shashes(algs, ARRAY_SIZE(algs));
}

