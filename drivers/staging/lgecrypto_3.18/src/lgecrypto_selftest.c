/*
 * Algorithm testing framework and tests.
 *
 * Copyright (c) 2002 James Morris <jmorris@intercode.com.au>
 * Copyright (c) 2002 Jean-Francois Dive <jef@linuxbe.org>
 * Copyright (c) 2007 Nokia Siemens Networks
 * Copyright (c) 2008 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * Updated RFC4106 AES-GCM testing.
 *    Authors: Aidan O'Mahony (aidan.o.mahony@intel.com)
 *             Adrian Hoban <adrian.hoban@intel.com>
 *             Gabriele Paoloni <gabriele.paoloni@intel.com>
 *             Tadeusz Struk (tadeusz.struk@intel.com)
 *    Copyright (c) 2010, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <linux/err.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "drbg.h"
#include "sha.h"
#include "lgecrypto_selftest.h"
#include "../lgecrypto_wrapper.h"

/*
 * Need slab memory for testing (size in number of pages).
 */
#define XBUFSIZE    8

/*
 * Indexes into the xbuf to simulate cross-page access.
 */
#define IDX1        32
#define IDX2        32400
#define IDX3        1
#define IDX4        8193
#define IDX5        22222
#define IDX6        17101
#define IDX7        27333
#define IDX8        3000

/*
* Used by test_cipher()
*/
#define ENCRYPT 1
#define DECRYPT 0

struct tcrypt_result {
    struct completion completion;
    int err;
};

struct cipher_test_suite {
    struct {
        struct cipher_testvec *vecs;
        unsigned int count;
    } enc, dec;
};

struct hash_test_suite {
    struct hash_testvec *vecs;
    unsigned int count;
};


struct drbg_test_suite {
    struct drbg_testvec *vecs;
    unsigned int count;
};

struct alg_test_desc {
    const char *alg;
    int (*test)(const struct alg_test_desc *desc, const char *driver,
            u32 type, u32 mask);
    int fips_allowed;    /* set if alg is allowed in fips mode */

    union {
        struct cipher_test_suite cipher;
        struct hash_test_suite hash;
        struct drbg_test_suite drbg;
    } suite;
};

static unsigned int IDX[8] = { IDX1, IDX2, IDX3, IDX4, IDX5, IDX6, IDX7, IDX8 };

#if FIPS_FUNC_TEST == 4
void hexdump(unsigned char *buf, unsigned int len)
#else
static void hexdump(unsigned char *buf, unsigned int len)
#endif
{
    print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
            16, 1,
            buf, len, false);
}

static void tcrypt_complete(struct crypto_async_request *req, int err)
{
    struct tcrypt_result *res = req->data;

    if (err == -EINPROGRESS)
        return;

    res->err = err;
    complete(&res->completion);
}

static int testmgr_alloc_buf(char *buf[XBUFSIZE])
{
    int i;

    for (i = 0; i < XBUFSIZE; i++) {
        buf[i] = (void *)__get_free_page(GFP_KERNEL);
        if (!buf[i])
            goto err_free_buf;
    }

    return 0;

err_free_buf:
    while (i-- > 0)
        free_page((unsigned long)buf[i]);

    return -ENOMEM;
}

static void testmgr_free_buf(char *buf[XBUFSIZE])
{
    int i;

    for (i = 0; i < XBUFSIZE; i++)
        free_page((unsigned long)buf[i]);
}

static int do_one_async_hash_op(struct ahash_request *req,
                struct tcrypt_result *tr,
                int ret)
{
    if (ret == -EINPROGRESS || ret == -EBUSY) {
        ret = wait_for_completion_interruptible(&tr->completion);
        if (!ret)
            ret = tr->err;
        reinit_completion(&(tr->completion));
    }
    return ret;
}

