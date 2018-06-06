/*
 * Cryptographic API.
 *
 * HMAC: Keyed-Hashing for Message Authentication (RFC2104).
 *
 * Copyright (c) 2002 James Morris <jmorris@intercode.com.au>
 * Copyright (c) 2006 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * The HMAC implementation is derived from USAGI.
 * Copyright (c) 2002 Kazunori Miyazawa <miyazawa@linux-ipv6.org> / USAGI
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>

#include "../lgecrypto_wrapper.h"
#include "lgecrypto_fips.h"

struct hmac_ctx {
    struct crypto_shash *hash;
};

static inline void *align_ptr(void *p, unsigned int align)
{
    return (void *)ALIGN((unsigned long)p, align);
}

static inline struct hmac_ctx *hmac_ctx(struct crypto_shash *tfm)
{
    return align_ptr(lgecryptow_crypto_shash_ctx_aligned(tfm) +
             lgecryptow_crypto_shash_statesize(tfm) * 2,
             lgecryptow_crypto_tfm_ctx_alignment());
}

static int hmac_setkey(struct crypto_shash *parent,
               const u8 *inkey, unsigned int keylen)
{
    int bs = lgecryptow_crypto_shash_blocksize(parent);
    int ds = lgecryptow_crypto_shash_digestsize(parent);
    int ss = lgecryptow_crypto_shash_statesize(parent);
    char *ipad = lgecryptow_crypto_shash_ctx_aligned(parent);
    char *opad = ipad + ss;
    struct hmac_ctx *ctx = align_ptr(opad + ss,
                     lgecryptow_crypto_tfm_ctx_alignment());
    struct crypto_shash *hash = ctx->hash;
    struct {
        struct shash_desc shash;
        char ctx[lgecryptow_crypto_shash_descsize(hash)];
    } desc;
    unsigned int i;

    desc.shash.tfm = hash;
    desc.shash.flags = lgecryptow_crypto_shash_get_flags(parent) &
                CRYPTO_TFM_REQ_MAY_SLEEP;

    if (keylen > bs) {
        int err;

        err = lgecryptow_crypto_shash_digest(&desc.shash, inkey, keylen, ipad);
        if (err)
            return err;

        keylen = ds;
    } else
        memcpy(ipad, inkey, keylen);

    memset(ipad + keylen, 0, bs - keylen);
    memcpy(opad, ipad, bs);

    for (i = 0; i < bs; i++) {
        ipad[i] ^= 0x36;
        opad[i] ^= 0x5c;
    }

    return lgecryptow_crypto_shash_init(&desc.shash) ?:
           lgecryptow_crypto_shash_update(&desc.shash, ipad, bs) ?:
           lgecryptow_crypto_shash_export(&desc.shash, ipad) ?:
           lgecryptow_crypto_shash_init(&desc.shash) ?:
           lgecryptow_crypto_shash_update(&desc.shash, opad, bs) ?:
           lgecryptow_crypto_shash_export(&desc.shash, opad);
}

static int hmac_export(struct shash_desc *pdesc, void *out)
{
    struct shash_desc *desc = lgecryptow_shash_desc_ctx(pdesc);

    desc->flags = pdesc->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

    return lgecryptow_crypto_shash_export(desc, out);
}

static int hmac_import(struct shash_desc *pdesc, const void *in)
{
    struct shash_desc *desc = lgecryptow_shash_desc_ctx(pdesc);
    struct hmac_ctx *ctx = hmac_ctx(pdesc->tfm);

    desc->tfm = ctx->hash;
    desc->flags = pdesc->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

    return lgecryptow_crypto_shash_import(desc, in);
}

static int hmac_init(struct shash_desc *pdesc)
{
    /* IF FIPS tests not passed, return error */
    if (g_fips_state == FIPS_STATUS_FAIL)
        return -ENXIO;

    return hmac_import(pdesc, lgecryptow_crypto_shash_ctx_aligned(pdesc->tfm));
}

static int hmac_update(struct shash_desc *pdesc,
               const u8 *data, unsigned int nbytes)
{
    struct shash_desc *desc = lgecryptow_shash_desc_ctx(pdesc);

    desc->flags = pdesc->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

    return lgecryptow_crypto_shash_update(desc, data, nbytes);
}

static int hmac_final(struct shash_desc *pdesc, u8 *out)
{
    struct crypto_shash *parent = pdesc->tfm;
    int ds = lgecryptow_crypto_shash_digestsize(parent);
    int ss = lgecryptow_crypto_shash_statesize(parent);
    char *opad = lgecryptow_crypto_shash_ctx_aligned(parent) + ss;
    struct shash_desc *desc = lgecryptow_shash_desc_ctx(pdesc);

    desc->flags = pdesc->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

    return lgecryptow_crypto_shash_final(desc, out) ?:
           lgecryptow_crypto_shash_import(desc, opad) ?:
           lgecryptow_crypto_shash_finup(desc, out, ds, out);
}

