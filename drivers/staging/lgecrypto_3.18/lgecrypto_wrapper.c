/* Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Alternatively, and instead of the terms immediately above, this
 * software may be relicensed by the recipient at their option under the
 * terms of the GNU General Public License version 2 ("GPL") and only
 * version 2.  If the recipient chooses to relicense the software under
 * the GPL, then the recipient shall replace all of the text immediately
 * above and including this paragraph with the text immediately below
 * and between the words START OF ALTERNATE GPL TERMS and END OF
 * ALTERNATE GPL TERMS and such notices and license terms shall apply
 * INSTEAD OF the notices and licensing terms given above.
 *
 * START OF ALTERNATE GPL TERMS
 *
 * Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 *
 * This software was originally licensed under the Code Aurora Forum
 * Inc. Dual BSD/GPL License version 1.1 and relicensed as permitted
 * under the terms thereof by a recipient under the General Public
 * License Version 2.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * END OF ALTERNATE GPL TERMS
 *
 */
#include "src/lgecrypto_fips.h"
#include "lgecrypto_wrapper.h"

#ifdef CONFIG_LGCRYPTO_FIPS_ENABLE

enum fips_state g_fips_state = FIPS_STATUS_NA;

int lgecryptow_crypto_register_alg(struct crypto_alg *alg)
{
    return crypto_register_alg(alg);
}

int lgecryptow_crypto_unregister_alg(struct crypto_alg *alg)
{
    return crypto_unregister_alg(alg);
}

int lgecryptow_crypto_register_algs(struct crypto_alg *alg, int count)
{
    return crypto_register_algs(alg, count);
}

int lgecryptow_crypto_unregister_algs(struct crypto_alg *alg, int count)
{
    return crypto_unregister_algs(alg, count);
}

int lgecryptow_crypto_register_shash(struct shash_alg *alg)
{
    return crypto_register_shash(alg);
}

int lgecryptow_crypto_unregister_shash(struct shash_alg *alg)
{
    return crypto_unregister_shash(alg);
}

int lgecryptow_crypto_register_shashes(struct shash_alg *alg, int count)
{
    return crypto_register_shashes(alg, count);
}

int lgecryptow_crypto_unregister_shashes(struct shash_alg *alg, int count)
{
    return crypto_unregister_shashes(alg, count);
}

int lgecryptow_crypto_register_template(struct crypto_template *tmpl)
{
    return crypto_register_template(tmpl);
}

void lgecryptow_crypto_unregister_template(struct crypto_template *tmpl)
{
    crypto_unregister_template(tmpl);
}

void *lgecryptow_crypto_tfm_ctx(struct crypto_tfm *tfm)
{
    return crypto_tfm_ctx(tfm);
}

unsigned int lgecryptow_crypto_tfm_ctx_alignment(void)
{
    return crypto_tfm_ctx_alignment();
}

void lgecryptow_crypto_cipher_clear_flags(struct crypto_cipher *tfm, u32 flags)
{
    crypto_cipher_clear_flags(tfm, flags);
}

void lgecryptow_crypto_cipher_set_flags(struct crypto_cipher *tfm, u32 flags)
{
    crypto_cipher_set_flags(tfm, flags);
}

u32 lgecryptow_crypto_tfm_get_flags(struct crypto_tfm *tfm)
{
    return crypto_tfm_get_flags(tfm);
}

int lgecryptow_crypto_cipher_setkey(struct crypto_cipher *tfm, const u8 *key, unsigned int keylen)
{
    return crypto_cipher_setkey(tfm, key, keylen);
}

void lgecryptow_crypto_tfm_set_flags(struct crypto_tfm *tfm, u32 flags)
{
    crypto_tfm_set_flags(tfm, flags);
}

u32 lgecryptow_crypto_cipher_get_flags(struct crypto_cipher *tfm)
{
    return crypto_cipher_get_flags(tfm);
}

int lgecryptow_crypto_cipher_blocksize(struct crypto_cipher *tfm)
{
    return crypto_cipher_blocksize(tfm);
}

struct crypto_tfm *lgecryptow_crypto_cipher_tfm(struct crypto_cipher *tfm)
{
    return crypto_cipher_tfm(tfm);
}

struct cipher_alg *lgecryptow_crypto_cipher_alg(struct crypto_cipher *tfm)
{
    return crypto_cipher_alg(tfm);
}

