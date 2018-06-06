/*
 * linux/arch/arm/crypto/aesbs-glue.c - glue code for NEON bit sliced AES
 *
 * Copyright (C) 2013 Linaro Ltd <ard.biesheuvel@linaro.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <asm/neon.h>
#include <crypto/ablk_helper.h>
#include <linux/module.h>

#include "../aes.h"
#include "aes_glue.h"
#include "../../lgecrypto_wrapper.h"
#include "../lgecrypto_fips.h"

#define BIT_SLICED_KEY_MAXSIZE    (128 * (AES_MAXNR - 1) + 2 * AES_BLOCK_SIZE)

struct BS_KEY {
    struct AES_KEY    rk;
    int        converted;
    u8 __aligned(8)    bs[BIT_SLICED_KEY_MAXSIZE];
} __aligned(8);

asmlinkage void bsaes_enc_key_convert(u8 out[], struct AES_KEY const *in);
asmlinkage void bsaes_dec_key_convert(u8 out[], struct AES_KEY const *in);

asmlinkage void bsaes_cbc_encrypt(u8 const in[], u8 out[], u32 bytes,
                  struct BS_KEY *key, u8 iv[]);

asmlinkage void bsaes_ctr32_encrypt_blocks(u8 const in[], u8 out[], u32 blocks,
                       struct BS_KEY *key, u8 const iv[]);

asmlinkage void bsaes_xts_encrypt(u8 const in[], u8 out[], u32 bytes,
                  struct BS_KEY *key, u8 tweak[]);

asmlinkage void bsaes_xts_decrypt(u8 const in[], u8 out[], u32 bytes,
                  struct BS_KEY *key, u8 tweak[]);

struct aesbs_cbc_ctx {
    struct AES_KEY    enc;
    struct BS_KEY    dec;
};

struct aesbs_ctr_ctx {
    struct BS_KEY    enc;
};

struct aesbs_xts_ctx {
    struct BS_KEY    enc;
    struct BS_KEY    dec;
    struct AES_KEY    twkey;
};

static int _aesbs_init(struct crypto_tfm *tfm)
{
    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    return 0;
}

static int _aesbs_async_init(struct crypto_tfm *tfm)
{
    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    return ablk_init(tfm);
}

static int aesbs_cbc_set_key(struct crypto_tfm *tfm, const u8 *in_key,
                 unsigned int key_len)
{
    struct aesbs_cbc_ctx *ctx = lgecryptow_crypto_tfm_ctx(tfm);
    int bits = key_len * 8;

    if (private_AES_set_encrypt_key(in_key, bits, &ctx->enc)) {
        tfm->crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }
    ctx->dec.rk = ctx->enc;
    private_AES_set_decrypt_key(in_key, bits, &ctx->dec.rk);
    ctx->dec.converted = 0;
    return 0;
}

static int aesbs_ctr_set_key(struct crypto_tfm *tfm, const u8 *in_key,
                 unsigned int key_len)
{
    struct aesbs_ctr_ctx *ctx = lgecryptow_crypto_tfm_ctx(tfm);
    int bits = key_len * 8;

    if (private_AES_set_encrypt_key(in_key, bits, &ctx->enc.rk)) {
        tfm->crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }
    ctx->enc.converted = 0;
    return 0;
}

static int aesbs_xts_set_key(struct crypto_tfm *tfm, const u8 *in_key,
                 unsigned int key_len)
{
    struct aesbs_xts_ctx *ctx = lgecryptow_crypto_tfm_ctx(tfm);
    int bits = key_len * 4;

    if (private_AES_set_encrypt_key(in_key, bits, &ctx->enc.rk)) {
        tfm->crt_flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }
    ctx->dec.rk = ctx->enc.rk;
    private_AES_set_decrypt_key(in_key, bits, &ctx->dec.rk);
    private_AES_set_encrypt_key(in_key + key_len / 2, bits, &ctx->twkey);
    ctx->enc.converted = ctx->dec.converted = 0;
    return 0;
}

static int aesbs_cbc_encrypt(struct blkcipher_desc *desc,
                 struct scatterlist *dst,
                 struct scatterlist *src, unsigned int nbytes)
{
    struct aesbs_cbc_ctx *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;

    lgecryptow_blkcipher_walk_init(&walk, dst, src, nbytes);
    err = lgecryptow_blkcipher_walk_virt(desc, &walk);

    while (walk.nbytes) {
        u32 blocks = walk.nbytes / AES_BLOCK_SIZE;
        u8 *src = walk.src.virt.addr;

        if (walk.dst.virt.addr == walk.src.virt.addr) {
            u8 *iv = walk.iv;

            do {
                lgecryptow_crypto_xor(src, iv, AES_BLOCK_SIZE);
                AES_encrypt(src, src, &ctx->enc);
                iv = src;
                src += AES_BLOCK_SIZE;
            } while (--blocks);
            memcpy(walk.iv, iv, AES_BLOCK_SIZE);
        } else {
            u8 *dst = walk.dst.virt.addr;

            do {
                lgecryptow_crypto_xor(walk.iv, src, AES_BLOCK_SIZE);
                AES_encrypt(walk.iv, dst, &ctx->enc);
                memcpy(walk.iv, dst, AES_BLOCK_SIZE);
                src += AES_BLOCK_SIZE;
                dst += AES_BLOCK_SIZE;
            } while (--blocks);
        }
        err = lgecryptow_blkcipher_walk_done(desc, &walk, walk.nbytes % AES_BLOCK_SIZE);
    }
    return err;
}

static int aesbs_cbc_decrypt(struct blkcipher_desc *desc,
                 struct scatterlist *dst,
                 struct scatterlist *src, unsigned int nbytes)
{
    struct aesbs_cbc_ctx *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;

    lgecryptow_blkcipher_walk_init(&walk, dst, src, nbytes);
    err = lgecryptow_blkcipher_walk_virt_block(desc, &walk, 8 * AES_BLOCK_SIZE);

    while ((walk.nbytes / AES_BLOCK_SIZE) >= 8) {
        kernel_neon_begin();
        bsaes_cbc_encrypt(walk.src.virt.addr, walk.dst.virt.addr,
                  walk.nbytes, &ctx->dec, walk.iv);
        kernel_neon_end();
        err = lgecryptow_blkcipher_walk_done(desc, &walk, walk.nbytes % AES_BLOCK_SIZE);
    }
    while (walk.nbytes) {
        u32 blocks = walk.nbytes / AES_BLOCK_SIZE;
        u8 *dst = walk.dst.virt.addr;
        u8 *src = walk.src.virt.addr;
        u8 bk[2][AES_BLOCK_SIZE];
        u8 *iv = walk.iv;

        do {
            if (walk.dst.virt.addr == walk.src.virt.addr)
                memcpy(bk[blocks & 1], src, AES_BLOCK_SIZE);

            AES_decrypt(src, dst, &ctx->dec.rk);
            lgecryptow_crypto_xor(dst, iv, AES_BLOCK_SIZE);

            if (walk.dst.virt.addr == walk.src.virt.addr)
                iv = bk[blocks & 1];
            else
                iv = src;

            dst += AES_BLOCK_SIZE;
            src += AES_BLOCK_SIZE;
        } while (--blocks);
        err = lgecryptow_blkcipher_walk_done(desc, &walk, walk.nbytes % AES_BLOCK_SIZE);
    }
    return err;
}

static void inc_be128_ctr(__be32 ctr[], u32 addend)
{
    int i;

    for (i = 3; i >= 0; i--, addend = 1) {
        u32 n = be32_to_cpu(ctr[i]) + addend;

        ctr[i] = cpu_to_be32(n);
        if (n >= addend)
            break;
    }
}

static int aesbs_ctr_encrypt(struct blkcipher_desc *desc,
                 struct scatterlist *dst, struct scatterlist *src,
                 unsigned int nbytes)
{
    struct aesbs_ctr_ctx *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    u32 blocks;
    int err;

    lgecryptow_blkcipher_walk_init(&walk, dst, src, nbytes);
    err = lgecryptow_blkcipher_walk_virt_block(desc, &walk, 8 * AES_BLOCK_SIZE);

    while ((blocks = walk.nbytes / AES_BLOCK_SIZE)) {
        u32 tail = walk.nbytes % AES_BLOCK_SIZE;
        __be32 *ctr = (__be32 *)walk.iv;
        u32 headroom = UINT_MAX - be32_to_cpu(ctr[3]);

        /* avoid 32 bit counter overflow in the NEON code */
        if (unlikely(headroom < blocks)) {
            blocks = headroom + 1;
            tail = walk.nbytes - blocks * AES_BLOCK_SIZE;
        }
        kernel_neon_begin();
        bsaes_ctr32_encrypt_blocks(walk.src.virt.addr,
                       walk.dst.virt.addr, blocks,
                       &ctx->enc, walk.iv);
        kernel_neon_end();
        inc_be128_ctr(ctr, blocks);

        nbytes -= blocks * AES_BLOCK_SIZE;
        if (nbytes && nbytes == tail && nbytes <= AES_BLOCK_SIZE)
            break;

        err = lgecryptow_blkcipher_walk_done(desc, &walk, tail);
    }
    if (walk.nbytes) {
        u8 *tdst = walk.dst.virt.addr + blocks * AES_BLOCK_SIZE;
        u8 *tsrc = walk.src.virt.addr + blocks * AES_BLOCK_SIZE;
        u8 ks[AES_BLOCK_SIZE];

        AES_encrypt(walk.iv, ks, &ctx->enc.rk);
        if (tdst != tsrc)
            memcpy(tdst, tsrc, nbytes);
        lgecryptow_crypto_xor(tdst, ks, nbytes);
        err = lgecryptow_blkcipher_walk_done(desc, &walk, 0);
    }
    return err;
}