static int test_hash(struct crypto_ahash *tfm, struct hash_testvec *template,
             unsigned int tcount, bool use_digest)
{
    const char *algo = lgecryptow_crypto_tfm_alg_driver_name(lgecryptow_crypto_ahash_tfm(tfm));
    unsigned int i, j, k, temp;
    struct scatterlist sg[8];
    char result[64];
    struct ahash_request *req;
    struct tcrypt_result tresult;
    void *hash_buff;
    char *xbuf[XBUFSIZE];
    int ret = -ENOMEM;

    if (testmgr_alloc_buf(xbuf))
        goto out_nobuf;

    init_completion(&tresult.completion);

    req = lgecryptow_ahash_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        printk(KERN_ERR "alg: hash: Failed to allocate request for "
               "%s\n", algo);
        goto out_noreq;
    }
    lgecryptow_ahash_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG,
                   tcrypt_complete, &tresult);

    j = 0;
    for (i = 0; i < tcount; i++) {
        if (template[i].np)
            continue;

        j++;
        memset(result, 0, 64);

        hash_buff = xbuf[0];

        memcpy(hash_buff, template[i].plaintext, template[i].psize);
        sg_init_one(&sg[0], hash_buff, template[i].psize);

        if (template[i].ksize) {
            lgecryptow_crypto_ahash_clear_flags(tfm, ~0);
            ret = lgecryptow_crypto_ahash_setkey(tfm, template[i].key,
                          template[i].ksize);
            if (ret) {
                printk(KERN_ERR "alg: hash: setkey failed on "
                       "test %d for %s: ret=%d\n", j, algo,
                       -ret);
                goto out;
            }
        }

        lgecryptow_ahash_request_set_crypt(req, sg, result, template[i].psize);
        if (use_digest) {
            ret = do_one_async_hash_op(req, &tresult,
                           lgecryptow_crypto_ahash_digest(req));
            if (ret) {
                pr_err("alg: hash: digest failed on test %d "
                       "for %s: ret=%d\n", j, algo, -ret);
                goto out;
            }
        } else {
            ret = do_one_async_hash_op(req, &tresult,
                           lgecryptow_crypto_ahash_init(req));
            if (ret) {
                pr_err("alt: hash: init failed on test %d "
                       "for %s: ret=%d\n", j, algo, -ret);
                goto out;
            }
            ret = do_one_async_hash_op(req, &tresult,
                           lgecryptow_crypto_ahash_update(req));
            if (ret) {
                pr_err("alt: hash: update failed on test %d "
                       "for %s: ret=%d\n", j, algo, -ret);
                goto out;
            }
            ret = do_one_async_hash_op(req, &tresult,
                           lgecryptow_crypto_ahash_final(req));
            if (ret) {
                pr_err("alt: hash: final failed on test %d "
                       "for %s: ret=%d\n", j, algo, -ret);
                goto out;
            }
        }

        if (memcmp(result, template[i].digest,
               lgecryptow_crypto_ahash_digestsize(tfm))) {
            printk(KERN_ERR "alg: hash: Test %d failed for %s\n",
                   j, algo);
            hexdump(result, lgecryptow_crypto_ahash_digestsize(tfm));
            ret = -EINVAL;
            goto out;
        }
    }

    j = 0;
    for (i = 0; i < tcount; i++) {
        if (template[i].np) {
            j++;
            memset(result, 0, 64);

            temp = 0;
            sg_init_table(sg, template[i].np);
            ret = -EINVAL;
            for (k = 0; k < template[i].np; k++) {
                if (WARN_ON(offset_in_page(IDX[k]) +
                        template[i].tap[k] > PAGE_SIZE))
                    goto out;
                sg_set_buf(&sg[k],
                       memcpy(xbuf[IDX[k] >> PAGE_SHIFT] +
                          offset_in_page(IDX[k]),
                          template[i].plaintext + temp,
                          template[i].tap[k]),
                       template[i].tap[k]);
                temp += template[i].tap[k];
            }

            if (template[i].ksize) {
                lgecryptow_crypto_ahash_clear_flags(tfm, ~0);
                ret = lgecryptow_crypto_ahash_setkey(tfm, template[i].key,
                              template[i].ksize);

                if (ret) {
                    printk(KERN_ERR "alg: hash: setkey "
                           "failed on chunking test %d "
                           "for %s: ret=%d\n", j, algo,
                           -ret);
                    goto out;
                }
            }

            lgecryptow_ahash_request_set_crypt(req, sg, result,
                        template[i].psize);
            ret = lgecryptow_crypto_ahash_digest(req);
            switch (ret) {
            case 0:
                break;
            case -EINPROGRESS:
            case -EBUSY:
                ret = wait_for_completion_interruptible(
                    &tresult.completion);
                if (!ret && !(ret = tresult.err)) {
                    reinit_completion(&tresult.completion);
                    break;
                }
                /* fall through */
            default:
                printk(KERN_ERR "alg: hash: digest failed "
                       "on chunking test %d for %s: "
                       "ret=%d\n", j, algo, -ret);
                goto out;
            }

            if (memcmp(result, template[i].digest,
                   lgecryptow_crypto_ahash_digestsize(tfm))) {
                printk(KERN_ERR "alg: hash: Chunking test %d "
                       "failed for %s\n", j, algo);
                hexdump(result, lgecryptow_crypto_ahash_digestsize(tfm));
                ret = -EINVAL;
                goto out;
            }
        }
    }

    ret = 0;

out:
    lgecryptow_ahash_request_free(req);
out_noreq:
    testmgr_free_buf(xbuf);
out_nobuf:
    return ret;
}

static int test_cipher(struct crypto_cipher *tfm, int enc,
               struct cipher_testvec *template, unsigned int tcount)
{
    const char *algo = lgecryptow_crypto_tfm_alg_driver_name(crypto_cipher_tfm(tfm));
    unsigned int i, j, k;
    char *q;
    const char *e;
    void *data;
    char *xbuf[XBUFSIZE];
    int ret = -ENOMEM;