static int hmac_finup(struct shash_desc *pdesc, const u8 *data,
              unsigned int nbytes, u8 *out)
{

    struct crypto_shash *parent = pdesc->tfm;
    int ds = lgecryptow_crypto_shash_digestsize(parent);
    int ss = lgecryptow_crypto_shash_statesize(parent);
    char *opad = lgecryptow_crypto_shash_ctx_aligned(parent) + ss;
    struct shash_desc *desc = lgecryptow_shash_desc_ctx(pdesc);

    desc->flags = pdesc->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

    return lgecryptow_crypto_shash_finup(desc, data, nbytes, out) ?:
           lgecryptow_crypto_shash_import(desc, opad) ?:
           lgecryptow_crypto_shash_finup(desc, out, ds, out);
}

static int hmac_init_tfm(struct crypto_tfm *tfm)
{
    struct crypto_shash *parent = lgecryptow_crypto_shash_cast(tfm);
    struct crypto_shash *hash;
    struct crypto_instance *inst = (void *)tfm->__crt_alg;
    struct crypto_shash_spawn *spawn = lgecryptow_crypto_instance_ctx(inst);
    struct hmac_ctx *ctx = hmac_ctx(parent);

    hash = lgecryptow_crypto_spawn_shash(spawn);
    if (IS_ERR(hash))
        return PTR_ERR(hash);

    parent->descsize = sizeof(struct shash_desc) +
               lgecryptow_crypto_shash_descsize(hash);

    ctx->hash = hash;
    return 0;
}

static void hmac_exit_tfm(struct crypto_tfm *tfm)
{
    struct hmac_ctx *ctx = hmac_ctx(lgecryptow_crypto_shash_cast(tfm));
    lgecryptow_crypto_free_shash(ctx->hash);
}

static int hmac_create(struct crypto_template *tmpl, struct rtattr **tb)
{
    struct shash_instance *inst;
    struct crypto_alg *alg;
    struct shash_alg *salg;
    int err;
    int ds;
    int ss;

    err = lgecryptow_crypto_check_attr_type(tb, CRYPTO_ALG_TYPE_SHASH);
    if (err)
        return err;

    salg = lgecryptow_shash_attr_alg(tb[1], 0, 0);
    if (IS_ERR(salg))
        return PTR_ERR(salg);

    err = -EINVAL;
    ds = salg->digestsize;
    ss = salg->statesize;
    alg = &salg->base;
    if (ds > alg->cra_blocksize ||
        ss < alg->cra_blocksize)
        goto out_put_alg;

    inst = lgecryptow_shash_alloc_instance("lge-hmac", alg);
    err = PTR_ERR(inst);
    if (IS_ERR(inst))
        goto out_put_alg;

    err = lgecryptow_crypto_init_shash_spawn(lgecryptow_shash_instance_ctx(inst), salg,
                      lgecryptow_shash_crypto_instance(inst));
    if (err)
        goto out_free_inst;

    inst->alg.base.cra_priority = alg->cra_priority;
    inst->alg.base.cra_blocksize = alg->cra_blocksize;
    inst->alg.base.cra_alignmask = alg->cra_alignmask;

    ss = ALIGN(ss, alg->cra_alignmask + 1);
    inst->alg.digestsize = ds;
    inst->alg.statesize = ss;

    inst->alg.base.cra_ctxsize = sizeof(struct hmac_ctx) +
                     ALIGN(ss * 2, lgecryptow_crypto_tfm_ctx_alignment());

    inst->alg.base.cra_init = hmac_init_tfm;
    inst->alg.base.cra_exit = hmac_exit_tfm;

    inst->alg.init = hmac_init;
    inst->alg.update = hmac_update;
    inst->alg.final = hmac_final;
    inst->alg.finup = hmac_finup;
    inst->alg.export = hmac_export;
    inst->alg.import = hmac_import;
    inst->alg.setkey = hmac_setkey;

    err = lgecryptow_shash_register_instance(tmpl, inst);
    if (err) {
out_free_inst:
        lgecryptow_shash_free_instance(lgecryptow_shash_crypto_instance(inst));
    }

out_put_alg:
    lgecryptow_crypto_mod_put(alg);
    return err;
}

static struct crypto_template hmac_tmpl = {
    .name = "lge-hmac",
    .create = hmac_create,
    .free = lgecryptow_shash_free_instance,
    .module = THIS_MODULE,
};

int __init hmac_module_init(void)
{
    return lgecryptow_crypto_register_template(&hmac_tmpl);
}

void __exit hmac_module_exit(void)
{
    lgecryptow_crypto_unregister_template(&hmac_tmpl);
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HMAC hash algorithm");