void *lgecryptow_crypto_blkcipher_ctx(struct crypto_blkcipher *tfm)
{
    return crypto_blkcipher_ctx(tfm);
}

void *lgecryptow_crypto_instance_ctx(struct crypto_instance *inst)
{
    return crypto_instance_ctx(inst);
}

struct crypto_cipher *lgecryptow_crypto_spawn_cipher(struct crypto_spawn *spawn)
{
    return crypto_spawn_cipher(spawn);
}

void lgecryptow_crypto_free_cipher(struct crypto_cipher *tfm)
{
    crypto_free_cipher(tfm);
}

int lgecryptow_crypto_check_attr_type(struct rtattr **tb, u32 type)
{
    return crypto_check_attr_type(tb, type);
}

struct crypto_alg *lgecryptow_crypto_get_attr_alg(struct rtattr **tb, u32 type, u32 mask)
{
    return crypto_get_attr_alg(tb, type, mask);
}

void *lgecryptow_crypto_alloc_instance(const char *name, struct crypto_alg *alg)
{
    return crypto_alloc_instance(name, alg);
}

void lgecryptow_crypto_mod_put(struct crypto_alg *alg)
{
    crypto_mod_put(alg);
}

void lgecryptow_crypto_drop_spawn(struct crypto_spawn *spawn)
{
    crypto_drop_spawn(spawn);
}

void lgecryptow_crypto_xor(u8 *dst, const u8 *src, unsigned int size)
{
    crypto_xor(dst, src, size);
}

unsigned int lgecryptow_crypto_cipher_alignmask(struct crypto_cipher *tfm)
{
    return crypto_cipher_alignmask(tfm);
}

void lgecryptow_crypto_cipher_encrypt_one(struct crypto_cipher *tfm, u8 *dst, const u8 *src)
{
    crypto_cipher_encrypt_one(tfm, dst, src);
}

void lgecryptow_crypto_cipher_decrypt_one(struct crypto_cipher *tfm, u8 *dst, const u8 *src)
{
    crypto_cipher_decrypt_one(tfm, dst, src);
}

void lgecryptow_crypto_inc(u8 *a, unsigned int size)
{
    crypto_inc(a, size);
}

struct crypto_alg *lgecryptow_crypto_attr_alg(struct rtattr *rta, u32 type, u32 mask)
{
    return crypto_attr_alg(rta, type, mask);
}

void lgecryptow_be128_xor(be128 *r, const be128 *p, const be128 *q)
{
    be128_xor(r, p, q);
}

void lgecryptow_gf128mul_x_ble(be128 *r, const be128 *x)
{
    gf128mul_x_ble(r, x);
}

void *lgecryptow_shash_desc_ctx(struct shash_desc *desc)
{
    return shash_desc_ctx(desc);
}

void lgecryptow_sha_transform(__u32 *digest, const char *data, __u32 *W)
{
    sha_transform(digest, data, W);
}

int lgecryptow_blkcipher_walk_virt(struct blkcipher_desc *desc, struct blkcipher_walk *walk)
{
    return blkcipher_walk_virt(desc, walk);
}

int lgecryptow_blkcipher_walk_done(struct blkcipher_desc *desc, struct blkcipher_walk *walk, int err)
{
    return blkcipher_walk_done(desc, walk, err);
}

void lgecryptow_blkcipher_walk_init(struct blkcipher_walk *walk,
                                    struct scatterlist *dst,
                                    struct scatterlist *src,
                                    unsigned int nbytes)
{
    blkcipher_walk_init(walk, dst, src, nbytes);
}

int lgecryptow_blkcipher_walk_virt_block(struct blkcipher_desc *desc,
                                         struct blkcipher_walk *walk,
                                         unsigned int blocksize)
{
    return blkcipher_walk_virt_block(desc, walk, blocksize);
}

unsigned int lgecryptow_crypto_shash_statesize(struct crypto_shash *tfm)
{
    return crypto_shash_statesize(tfm);
}

void *lgecryptow_crypto_shash_ctx_aligned(struct crypto_shash *tfm)
{
    return crypto_shash_ctx_aligned(tfm);
}

unsigned int lgecryptow_crypto_shash_blocksize(struct crypto_shash *tfm)
{
    return crypto_shash_blocksize(tfm);
}