    if (testmgr_alloc_buf(xbuf))
        goto out_nobuf;

    if (enc == ENCRYPT)
            e = "encryption";
    else
        e = "decryption";

    j = 0;
    for (i = 0; i < tcount; i++) {
        if (template[i].np)
            continue;

        j++;

        ret = -EINVAL;
        if (WARN_ON(template[i].ilen > PAGE_SIZE))
            goto out;

        data = xbuf[0];
        memcpy(data, template[i].input, template[i].ilen);

        lgecryptow_crypto_cipher_clear_flags(tfm, ~0);
        if (template[i].wk)
            lgecryptow_crypto_cipher_set_flags(tfm, CRYPTO_TFM_REQ_WEAK_KEY);

        ret = lgecryptow_crypto_cipher_setkey(tfm, template[i].key,
                       template[i].klen);
        if (ret != template[i].fail) {
            printk(KERN_ERR "alg: cipher: setkey failed "
                   "on test %d for %s: flags=%x\n", j,
                   algo, lgecryptow_crypto_cipher_get_flags(tfm));
            goto out;
        } else if (ret)
            continue;

        for (k = 0; k < template[i].ilen;
             k += lgecryptow_crypto_cipher_blocksize(tfm)) {
            if (enc)
                lgecryptow_crypto_cipher_encrypt_one(tfm, data + k,
                              data + k);
            else
                lgecryptow_crypto_cipher_decrypt_one(tfm, data + k,
                              data + k);
        }

        q = data;
        if (memcmp(q, template[i].result, template[i].rlen)) {
            printk(KERN_ERR "alg: cipher: Test %d failed "
                   "on %s for %s\n", j, e, algo);
            hexdump(q, template[i].rlen);
            ret = -EINVAL;
            goto out;
        }
    }

    ret = 0;

out:
    testmgr_free_buf(xbuf);
out_nobuf:
    return ret;
}