static int aesbs_xts_encrypt(struct blkcipher_desc *desc,
                 struct scatterlist *dst,
                 struct scatterlist *src, unsigned int nbytes)
{
    struct aesbs_xts_ctx *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;

    lgecryptow_blkcipher_walk_init(&walk, dst, src, nbytes);
    err = lgecryptow_blkcipher_walk_virt_block(desc, &walk, 8 * AES_BLOCK_SIZE);

    /* generate the initial tweak */
    AES_encrypt(walk.iv, walk.iv, &ctx->twkey);

    while (walk.nbytes) {
        kernel_neon_begin();
        bsaes_xts_encrypt(walk.src.virt.addr, walk.dst.virt.addr,
                  walk.nbytes, &ctx->enc, walk.iv);
        kernel_neon_end();
        err = lgecryptow_blkcipher_walk_done(desc, &walk, walk.nbytes % AES_BLOCK_SIZE);
    }
    return err;
}

static int aesbs_xts_decrypt(struct blkcipher_desc *desc,
                 struct scatterlist *dst,
                 struct scatterlist *src, unsigned int nbytes)
{
    struct aesbs_xts_ctx *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk walk;
    int err;

    lgecryptow_blkcipher_walk_init(&walk, dst, src, nbytes);
    err = lgecryptow_blkcipher_walk_virt_block(desc, &walk, 8 * AES_BLOCK_SIZE);