unsigned int lgecryptow_crypto_shash_digestsize(struct crypto_shash *tfm)
{
    return crypto_shash_digestsize(tfm);
}

unsigned int lgecryptow_crypto_shash_descsize(struct crypto_shash *tfm)
{
    return crypto_shash_descsize(tfm);
}

u32 lgecryptow_crypto_shash_get_flags(struct crypto_shash *tfm)
{
    return crypto_shash_get_flags(tfm);
}

int lgecryptow_crypto_shash_digest(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out)
{
    return crypto_shash_digest(desc, data, len, out);
}

int lgecryptow_crypto_shash_init(struct shash_desc *desc)
{
    return crypto_shash_init(desc);
}

int lgecryptow_crypto_shash_update(struct shash_desc *desc, const u8 *data, unsigned int len)
{
    return crypto_shash_update(desc, data, len);
}

int lgecryptow_crypto_shash_export(struct shash_desc *desc, void *out)
{
    return crypto_shash_export(desc, out);
}

int lgecryptow_crypto_shash_import(struct shash_desc *desc, const void *in)
{
    return crypto_shash_import(desc, in);
}

int lgecryptow_crypto_shash_final(struct shash_desc *desc, u8 *out)
{
    return crypto_shash_final(desc, out);
}

int lgecryptow_crypto_shash_finup(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out)
{
    return crypto_shash_finup(desc, data, len, out);
}

struct crypto_shash *lgecryptow_crypto_shash_cast(struct crypto_tfm *tfm)
{
    return __crypto_shash_cast(tfm);
}

struct crypto_shash *lgecryptow_crypto_spawn_shash(struct crypto_shash_spawn *spawn)
{
    return crypto_spawn_shash(spawn);
}

void lgecryptow_crypto_free_shash(struct crypto_shash *tfm)
{
    crypto_free_shash(tfm);
}

struct shash_alg *lgecryptow_shash_attr_alg(struct rtattr *rta, u32 type, u32 mask)
{
    return shash_attr_alg(rta, type, mask);
}

struct shash_instance *lgecryptow_shash_alloc_instance(const char *name, struct crypto_alg *alg)
{
    return shash_alloc_instance(name, alg);
}

int lgecryptow_crypto_init_shash_spawn(struct crypto_shash_spawn *spawn,
                                       struct shash_alg *alg,
                                       struct crypto_instance *inst)
{
    return crypto_init_shash_spawn(spawn, alg, inst);
}

struct crypto_instance *lgecryptow_shash_crypto_instance(struct shash_instance *inst)
{
    return shash_crypto_instance(inst);
}

void *lgecryptow_shash_instance_ctx(struct shash_instance *inst)
{
    return shash_instance_ctx(inst);
}

int lgecryptow_shash_register_instance(struct crypto_template *tmpl, struct shash_instance *inst)
{
    return shash_register_instance(tmpl, inst);
}

void lgecryptow_shash_free_instance(struct crypto_instance *inst)
{
    shash_free_instance(inst);
}

void lgecryptow_get_random_bytes(void *buf, int nbytes)
{
    get_random_bytes(buf, nbytes);
}

int lgecryptow_crypto_rng_get_bytes(struct crypto_rng *tfm, u8 *rdata, unsigned int dlen)
{
    return crypto_rng_get_bytes(tfm, rdata, dlen);
}

struct crypto_rng *lgecryptow_crypto_alloc_rng(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_rng(alg_name, type, mask);
}

void lgecryptow_crypto_free_rng(struct crypto_rng *tfm)
{
    crypto_free_rng(tfm);
}

void *lgecryptow_crypto_rng_ctx(struct crypto_rng *tfm)
{
    return crypto_rng_ctx(tfm);
}

struct crypto_shash *lgecryptow_crypto_alloc_shash(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_shash(alg_name, type, mask);
}

int lgecryptow_crypto_shash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen)
{
    return crypto_shash_setkey(tfm, key, keylen);
}

struct crypto_cipher *lgecryptow_crypto_alloc_cipher(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_cipher(alg_name, type, mask);
}

struct crypto_tfm *lgecryptow_crypto_rng_tfm(struct crypto_rng *tfm)
{
    return crypto_rng_tfm(tfm);
}