static int __test_skcipher(struct crypto_ablkcipher *tfm, int enc,
               struct cipher_testvec *template, unsigned int tcount,
               const bool diff_dst)
{
    const char *algo =
        lgecryptow_crypto_tfm_alg_driver_name(lgecryptow_crypto_ablkcipher_tfm(tfm));
    unsigned int i, j, k, n, temp;
    char *q;
    struct ablkcipher_request *req;
    struct scatterlist sg[8];
    struct scatterlist sgout[8];
    const char *e, *d;
    struct tcrypt_result result;
    void *data;
    char iv[MAX_IVLEN];
    char *xbuf[XBUFSIZE];
    char *xoutbuf[XBUFSIZE];
    int ret = -ENOMEM;

    if (testmgr_alloc_buf(xbuf))
        goto out_nobuf;

    if (diff_dst && testmgr_alloc_buf(xoutbuf))
        goto out_nooutbuf;

    if (diff_dst)
        d = "-ddst";
    else
        d = "";

    if (enc == ENCRYPT)
            e = "encryption";
    else
        e = "decryption";

    init_completion(&result.completion);

    req = lgecryptow_ablkcipher_request_alloc(tfm, GFP_KERNEL);
    if (!req) {
        pr_err("alg: skcipher%s: Failed to allocate request for %s\n",
               d, algo);
        goto out;
    }

    lgecryptow_ablkcipher_request_set_callback(req, CRYPTO_TFM_REQ_MAY_BACKLOG,
                    tcrypt_complete, &result);

    j = 0;
    for (i = 0; i < tcount; i++) {
        if (template[i].iv)
            memcpy(iv, template[i].iv, MAX_IVLEN);
        else
            memset(iv, 0, MAX_IVLEN);

        if (!(template[i].np) || (template[i].also_non_np)) {
            j++;

            ret = -EINVAL;
            if (WARN_ON(template[i].ilen > PAGE_SIZE))
                goto out;

            data = xbuf[0];
            memcpy(data, template[i].input, template[i].ilen);

            lgecryptow_crypto_ablkcipher_clear_flags(tfm, ~0);
            if (template[i].wk)
                lgecryptow_crypto_ablkcipher_set_flags(
                    tfm, CRYPTO_TFM_REQ_WEAK_KEY);

            ret = lgecryptow_crypto_ablkcipher_setkey(tfm, template[i].key,
                               template[i].klen);
            if (ret == template[i].fail) {
                pr_err("alg: skcipher%s: setkey failed on test %d for %s: flags=%x\n",
                       d, j, algo,
                       lgecryptow_crypto_ablkcipher_get_flags(tfm));
                goto out;
            } else if (ret)
                continue;

            sg_init_one(&sg[0], data, template[i].ilen);
            if (diff_dst) {
                data = xoutbuf[0];
                sg_init_one(&sgout[0], data, template[i].ilen);
            }

            lgecryptow_ablkcipher_request_set_crypt(req, sg,
                             (diff_dst) ? sgout : sg,
                             template[i].ilen, iv);
            ret = enc ?
                lgecryptow_crypto_ablkcipher_encrypt(req) :
                lgecryptow_crypto_ablkcipher_decrypt(req);

            switch (ret) {
            case 0:
                break;
            case -EINPROGRESS:
            case -EBUSY:
                ret = wait_for_completion_interruptible(
                    &result.completion);
                if (!ret && !((ret = result.err))) {
                    reinit_completion(&result.completion);
                    break;
                }
                /* fall through */
            default:
                pr_err("alg: skcipher%s: %s failed on test %d for %s: ret=%d\n",
                       d, e, j, algo, -ret);
                goto out;
            }

            q = data;
            if (memcmp(q, template[i].result, template[i].rlen)) {
                pr_err("alg: skcipher%s: Test %d failed on %s for %s\n",
                       d, j, e, algo);
                hexdump(q, template[i].rlen);
                ret = -EINVAL;
                goto out;
            }
        }
    }

    j = 0;
    for (i = 0; i < tcount; i++) {

        if (template[i].iv)
            memcpy(iv, template[i].iv, MAX_IVLEN);
        else
            memset(iv, 0, MAX_IVLEN);

        if (template[i].np) {
            j++;

            lgecryptow_crypto_ablkcipher_clear_flags(tfm, ~0);
            if (template[i].wk)
                lgecryptow_crypto_ablkcipher_set_flags(
                    tfm, CRYPTO_TFM_REQ_WEAK_KEY);

            ret = lgecryptow_crypto_ablkcipher_setkey(tfm, template[i].key,
                               template[i].klen);
            if (ret != template[i].fail) {
                pr_err("alg: skcipher%s: setkey failed on chunk test %d for %s: flags=%x\n",
                       d, j, algo,
                       lgecryptow_crypto_ablkcipher_get_flags(tfm));
                goto out;
            } else if (ret)
                continue;

            temp = 0;
            ret = -EINVAL;
            sg_init_table(sg, template[i].np);
            if (diff_dst)
                sg_init_table(sgout, template[i].np);
            for (k = 0; k < template[i].np; k++) {
                if (WARN_ON(offset_in_page(IDX[k]) +
                        template[i].tap[k] > PAGE_SIZE))
                    goto out;

                q = xbuf[IDX[k] >> PAGE_SHIFT] +
                    offset_in_page(IDX[k]);

                memcpy(q, template[i].input + temp,
                       template[i].tap[k]);

                if (offset_in_page(q) + template[i].tap[k] <
                    PAGE_SIZE)
                    q[template[i].tap[k]] = 0;

                sg_set_buf(&sg[k], q, template[i].tap[k]);
                if (diff_dst) {
                    q = xoutbuf[IDX[k] >> PAGE_SHIFT] +
                        offset_in_page(IDX[k]);

                    sg_set_buf(&sgout[k], q,
                           template[i].tap[k]);

                    memset(q, 0, template[i].tap[k]);
                    if (offset_in_page(q) +
                        template[i].tap[k] < PAGE_SIZE)
                        q[template[i].tap[k]] = 0;
                }

                temp += template[i].tap[k];
            }

            lgecryptow_ablkcipher_request_set_crypt(req, sg,
                    (diff_dst) ? sgout : sg,
                    template[i].ilen, iv);

            ret = enc ?
                lgecryptow_crypto_ablkcipher_encrypt(req) :
                lgecryptow_crypto_ablkcipher_decrypt(req);

            switch (ret) {
            case 0:
                break;
            case -EINPROGRESS:
            case -EBUSY:
                ret = wait_for_completion_interruptible(
                    &result.completion);
                if (!ret && !((ret = result.err))) {
                    reinit_completion(&result.completion);
                    break;
                }
                /* fall through */
            default:
                pr_err("alg: skcipher%s: %s failed on chunk test %d for %s: ret=%d\n",
                       d, e, j, algo, -ret);
                goto out;
            }

            temp = 0;
            ret = -EINVAL;
            for (k = 0; k < template[i].np; k++) {
                if (diff_dst)
                    q = xoutbuf[IDX[k] >> PAGE_SHIFT] +
                        offset_in_page(IDX[k]);
                else
                    q = xbuf[IDX[k] >> PAGE_SHIFT] +
                        offset_in_page(IDX[k]);

                if (memcmp(q, template[i].result + temp,
                       template[i].tap[k])) {
                    pr_err("alg: skcipher%s: Chunk test %d failed on %s at page %u for %s\n",
                           d, j, e, k, algo);
                    hexdump(q, template[i].tap[k]);
                    goto out;
                }

                q += template[i].tap[k];
                for (n = 0; offset_in_page(q + n) && q[n]; n++)
                    ;
                if (n) {
                    pr_err("alg: skcipher%s: Result buffer corruption in chunk test %d on %s at page %u for %s: %u bytes:\n",
                           d, j, e, k, algo, n);
                    hexdump(q, n);
                    goto out;
                }
                temp += template[i].tap[k];
            }
        }
    }

    ret = 0;

out:
    lgecryptow_ablkcipher_request_free(req);
    if (diff_dst)
        testmgr_free_buf(xoutbuf);
out_nooutbuf:
    testmgr_free_buf(xbuf);
out_nobuf:
    return ret;
}

