/* XTS: as defined in IEEE1619/D16
 *    http://grouper.ieee.org/groups/1619/email/pdf00086.pdf
 *    (sector sizes which are not a multiple of 16 bytes are,
 *    however currently unsupported)
 *
 * Copyright (c) 2007 Rik Snel <rsnel@cube.dyndns.org>
 *
 * Based om ecb.c
 * Copyright (c) 2006 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>

#include "xts.h"
#include "../lgecrypto_wrapper.h"

struct priv {
    struct crypto_cipher *child;
    struct crypto_cipher *tweak;
};

static int setkey(struct crypto_tfm *parent, const u8 *key,
          unsigned int keylen)
{
    struct priv *ctx = lgecryptow_crypto_tfm_ctx(parent);
    struct crypto_cipher *child = ctx->tweak;
    u32 *flags = &parent->crt_flags;
    int err;

    /* key consists of keys of equal size concatenated, therefore
     * the length must be even */
    if (keylen % 2) {
        /* tell the user why there was an error */
        *flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    /* we need two cipher instances: one to compute the initial 'tweak'
     * by encrypting the IV (usually the 'plain' iv) and the other
     * one to encrypt and decrypt the data */

    /* tweak cipher, uses Key2 i.e. the second half of *key */
    lgecryptow_crypto_cipher_clear_flags(child, CRYPTO_TFM_REQ_MASK);
    lgecryptow_crypto_cipher_set_flags(child, lgecryptow_crypto_tfm_get_flags(parent) &
                       CRYPTO_TFM_REQ_MASK);
    err = lgecryptow_crypto_cipher_setkey(child, key + keylen/2, keylen/2);
    if (err)
        return err;

    lgecryptow_crypto_tfm_set_flags(parent, lgecryptow_crypto_cipher_get_flags(child) &
                     CRYPTO_TFM_RES_MASK);

    child = ctx->child;

    /* data cipher, uses Key1 i.e. the first half of *key */
    lgecryptow_crypto_cipher_clear_flags(child, CRYPTO_TFM_REQ_MASK);
    lgecryptow_crypto_cipher_set_flags(child, lgecryptow_crypto_tfm_get_flags(parent) &
                       CRYPTO_TFM_REQ_MASK);
    err = lgecryptow_crypto_cipher_setkey(child, key, keylen/2);
    if (err)
        return err;

    lgecryptow_crypto_tfm_set_flags(parent, lgecryptow_crypto_cipher_get_flags(child) &
                     CRYPTO_TFM_RES_MASK);

    return 0;
}

struct sinfo {
    be128 *t;
    struct crypto_tfm *tfm;
    void (*fn)(struct crypto_tfm *, u8 *, const u8 *);
};

static inline void xts_round(struct sinfo *s, void *dst, const void *src)
{
    lgecryptow_be128_xor(dst, s->t, src);        /* PP <- T xor P */
    s->fn(s->tfm, dst, dst);        /* CC <- E(Key1,PP) */
    lgecryptow_be128_xor(dst, dst, s->t);        /* C <- T xor CC */
}

static int crypt(struct blkcipher_desc *d,
         struct blkcipher_walk *w, struct priv *ctx,
         void (*tw)(struct crypto_tfm *, u8 *, const u8 *),
         void (*fn)(struct crypto_tfm *, u8 *, const u8 *))
{
    int err;
    unsigned int avail;
    const int bs = XTS_BLOCK_SIZE;
    struct sinfo s = {
        .tfm = lgecryptow_crypto_cipher_tfm(ctx->child),
        .fn = fn
    };
    u8 *wsrc;
    u8 *wdst;

    err = lgecryptow_blkcipher_walk_virt(d, w);
    if (!w->nbytes)
        return err;

    s.t = (be128 *)w->iv;
    avail = w->nbytes;

    wsrc = w->src.virt.addr;
    wdst = w->dst.virt.addr;

    /* calculate first value of T */
    tw(lgecryptow_crypto_cipher_tfm(ctx->tweak), w->iv, w->iv);

    goto first;

    for (;;) {
        do {
            lgecryptow_gf128mul_x_ble(s.t, s.t);

first:
            xts_round(&s, wdst, wsrc);

            wsrc += bs;
            wdst += bs;
        } while ((avail -= bs) >= bs);

        err = lgecryptow_blkcipher_walk_done(d, w, avail);
        if (!w->nbytes)
            break;

        avail = w->nbytes;

        wsrc = w->src.virt.addr;
        wdst = w->dst.virt.addr;
    }

    return err;
}