const char *lgecryptow_crypto_tfm_alg_driver_name(struct crypto_tfm *tfm)
{
    return crypto_tfm_alg_driver_name(tfm);
}

#if 0
int lgecryptow_crypto_register_rngs(struct rng_alg *algs, int count)
{
    return crypto_register_rngs(algs, count);
}

void lgecryptow_crypto_unregister_rngs(struct rng_alg *algs, int count)
{
    crypto_unregister_rngs(algs, count);
}
#endif

struct crypto_ablkcipher *lgecryptow_skcipher_givcrypt_reqtfm(struct skcipher_givcrypt_request *req)
{
    return skcipher_givcrypt_reqtfm(req);
}

void *lgecryptow_crypto_ablkcipher_ctx(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_ctx(tfm);
}

void *lgecryptow_skcipher_givcrypt_reqctx(struct skcipher_givcrypt_request *req)
{
    return skcipher_givcrypt_reqctx(req);
}

void lgecryptow_ablkcipher_request_set_tfm(struct ablkcipher_request *req, struct crypto_ablkcipher *tfm)
{
    ablkcipher_request_set_tfm(req, tfm);
}

struct crypto_ablkcipher *lgecryptow_skcipher_geniv_cipher(struct crypto_ablkcipher *geniv)
{
    return skcipher_geniv_cipher(geniv);
}

void lgecryptow_ablkcipher_request_set_callback(struct ablkcipher_request *req,
                                     u32 flags, crypto_completion_t complete, void *data)
{
    ablkcipher_request_set_callback(req, flags, complete, data);
}

void lgecryptow_ablkcipher_request_set_crypt(struct ablkcipher_request *req,
                                  struct scatterlist *src, struct scatterlist *dst,
                                  unsigned int nbytes, void *iv)
{
    ablkcipher_request_set_crypt(req, src, dst, nbytes, iv);
}

unsigned int lgecryptow_crypto_ablkcipher_ivsize(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_ivsize(tfm);
}

int lgecryptow_crypto_ablkcipher_encrypt(struct ablkcipher_request *req)
{
    return crypto_ablkcipher_encrypt(req);
}

int lgecryptow_crypto_ablkcipher_decrypt(struct ablkcipher_request *req)
{
    return crypto_ablkcipher_decrypt(req);
}

struct ablkcipher_tfm *lgecryptow_crypto_ablkcipher_crt(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_crt(tfm);
}

int lgecryptow_skcipher_geniv_init(struct crypto_tfm *tfm)
{
    return skcipher_geniv_init(tfm);
}

int lgecryptow_skcipher_enqueue_givcrypt(struct crypto_queue *queue, struct skcipher_givcrypt_request *request)
{
    return skcipher_enqueue_givcrypt(queue, request);
}

struct skcipher_givcrypt_request *lgecryptow_skcipher_dequeue_givcrypt(struct crypto_queue *queue)
{
    return skcipher_dequeue_givcrypt(queue);
}

void lgecryptow_skcipher_givcrypt_complete(struct skcipher_givcrypt_request *req, int err)
{
    skcipher_givcrypt_complete(req, err);
}

void lgecryptow_crypto_init_queue(struct crypto_queue *queue, unsigned int max_qlen)
{
    crypto_init_queue(queue, max_qlen);
}

void lgecryptow_skcipher_geniv_exit(struct crypto_tfm *tfm)
{
    skcipher_geniv_exit(tfm);
}

struct crypto_attr_type *lgecryptow_crypto_get_attr_type(struct rtattr **tb)
{
    return crypto_get_attr_type(tb);
}

int lgecryptow_crypto_get_default_rng(void)
{
    return crypto_get_default_rng();
}

void lgecryptow_crypto_put_default_rng(void)
{
    crypto_put_default_rng();
}

struct crypto_instance *lgecryptow_skcipher_geniv_alloc(struct crypto_template *tmpl,
                         struct rtattr **tb, u32 type, u32 mask)
{
    return skcipher_geniv_alloc(tmpl, tb, type, mask);
}

int lgecryptow_crypto_requires_sync(u32 type, u32 mask)
{
    return crypto_requires_sync(type, mask);
}

void lgecryptow_skcipher_geniv_free(struct crypto_instance *inst)
{
    return skcipher_geniv_free(inst);
}