static int test_skcipher(struct crypto_ablkcipher *tfm, int enc,
             struct cipher_testvec *template, unsigned int tcount)
{
    int ret;

    /* test 'dst == src' case */
    ret = __test_skcipher(tfm, enc, template, tcount, false);
    if (ret)
        return ret;

    /* test 'dst != src' case */
    return __test_skcipher(tfm, enc, template, tcount, true);
}

static int alg_selftest_cipher(const struct alg_test_desc *desc,
               const char *driver, u32 type, u32 mask)
{
    struct crypto_cipher *tfm;
    int err = 0;

    tfm = lgecryptow_crypto_alloc_cipher(driver, type, mask);
    if (IS_ERR(tfm)) {
        printk(KERN_ERR "alg: cipher: Failed to load transform for "
               "%s: %ld\n", driver, PTR_ERR(tfm));
        return PTR_ERR(tfm);
    }

    if (desc->suite.cipher.enc.vecs) {
        err = test_cipher(tfm, ENCRYPT, desc->suite.cipher.enc.vecs,
                  desc->suite.cipher.enc.count);
        if (err)
            goto out;
    }

    if (desc->suite.cipher.dec.vecs)
        err = test_cipher(tfm, DECRYPT, desc->suite.cipher.dec.vecs,
                  desc->suite.cipher.dec.count);

out:
    lgecryptow_crypto_free_cipher(tfm);
    return err;
}

static int alg_selftest_skcipher(const struct alg_test_desc *desc,
                 const char *driver, u32 type, u32 mask)
{
    struct crypto_ablkcipher *tfm;
    int err = 0;

    tfm = lgecryptow_crypto_alloc_ablkcipher(driver, type, mask);
    if (IS_ERR(tfm)) {
        printk(KERN_ERR "alg: skcipher: Failed to load transform for "
               "%s: %ld\n", driver, PTR_ERR(tfm));
        return PTR_ERR(tfm);
    }

    if (desc->suite.cipher.enc.vecs) {
        err = test_skcipher(tfm, ENCRYPT, desc->suite.cipher.enc.vecs,
                    desc->suite.cipher.enc.count);
        if (err)
            goto out;
    }

    if (desc->suite.cipher.dec.vecs)
        err = test_skcipher(tfm, DECRYPT, desc->suite.cipher.dec.vecs,
                    desc->suite.cipher.dec.count);

out:
    lgecryptow_crypto_free_ablkcipher(tfm);
    return err;
}

static int alg_selftest_hash(const struct alg_test_desc *desc, const char *driver,
             u32 type, u32 mask)
{
    struct crypto_ahash *tfm;
    int err;

    tfm = lgecryptow_crypto_alloc_ahash(driver, type, mask);
    if (IS_ERR(tfm)) {
        printk(KERN_ERR "alg: hash: Failed to load transform for %s: "
               "%ld\n", driver, PTR_ERR(tfm));
        return PTR_ERR(tfm);
    }

    err = test_hash(tfm, desc->suite.hash.vecs,
            desc->suite.hash.count, true);
    if (!err)
        err = test_hash(tfm, desc->suite.hash.vecs,
                desc->suite.hash.count, false);

    lgecryptow_crypto_free_ahash(tfm);
    return err;
}