    /* generate the initial tweak */
    AES_encrypt(walk.iv, walk.iv, &ctx->twkey);

    while (walk.nbytes) {
        kernel_neon_begin();
        bsaes_xts_decrypt(walk.src.virt.addr, walk.dst.virt.addr,
                  walk.nbytes, &ctx->dec, walk.iv);
        kernel_neon_end();
        err = lgecryptow_blkcipher_walk_done(desc, &walk, walk.nbytes % AES_BLOCK_SIZE);
    }
    return err;
}

static struct crypto_alg aesbs_algs[] = { {
    .cra_name        = "__lge-cbc-aes-neonbs",
    .cra_driver_name    = "__driver-lge-cbc-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize        = AES_BLOCK_SIZE,
    .cra_ctxsize        = sizeof(struct aesbs_cbc_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_blkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init       = _aesbs_init,
    .cra_blkcipher = {
        .min_keysize    = AES_MIN_KEY_SIZE,
        .max_keysize    = AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = aesbs_cbc_set_key,
        .encrypt    = aesbs_cbc_encrypt,
        .decrypt    = aesbs_cbc_decrypt,
    },
}, {
    .cra_name        = "__lge-ctr-aes-neonbs",
    .cra_driver_name    = "__driver-lge-ctr-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize        = 1,
    .cra_ctxsize        = sizeof(struct aesbs_ctr_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_blkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init       = _aesbs_init,
    .cra_blkcipher = {
        .min_keysize    = AES_MIN_KEY_SIZE,
        .max_keysize    = AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = aesbs_ctr_set_key,
        .encrypt    = aesbs_ctr_encrypt,
        .decrypt    = aesbs_ctr_encrypt,
    },
}, {
    .cra_name        = "__lge-xts-aes-neonbs",
    .cra_driver_name    = "__driver-lge-xts-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_BLKCIPHER,
    .cra_blocksize        = AES_BLOCK_SIZE,
    .cra_ctxsize        = sizeof(struct aesbs_xts_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_blkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init       = _aesbs_init,
    .cra_blkcipher = {
        .min_keysize    = 2 * AES_MIN_KEY_SIZE,
        .max_keysize    = 2 * AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = aesbs_xts_set_key,
        .encrypt    = aesbs_xts_encrypt,
        .decrypt    = aesbs_xts_decrypt,
    },
}, {
    .cra_name        = "lge-cbc(aes)",
    .cra_driver_name    = "lge-cbc-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_ABLKCIPHER|CRYPTO_ALG_ASYNC,
    .cra_blocksize        = AES_BLOCK_SIZE,
    .cra_ctxsize        = sizeof(struct async_helper_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_ablkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init        = _aesbs_async_init,
    .cra_exit        = ablk_exit,
    .cra_ablkcipher = {
        .min_keysize    = AES_MIN_KEY_SIZE,
        .max_keysize    = AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = ablk_set_key,
        .encrypt    = __ablk_encrypt,
        .decrypt    = ablk_decrypt,
    }
}, {
    .cra_name        = "lge-ctr(aes)",
    .cra_driver_name    = "lge-ctr-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_ABLKCIPHER|CRYPTO_ALG_ASYNC,
    .cra_blocksize        = 1,
    .cra_ctxsize        = sizeof(struct async_helper_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_ablkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init        = _aesbs_async_init,
    .cra_exit        = ablk_exit,
    .cra_ablkcipher = {
        .min_keysize    = AES_MIN_KEY_SIZE,
        .max_keysize    = AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = ablk_set_key,
        .encrypt    = ablk_encrypt,
        .decrypt    = ablk_decrypt,
    }
}, {
    .cra_name        = "lge-xts(aes)",
    .cra_driver_name    = "lge-xts-aes-neonbs",
    .cra_priority        = 250,
    .cra_flags        = CRYPTO_ALG_TYPE_ABLKCIPHER|CRYPTO_ALG_ASYNC,
    .cra_blocksize        = AES_BLOCK_SIZE,
    .cra_ctxsize        = sizeof(struct async_helper_ctx),
    .cra_alignmask        = 7,
    .cra_type        = &crypto_ablkcipher_type,
    .cra_module        = THIS_MODULE,
    .cra_init        = _aesbs_async_init,
    .cra_exit        = ablk_exit,
    .cra_ablkcipher = {
        .min_keysize    = 2 * AES_MIN_KEY_SIZE,
        .max_keysize    = 2 * AES_MAX_KEY_SIZE,
        .ivsize        = AES_BLOCK_SIZE,
        .setkey        = ablk_set_key,
        .encrypt    = ablk_encrypt,
        .decrypt    = ablk_decrypt,
    }
} };

int __init aes_armv8_neon_32_init(void)
{
    if (!cpu_has_neon())
        return -ENODEV;

    return lgecryptow_crypto_register_algs(aesbs_algs, ARRAY_SIZE(aesbs_algs));
}

void __exit aes_armv8_neon_32_exit(void)
{
    lgecryptow_crypto_unregister_algs(aesbs_algs, ARRAY_SIZE(aesbs_algs));
}

MODULE_DESCRIPTION("Bit sliced AES in CBC/CTR/XTS modes using NEON");
MODULE_AUTHOR("Ard Biesheuvel <ard.biesheuvel@linaro.org>");
MODULE_LICENSE("GPL");