unsigned int lgecryptow_crypto_ablkcipher_alignmask(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_alignmask(tfm);
}

void lgecryptow_scatterwalk_crypto_chain(struct scatterlist *head, struct scatterlist *sg, int chain, int num)
{
    scatterwalk_crypto_chain(head, sg, chain, num);
}

struct crypto_ablkcipher *lgecryptow_crypto_ablkcipher_cast(struct crypto_tfm *tfm)
{
    return __crypto_ablkcipher_cast(tfm);
}

void *lgecryptow_aead_givcrypt_reqctx(struct aead_givcrypt_request *req)
{
    return aead_givcrypt_reqctx(req);
}

struct crypto_aead *lgecryptow_aead_givcrypt_reqtfm(struct aead_givcrypt_request *req)
{
    return aead_givcrypt_reqtfm(req);
}

unsigned int lgecryptow_crypto_aead_ivsize(struct crypto_aead *tfm)
{
    return crypto_aead_ivsize(tfm);
}

void lgecryptow_aead_givcrypt_complete(struct aead_givcrypt_request *req, int err)
{
    aead_givcrypt_complete(req, err);
}

void *lgecryptow_crypto_aead_ctx(struct crypto_aead *tfm)
{
    return crypto_aead_ctx(tfm);
}

void lgecryptow_aead_request_set_tfm(struct aead_request *req, struct crypto_aead *tfm)
{
    aead_request_set_tfm(req, tfm);
}

struct crypto_aead *lgecryptow_aead_geniv_base(struct crypto_aead *geniv)
{
    return aead_geniv_base(geniv);
}

unsigned int lgecryptow_crypto_aead_alignmask(struct crypto_aead *tfm)
{
    return crypto_aead_alignmask(tfm);
}

void lgecryptow_aead_request_set_callback(struct aead_request *req, u32 flags, crypto_completion_t complete, void *data)
{
    aead_request_set_callback(req, flags, complete, data);
}

void lgecryptow_aead_request_set_crypt(struct aead_request *req, struct scatterlist *src, struct scatterlist *dst, unsigned int cryptlen, u8 *iv)
{
    aead_request_set_crypt(req, src, dst, cryptlen, iv);
}

void lgecryptow_aead_request_set_assoc(struct aead_request *req, struct scatterlist *assoc, unsigned int assoclen)
{
    aead_request_set_assoc(req, assoc, assoclen);
}

int lgecryptow_crypto_aead_encrypt(struct aead_request *req)
{
    return crypto_aead_encrypt(req);
}

struct aead_tfm *lgecryptow_crypto_aead_crt(struct crypto_aead *tfm)
{
    return crypto_aead_crt(tfm);
}

struct crypto_aead *lgecryptow_crypto_aead_cast(struct crypto_tfm *tfm)
{
    return __crypto_aead_cast(tfm);
}

int lgecryptow_aead_geniv_init(struct crypto_tfm *tfm)
{
    return aead_geniv_init(tfm);
}

struct crypto_instance *lgecryptow_aead_geniv_alloc(struct crypto_template *tmpl, struct rtattr **tb, u32 type, u32 mask)
{
    return aead_geniv_alloc(tmpl, tb, type, mask);
}

void lgecryptow_aead_geniv_exit(struct crypto_tfm *tfm)
{
    aead_geniv_exit(tfm);
}

void lgecryptow_aead_geniv_free(struct crypto_instance *inst)
{
    aead_geniv_free(inst);
}

struct crypto_tfm *lgecryptow_crypto_ahash_tfm(struct crypto_ahash *tfm)
{
    return crypto_ahash_tfm(tfm);
}

struct ahash_request *lgecryptow_ahash_request_alloc(struct crypto_ahash *tfm, gfp_t gfp)
{
    return ahash_request_alloc(tfm, gfp);
}

void lgecryptow_ahash_request_set_callback(struct ahash_request *req, u32 flags, crypto_completion_t complete, void *data)
{
    ahash_request_set_callback(req, flags, complete, data);
}

void lgecryptow_crypto_ahash_clear_flags(struct crypto_ahash *tfm, u32 flags)
{
    crypto_ahash_clear_flags(tfm, flags);
}

int lgecryptow_crypto_ahash_setkey(struct crypto_ahash *tfm, const u8 *key, unsigned int keylen)
{
    return crypto_ahash_setkey(tfm, key, keylen);
}