static int encrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
           struct scatterlist *src, unsigned int nbytes)
{
    struct priv *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk w;

    lgecryptow_blkcipher_walk_init(&w, dst, src, nbytes);
    return crypt(desc, &w, ctx, lgecryptow_crypto_cipher_alg(ctx->tweak)->cia_encrypt,
             lgecryptow_crypto_cipher_alg(ctx->child)->cia_encrypt);
}

static int decrypt(struct blkcipher_desc *desc, struct scatterlist *dst,
           struct scatterlist *src, unsigned int nbytes)
{
    struct priv *ctx = lgecryptow_crypto_blkcipher_ctx(desc->tfm);
    struct blkcipher_walk w;

    lgecryptow_blkcipher_walk_init(&w, dst, src, nbytes);
    return crypt(desc, &w, ctx, lgecryptow_crypto_cipher_alg(ctx->tweak)->cia_encrypt,
             lgecryptow_crypto_cipher_alg(ctx->child)->cia_decrypt);
}

int lgecrypto_xts_crypt(struct blkcipher_desc *desc, struct scatterlist *sdst,
          struct scatterlist *ssrc, unsigned int nbytes,
          struct xts_crypt_req *req)
{
    const unsigned int bsize = XTS_BLOCK_SIZE;
    const unsigned int max_blks = req->tbuflen / bsize;
    struct blkcipher_walk walk;
    unsigned int nblocks;
    be128 *src, *dst, *t;
    be128 *t_buf = req->tbuf;
    int err, i;

    BUG_ON(max_blks < 1);

    lgecryptow_blkcipher_walk_init(&walk, sdst, ssrc, nbytes);

    err = lgecryptow_blkcipher_walk_virt(desc, &walk);
    nbytes = walk.nbytes;
    if (!nbytes)
        return err;

    nblocks = min(nbytes / bsize, max_blks);
    src = (be128 *)walk.src.virt.addr;
    dst = (be128 *)walk.dst.virt.addr;

    /* calculate first value of T */
    req->tweak_fn(req->tweak_ctx, (u8 *)&t_buf[0], walk.iv);

    i = 0;
    goto first;

    for (;;) {
        do {
            for (i = 0; i < nblocks; i++) {
                lgecryptow_gf128mul_x_ble(&t_buf[i], t);
first:
                t = &t_buf[i];

                /* PP <- T xor P */
                lgecryptow_be128_xor(dst + i, t, src + i);
            }

            /* CC <- E(Key2,PP) */
            req->crypt_fn(req->crypt_ctx, (u8 *)dst,
                      nblocks * bsize);

            /* C <- T xor CC */
            for (i = 0; i < nblocks; i++)
                lgecryptow_be128_xor(dst + i, dst + i, &t_buf[i]);

            src += nblocks;
            dst += nblocks;
            nbytes -= nblocks * bsize;
            nblocks = min(nbytes / bsize, max_blks);
        } while (nblocks > 0);

        *(be128 *)walk.iv = *t;

        err = lgecryptow_blkcipher_walk_done(desc, &walk, nbytes);
        nbytes = walk.nbytes;
        if (!nbytes)
            break;

        nblocks = min(nbytes / bsize, max_blks);
        src = (be128 *)walk.src.virt.addr;
        dst = (be128 *)walk.dst.virt.addr;
    }

    return err;
}
EXPORT_SYMBOL_GPL(lgecrypto_xts_crypt);