static int drbg_cavs_test(struct drbg_testvec *test, int pr,
              const char *driver, u32 type, u32 mask)
{
    int ret = -EAGAIN;
    struct crypto_rng *drng;
    struct drbg_test_data test_data;
    struct drbg_string addtl, pers, testentropy;
    unsigned char *buf = kzalloc(test->expectedlen, GFP_KERNEL);

    if (!buf)
        return -ENOMEM;

    drng = lgecryptow_crypto_alloc_rng(driver, type, mask);
    if (IS_ERR(drng)) {
        printk(KERN_ERR "alg: drbg: could not allocate DRNG handle for "
               "%s\n", driver);
        kzfree(buf);
        return -ENOMEM;
    }

    test_data.testentropy = &testentropy;
    drbg_string_fill(&testentropy, test->entropy, test->entropylen);
    drbg_string_fill(&pers, test->pers, test->perslen);
    ret = crypto_drbg_reset_test(drng, &pers, &test_data);
    if (ret) {
        printk(KERN_ERR "alg: drbg: Failed to reset rng\n");
        goto outbuf;
    }

    drbg_string_fill(&addtl, test->addtla, test->addtllen);
    if (pr) {
        drbg_string_fill(&testentropy, test->entpra, test->entprlen);
        ret = crypto_drbg_get_bytes_addtl_test(drng,
            buf, test->expectedlen, &addtl, &test_data);
    } else {
        ret = crypto_drbg_get_bytes_addtl(drng,
            buf, test->expectedlen, &addtl);
    }
    if (ret < 0) {
        printk(KERN_ERR "alg: drbg: could not obtain random data for "
               "driver %s\n", driver);
        goto outbuf;
    }

    drbg_string_fill(&addtl, test->addtlb, test->addtllen);
    if (pr) {
        drbg_string_fill(&testentropy, test->entprb, test->entprlen);
        ret = crypto_drbg_get_bytes_addtl_test(drng,
            buf, test->expectedlen, &addtl, &test_data);
    } else {
        ret = crypto_drbg_get_bytes_addtl(drng,
            buf, test->expectedlen, &addtl);
    }
    if (ret < 0) {
        printk(KERN_ERR "alg: drbg: could not obtain random data for "
               "driver %s\n", driver);
        goto outbuf;
    }

    ret = memcmp(test->expected, buf, test->expectedlen);

outbuf:
    lgecryptow_crypto_free_rng(drng);
    kzfree(buf);
    return ret;
}

static int alg_selftest_drbg(const struct alg_test_desc *desc, const char *driver,
             u32 type, u32 mask)
{
    int err = 0;
    int pr = 0;
    int i = 0;
    struct drbg_testvec *template = desc->suite.drbg.vecs;
    unsigned int tcount = desc->suite.drbg.count;

    if (0 == memcmp(driver, "drbg_pr_", 8))
        pr = 1;

    for (i = 0; i < tcount; i++) {
        err = drbg_cavs_test(&template[i], pr, driver, type, mask);
        if (err) {
            printk(KERN_ERR "alg: drbg: Test %d failed for %s\n",
                   i, driver);
            err = -EINVAL;
            break;
        }
    }
    return err;

}

static int alg_selftest_null(const struct alg_test_desc *desc,
                 const char *driver, u32 type, u32 mask)
{
    return 0;
}