void lgecryptow_ahash_request_set_crypt(struct ahash_request *req, struct scatterlist *src, u8 *result, unsigned int nbytes)
{
    ahash_request_set_crypt(req, src, result, nbytes);
}

unsigned int lgecryptow_crypto_ahash_digestsize(struct crypto_ahash *tfm)
{
    return crypto_ahash_digestsize(tfm);
}

int lgecryptow_crypto_ahash_digest(struct ahash_request *req)
{
    return crypto_ahash_digest(req);
}

int lgecryptow_crypto_ahash_init(struct ahash_request *req)
{
    return crypto_ahash_init(req);
}

int lgecryptow_crypto_ahash_update(struct ahash_request *req)
{
    return crypto_ahash_update(req);
}

int lgecryptow_crypto_ahash_final(struct ahash_request *req)
{
    return crypto_ahash_final(req);
}

void lgecryptow_ahash_request_free(struct ahash_request *req)
{
    ahash_request_free(req);
}

struct crypto_tfm *lgecryptow_crypto_ablkcipher_tfm(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_tfm(tfm);
}

struct ablkcipher_request *lgecryptow_ablkcipher_request_alloc(struct crypto_ablkcipher *tfm, gfp_t gfp)
{
    return ablkcipher_request_alloc(tfm, gfp);
}

void lgecryptow_crypto_ablkcipher_clear_flags(struct crypto_ablkcipher *tfm, u32 flags)
{
    crypto_ablkcipher_clear_flags(tfm, flags);
}

void lgecryptow_crypto_ablkcipher_set_flags(struct crypto_ablkcipher *tfm, u32 flags)
{
    crypto_ablkcipher_set_flags(tfm, flags);
}

int lgecryptow_crypto_ablkcipher_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen)
{
    return crypto_ablkcipher_setkey(tfm, key, keylen);
}

u32 lgecryptow_crypto_ablkcipher_get_flags(struct crypto_ablkcipher *tfm)
{
    return crypto_ablkcipher_get_flags(tfm);
}

void lgecryptow_ablkcipher_request_free(struct ablkcipher_request *req)
{
    ablkcipher_request_free(req);
}

struct crypto_ablkcipher *lgecryptow_crypto_alloc_ablkcipher(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_ablkcipher(alg_name, type, mask);
}

void lgecryptow_crypto_free_ablkcipher(struct crypto_ablkcipher *tfm)
{
    crypto_free_ablkcipher(tfm);
}

struct crypto_ahash *lgecryptow_crypto_alloc_ahash(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_ahash(alg_name, type, mask);
}

void lgecryptow_crypto_free_ahash(struct crypto_ahash *tfm)
{
    crypto_free_ahash(tfm);
}

struct crypto_blkcipher *lgecryptow_crypto_alloc_blkcipher(const char *alg_name, u32 type, u32 mask)
{
    return crypto_alloc_blkcipher(alg_name, type, mask);
}

unsigned int lgecryptow_crypto_blkcipher_blocksize(struct crypto_blkcipher *tfm)
{
    return crypto_blkcipher_blocksize(tfm);
}

int lgecryptow_crypto_blkcipher_setkey(struct crypto_blkcipher *tfm, const u8 *key, unsigned int keylen)
{
    return crypto_blkcipher_setkey(tfm, key, keylen);
}

int lgecryptow_crypto_blkcipher_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes)
{
    return crypto_blkcipher_encrypt(desc, dst, src, nbytes);
}

void lgecryptow_crypto_free_blkcipher(struct crypto_blkcipher *tfm)
{
    crypto_free_blkcipher(tfm);
}

/*
 * wrapper functions
 */

static int __init _lgecrypto_module_init(void)
{
    int ret;

    ret = lgecrypto_init();
    printk("_lgecrypto_init return = %d\n", ret);
    if (ret)
        return ret;

    return ret;
}

static void __exit _lgecrypto_module_exit(void)
{
    lgecrypto_exit();
}

#else
static int __init _lgecrypto_module_init(void)
{
    return 0;
}

static void __exit _lgecrypto_module_exit(void)
{
    return;
}

#endif
module_init(_lgecrypto_module_init);
module_exit(_lgecrypto_module_exit);


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("LGE FIPS Crypto driver");