static int init_tfm(struct crypto_tfm *tfm)
{
    struct crypto_cipher *cipher;
    struct crypto_instance *inst = (void *)tfm->__crt_alg;
    struct crypto_spawn *spawn = lgecryptow_crypto_instance_ctx(inst);
    struct priv *ctx = lgecryptow_crypto_tfm_ctx(tfm);
    u32 *flags = &tfm->crt_flags;

    cipher = lgecryptow_crypto_spawn_cipher(spawn);
    if (IS_ERR(cipher))
        return PTR_ERR(cipher);

    if (lgecryptow_crypto_cipher_blocksize(cipher) != XTS_BLOCK_SIZE) {
        *flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
        lgecryptow_crypto_free_cipher(cipher);
        return -EINVAL;
    }

    ctx->child = cipher;

    cipher = lgecryptow_crypto_spawn_cipher(spawn);
    if (IS_ERR(cipher)) {
        lgecryptow_crypto_free_cipher(ctx->child);
        return PTR_ERR(cipher);
    }

    /* this check isn't really needed, leave it here just in case */
    if (lgecryptow_crypto_cipher_blocksize(cipher) != XTS_BLOCK_SIZE) {
        lgecryptow_crypto_free_cipher(cipher);
        lgecryptow_crypto_free_cipher(ctx->child);
        *flags |= CRYPTO_TFM_RES_BAD_BLOCK_LEN;
        return -EINVAL;
    }

    ctx->tweak = cipher;

    return 0;
}

static void exit_tfm(struct crypto_tfm *tfm)
{
    struct priv *ctx = lgecryptow_crypto_tfm_ctx(tfm);
    lgecryptow_crypto_free_cipher(ctx->child);
    lgecryptow_crypto_free_cipher(ctx->tweak);
}

static struct crypto_instance *alloc(struct rtattr **tb)
{
    struct crypto_instance *inst;
    struct crypto_alg *alg;
    int err;

    err = lgecryptow_crypto_check_attr_type(tb, CRYPTO_ALG_TYPE_BLKCIPHER);
    if (err)
        return ERR_PTR(err);

    alg = lgecryptow_crypto_get_attr_alg(tb, CRYPTO_ALG_TYPE_CIPHER,
                  CRYPTO_ALG_TYPE_MASK);
    if (IS_ERR(alg))
        return ERR_CAST(alg);

    inst = lgecryptow_crypto_alloc_instance("lge-xts", alg);
    if (IS_ERR(inst))
        goto out_put_alg;

    inst->alg.cra_flags = CRYPTO_ALG_TYPE_BLKCIPHER;
    inst->alg.cra_priority = alg->cra_priority;
    inst->alg.cra_blocksize = alg->cra_blocksize;

    if (alg->cra_alignmask < 7)
        inst->alg.cra_alignmask = 7;
    else
        inst->alg.cra_alignmask = alg->cra_alignmask;

    inst->alg.cra_type = &crypto_blkcipher_type;

    inst->alg.cra_blkcipher.ivsize = alg->cra_blocksize;
    inst->alg.cra_blkcipher.min_keysize =
        2 * alg->cra_cipher.cia_min_keysize;
    inst->alg.cra_blkcipher.max_keysize =
        2 * alg->cra_cipher.cia_max_keysize;

    inst->alg.cra_ctxsize = sizeof(struct priv);

    inst->alg.cra_init = init_tfm;
    inst->alg.cra_exit = exit_tfm;

    inst->alg.cra_blkcipher.setkey = setkey;
    inst->alg.cra_blkcipher.encrypt = encrypt;
    inst->alg.cra_blkcipher.decrypt = decrypt;

out_put_alg:
    lgecryptow_crypto_mod_put(alg);
    return inst;
}

static void free(struct crypto_instance *inst)
{
    lgecryptow_crypto_drop_spawn(lgecryptow_crypto_instance_ctx(inst));
    kfree(inst);
}

static struct crypto_template crypto_tmpl = {
    .name = "lge-xts",
    .alloc = alloc,
    .free = free,
    .module = THIS_MODULE,
};

int __init crypto_module_init(void)
{
    return lgecryptow_crypto_register_template(&crypto_tmpl);
}

void __exit crypto_module_exit(void)
{
    lgecryptow_crypto_unregister_template(&crypto_tmpl);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("XTS block cipher mode");