/* Please keep this list sorted by algorithm name. */
static const struct alg_test_desc alg_test_descs[] = {
    {
        .alg = "__lge-cbc-aes-ce",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_cbc_enc_tv_template,
                    .count = AES_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_cbc_dec_tv_template,
                    .count = AES_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-cbc-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_cbc_enc_tv_template,
                    .count = AES_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_cbc_dec_tv_template,
                    .count = AES_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-ctr-aes-ce",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_ctr_enc_tv_template,
                    .count = AES_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_ctr_dec_tv_template,
                    .count = AES_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-ctr-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_ctr_enc_tv_template,
                    .count = AES_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_ctr_dec_tv_template,
                    .count = AES_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-ecb-aes-ce",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_enc_tv_template,
                    .count = AES_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_dec_tv_template,
                    .count = AES_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-xts-aes-ce",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_xts_enc_tv_template,
                    .count = AES_XTS_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_xts_dec_tv_template,
                    .count = AES_XTS_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "__lge-xts-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_xts_enc_tv_template,
                    .count = AES_XTS_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_xts_dec_tv_template,
                    .count = AES_XTS_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "drbg_nopr_ctr_aes128",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_nopr_ctr_aes128_tv_template,
                .count = ARRAY_SIZE(drbg_nopr_ctr_aes128_tv_template)
            }
        }
    }, {
        .alg = "drbg_nopr_ctr_aes192",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_nopr_ctr_aes192_tv_template,
                .count = ARRAY_SIZE(drbg_nopr_ctr_aes192_tv_template)
            }
        }
    }, {
        .alg = "drbg_nopr_ctr_aes256",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_nopr_ctr_aes256_tv_template,
                .count = ARRAY_SIZE(drbg_nopr_ctr_aes256_tv_template)
            }
        }
    }, {
        /*
         * There is no need to specifically test the DRBG with every
         * backend cipher -- covered by drbg_nopr_hmac_sha256 test
         */
        .alg = "drbg_nopr_hmac_sha1",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_nopr_hmac_sha256",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_nopr_hmac_sha256_tv_template,
                .count = ARRAY_SIZE(drbg_nopr_hmac_sha256_tv_template)
            }
        }
    }, {
        /* covered by drbg_nopr_hmac_sha256 test */
        .alg = "drbg_nopr_hmac_sha384",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_nopr_hmac_sha512",
        .test = alg_selftest_null,
        .fips_allowed = 1,
    }, {
        .alg = "drbg_nopr_sha1",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_nopr_sha256",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_nopr_sha256_tv_template,
                .count = ARRAY_SIZE(drbg_nopr_sha256_tv_template)
            }
        }
    }, {
        /* covered by drbg_nopr_sha256 test */
        .alg = "drbg_nopr_sha384",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_nopr_sha512",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_ctr_aes128",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_pr_ctr_aes128_tv_template,
                .count = ARRAY_SIZE(drbg_pr_ctr_aes128_tv_template)
            }
        }
    }, {
        /* covered by drbg_pr_ctr_aes128 test */
        .alg = "drbg_pr_ctr_aes192",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_ctr_aes256",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_hmac_sha1",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_hmac_sha256",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_pr_hmac_sha256_tv_template,
                .count = ARRAY_SIZE(drbg_pr_hmac_sha256_tv_template)
            }
        }
    }, {
        /* covered by drbg_pr_hmac_sha256 test */
        .alg = "drbg_pr_hmac_sha384",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_hmac_sha512",
        .test = alg_selftest_null,
        .fips_allowed = 1,
    }, {
        .alg = "drbg_pr_sha1",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_sha256",
        .test = alg_selftest_drbg,
        .fips_allowed = 1,
        .suite = {
            .drbg = {
                .vecs = drbg_pr_sha256_tv_template,
                .count = ARRAY_SIZE(drbg_pr_sha256_tv_template)
            }
        }
    }, {
        /* covered by drbg_pr_sha256 test */
        .alg = "drbg_pr_sha384",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "drbg_pr_sha512",
        .fips_allowed = 1,
        .test = alg_selftest_null,
    }, {
        .alg = "ecb(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_enc_tv_template,
                    .count = AES_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_dec_tv_template,
                    .count = AES_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "ecb(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_enc_tv_template,
                    .count = AES_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_dec_tv_template,
                    .count = AES_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "ecb(lge-des3_ede)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = des3_ede_enc_tv_template,
                    .count = DES3_EDE_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = des3_ede_dec_tv_template,
                    .count = DES3_EDE_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-cbc(aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_cbc_enc_tv_template,
                    .count = AES_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_cbc_dec_tv_template,
                    .count = AES_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-cbc(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_cbc_enc_tv_template,
                    .count = AES_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_cbc_dec_tv_template,
                    .count = AES_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-cbc(lge-des3_ede)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = des3_ede_cbc_enc_tv_template,
                    .count = DES3_EDE_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = des3_ede_cbc_dec_tv_template,
                    .count = DES3_EDE_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-cbc-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_cbc_enc_tv_template,
                    .count = AES_CBC_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_cbc_dec_tv_template,
                    .count = AES_CBC_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ctr(aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_ctr_enc_tv_template,
                    .count = AES_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_ctr_dec_tv_template,
                    .count = AES_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ctr(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_ctr_enc_tv_template,
                    .count = AES_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_ctr_dec_tv_template,
                    .count = AES_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ctr(lge-des3_ede)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = des3_ede_ctr_enc_tv_template,
                    .count = DES3_EDE_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = des3_ede_ctr_dec_tv_template,
                    .count = DES3_EDE_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ctr-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_ctr_enc_tv_template,
                    .count = AES_CTR_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_ctr_dec_tv_template,
                    .count = AES_CTR_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ecb(aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_enc_tv_template,
                    .count = AES_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_dec_tv_template,
                    .count = AES_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ecb(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_enc_tv_template,
                    .count = AES_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_dec_tv_template,
                    .count = AES_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-ecb(lge-des3_ede)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = des3_ede_enc_tv_template,
                    .count = DES3_EDE_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = des3_ede_dec_tv_template,
                    .count = DES3_EDE_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-hmac(lge-sha1)",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = hmac_sha1_tv_template,
                .count = HMAC_SHA1_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-hmac(lge-sha224)",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = hmac_sha224_tv_template,
                .count = HMAC_SHA224_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-hmac(lge-sha256)",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = hmac_sha256_tv_template,
                .count = HMAC_SHA256_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-hmac(lge-sha384)",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = hmac_sha384_tv_template,
                .count = HMAC_SHA384_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-hmac(lge-sha512)",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = hmac_sha512_tv_template,
                .count = HMAC_SHA512_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha1",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha1_tv_template,
                .count = SHA1_TEST_VECTORS
            }
        }
    }, {
     .alg = "lge-sha1-generic",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha1_tv_template,
                .count = SHA1_TEST_VECTORS
            }
        }
    }, {
     .alg = "lge-sha1-neon",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha1_tv_template,
                .count = SHA1_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha224",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha224_tv_template,
                .count = SHA224_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha224-generic",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha224_tv_template,
                .count = SHA224_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha224-neon",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha224_tv_template,
                .count = SHA224_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha256",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha256_tv_template,
                .count = SHA256_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha256-generic",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha256_tv_template,
                .count = SHA256_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha256-neon",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha256_tv_template,
                .count = SHA256_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha384",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha384_tv_template,
                .count = SHA384_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha384-generic",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha384_tv_template,
                .count = SHA384_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha384-neon",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha384_tv_template,
                .count = SHA384_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha512",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha512_tv_template,
                .count = SHA512_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha512-generic",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha512_tv_template,
                .count = SHA512_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-sha512-neon",
        .test = alg_selftest_hash,
        .fips_allowed = 1,
        .suite = {
            .hash = {
                .vecs = sha512_tv_template,
                .count = SHA512_TEST_VECTORS
            }
        }
    }, {
        .alg = "lge-xts(aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_xts_enc_tv_template,
                    .count = AES_XTS_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_xts_dec_tv_template,
                    .count = AES_XTS_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-xts(lge-aes)",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_xts_enc_tv_template,
                    .count = AES_XTS_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_xts_dec_tv_template,
                    .count = AES_XTS_DEC_TEST_VECTORS
                }
            }
        }
    }, {
        .alg = "lge-xts-aes-neonbs",
        .test = alg_selftest_skcipher,
        .fips_allowed = 1,
        .suite = {
            .cipher = {
                .enc = {
                    .vecs = aes_xts_enc_tv_template,
                    .count = AES_XTS_ENC_TEST_VECTORS
                },
                .dec = {
                    .vecs = aes_xts_dec_tv_template,
                    .count = AES_XTS_DEC_TEST_VECTORS
                }
            }
        }
    }

};

static int alg_find_test(const char *alg)
{
    int start = 0;
    int end = ARRAY_SIZE(alg_test_descs);

    while (start < end) {
        int i = (start + end) / 2;
        int diff = strcmp(alg_test_descs[i].alg, alg);

        if (diff > 0) {
            end = i;
            continue;
        }

        if (diff < 0) {
            start = i + 1;
            continue;
        }

        return i;
    }

    return -1;
}

int alg_selftest(const char *driver, const char *alg, u32 type, u32 mask)
{
    int i;
    int j;
    int rc = 0;

    if ((type & CRYPTO_ALG_TYPE_MASK) == CRYPTO_ALG_TYPE_CIPHER) {
        char nalg[CRYPTO_MAX_ALG_NAME];

        if (snprintf(nalg, sizeof(nalg), "ecb(%s)", alg) >=
            sizeof(nalg))
            return -ENAMETOOLONG;

        i = alg_find_test(nalg);
        if (i < 0)
            goto disallowed;

        rc = alg_selftest_cipher(alg_test_descs + i, driver, type, mask);
        goto test_done;
    }

    i = alg_find_test(alg);
    j = alg_find_test(driver);

    if (i < 0 && j < 0)
        goto disallowed;

    printk(KERN_INFO "alg: self-tests for %s (%s) starting\n", driver, alg);
    printk(KERN_INFO " [CCAudit] klcm:alg: self-tests for %s (%s) starting\n", driver, alg);

    rc = 0;
    if (i >= 0)
        rc |= alg_test_descs[i].test(alg_test_descs + i, driver, type, mask);
    if (j >= 0 && j != i)
        rc |= alg_test_descs[j].test(alg_test_descs + j, driver, type, mask);

test_done:
    if (rc == -ENOENT) { // algorithm not present
        printk(KERN_INFO "alg: %s (%s) is not available\n", driver, alg);
        printk(KERN_INFO " [CCAudit] klcm:alg: %s (%s) is not available\n", driver, alg);
        return -EINVAL;
    }
    else if (rc) { // algorithm present but failed
        printk(KERN_INFO "alg: self-tests for %s (%s) failed in FIPS mode\n", driver, alg);
        printk(KERN_INFO " [CCAudit] klcm:alg: self-tests for %s (%s) failed in FIPS mode\n", driver, alg);
        set_klmfips_state(FIPS_STATUS_FAIL);
    }
    else // algorithm passed
    {
        printk(KERN_INFO "alg: self-tests for %s (%s) passed\n", driver, alg);
        printk(KERN_INFO " [CCAudit] klcm:alg: self-tests for %s (%s) passed\n", driver, alg);
    }

    return rc;

disallowed:
    printk(KERN_INFO "alg(non-FIPS): %s (%s) cannot be used in FIPS mode\n", driver, alg);
    printk(KERN_INFO " [CCAudit] klcm:alg(non-FIPS): %s (%s) cannot be used in FIPS mode\n", driver, alg);
    return -EINVAL;
}

EXPORT_SYMBOL_GPL(alg_selftest);
