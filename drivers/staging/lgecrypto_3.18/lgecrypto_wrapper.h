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
#ifndef __LGE_FIPSCRYPTO_WRAPPER_HEADER__
#define __LGE_FIPSCRYPTO_WRAPPER_HEADER__

#include <crypto/b128ops.h>
#include <crypto/gf128mul.h>
#include <crypto/algapi.h>
#include <crypto/hash.h>
#include <crypto/hash.h>
#include <crypto/rng.h>
#include <crypto/scatterwalk.h>
#include <crypto/crypto_wq.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>
#include <crypto/internal/aead.h>
#include <crypto/internal/skcipher.h>
#include <linux/cryptohash.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kmod.h>
#include <linux/notifier.h>
#include <linux/crypto.h>
#include <linux/fips_status.h>
#include <linux/kernel.h>

int lgecryptow_crypto_register_alg(struct crypto_alg *alg);
int lgecryptow_crypto_unregister_alg(struct crypto_alg *alg);
int lgecryptow_crypto_register_algs(struct crypto_alg *alg, int count);
int lgecryptow_crypto_unregister_algs(struct crypto_alg *alg, int count);
int lgecryptow_crypto_register_shash(struct shash_alg *alg);
int lgecryptow_crypto_unregister_shash(struct shash_alg *alg);
int lgecryptow_crypto_register_shashes(struct shash_alg *alg, int count);
int lgecryptow_crypto_unregister_shashes(struct shash_alg *alg, int count);
int lgecryptow_crypto_register_template(struct crypto_template *tmpl);
void lgecryptow_crypto_unregister_template(struct crypto_template *tmpl);
void *lgecryptow_crypto_tfm_ctx(struct crypto_tfm *tfm);
unsigned int lgecryptow_crypto_tfm_ctx_alignment(void);
void lgecryptow_crypto_cipher_clear_flags(struct crypto_cipher *tfm, u32 flags);
void lgecryptow_crypto_cipher_set_flags(struct crypto_cipher *tfm, u32 flags);
u32 lgecryptow_crypto_tfm_get_flags(struct crypto_tfm *tfm);
int lgecryptow_crypto_cipher_setkey(struct crypto_cipher *tfm, const u8 *key, unsigned int keylen);
void lgecryptow_crypto_tfm_set_flags(struct crypto_tfm *tfm, u32 flags);
u32 lgecryptow_crypto_cipher_get_flags(struct crypto_cipher *tfm);
int lgecryptow_crypto_cipher_blocksize(struct crypto_cipher *tfm);
struct crypto_tfm *lgecryptow_crypto_cipher_tfm(struct crypto_cipher *tfm);
struct cipher_alg *lgecryptow_crypto_cipher_alg(struct crypto_cipher *tfm);
void *lgecryptow_crypto_blkcipher_ctx(struct crypto_blkcipher *tfm);
void *lgecryptow_crypto_instance_ctx(struct crypto_instance *inst);
struct crypto_cipher *lgecryptow_crypto_spawn_cipher(struct crypto_spawn *spawn);
void lgecryptow_crypto_free_cipher(struct crypto_cipher *tfm);
int lgecryptow_crypto_check_attr_type(struct rtattr **tb, u32 type);
struct crypto_alg *lgecryptow_crypto_get_attr_alg(struct rtattr **tb, u32 type, u32 mask);
void *lgecryptow_crypto_alloc_instance(const char *name, struct crypto_alg *alg);
void lgecryptow_crypto_mod_put(struct crypto_alg *alg);
void lgecryptow_crypto_drop_spawn(struct crypto_spawn *spawn);
void lgecryptow_crypto_xor(u8 *dst, const u8 *src, unsigned int size);
unsigned int lgecryptow_crypto_cipher_alignmask(struct crypto_cipher *tfm);
void lgecryptow_crypto_cipher_encrypt_one(struct crypto_cipher *tfm, u8 *dst, const u8 *src);
void lgecryptow_crypto_cipher_decrypt_one(struct crypto_cipher *tfm, u8 *dst, const u8 *src);
void lgecryptow_crypto_inc(u8 *a, unsigned int size);
struct crypto_alg *lgecryptow_crypto_attr_alg(struct rtattr *rta, u32 type, u32 mask);
void lgecryptow_be128_xor(be128 *r, const be128 *p, const be128 *q);
void lgecryptow_gf128mul_x_ble(be128 *r, const be128 *x);
void *lgecryptow_shash_desc_ctx(struct shash_desc *desc);
void lgecryptow_sha_transform(__u32 *digest, const char *data, __u32 *W);
int lgecryptow_blkcipher_walk_virt(struct blkcipher_desc *desc, struct blkcipher_walk *walk);
int lgecryptow_blkcipher_walk_done(struct blkcipher_desc *desc, struct blkcipher_walk *walk, int err);
void lgecryptow_blkcipher_walk_init(struct blkcipher_walk *walk, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes);
int lgecryptow_blkcipher_walk_virt_block(struct blkcipher_desc *desc, struct blkcipher_walk *walk, unsigned int blocksize);
unsigned int lgecryptow_crypto_shash_statesize(struct crypto_shash *tfm);
void *lgecryptow_crypto_shash_ctx_aligned(struct crypto_shash *tfm);
unsigned int lgecryptow_crypto_shash_blocksize(struct crypto_shash *tfm);
unsigned int lgecryptow_crypto_shash_digestsize(struct crypto_shash *tfm);
unsigned int lgecryptow_crypto_shash_descsize(struct crypto_shash *tfm);
u32 lgecryptow_crypto_shash_get_flags(struct crypto_shash *tfm);
int lgecryptow_crypto_shash_digest(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out);
int lgecryptow_crypto_shash_init(struct shash_desc *desc);
int lgecryptow_crypto_shash_update(struct shash_desc *desc, const u8 *data, unsigned int len);
int lgecryptow_crypto_shash_export(struct shash_desc *desc, void *out);
int lgecryptow_crypto_shash_import(struct shash_desc *desc, const void *in);
int lgecryptow_crypto_shash_final(struct shash_desc *desc, u8 *out);
int lgecryptow_crypto_shash_finup(struct shash_desc *desc, const u8 *data, unsigned int len, u8 *out);
struct crypto_shash *lgecryptow_crypto_shash_cast(struct crypto_tfm *tfm);
struct crypto_shash *lgecryptow_crypto_spawn_shash(struct crypto_shash_spawn *spawn);
void lgecryptow_crypto_free_shash(struct crypto_shash *tfm);
struct shash_alg *lgecryptow_shash_attr_alg(struct rtattr *rta, u32 type, u32 mask);
struct shash_instance *lgecryptow_shash_alloc_instance(const char *name, struct crypto_alg *alg);
int lgecryptow_crypto_init_shash_spawn(struct crypto_shash_spawn *spawn, struct shash_alg *alg, struct crypto_instance *inst);
struct crypto_instance *lgecryptow_shash_crypto_instance(struct shash_instance *inst);
void *lgecryptow_shash_instance_ctx(struct shash_instance *inst);
int lgecryptow_shash_register_instance(struct crypto_template *tmpl, struct shash_instance *inst);
void lgecryptow_shash_free_instance(struct crypto_instance *inst);
void lgecryptow_get_random_bytes(void *buf, int nbytes);
int lgecryptow_crypto_rng_get_bytes(struct crypto_rng *tfm, u8 *rdata, unsigned int dlen);
struct crypto_rng *lgecryptow_crypto_alloc_rng(const char *alg_name, u32 type, u32 mask);
void lgecryptow_crypto_free_rng(struct crypto_rng *tfm);
void *lgecryptow_crypto_rng_ctx(struct crypto_rng *tfm);
struct crypto_shash *lgecryptow_crypto_alloc_shash(const char *alg_name, u32 type, u32 mask);
int lgecryptow_crypto_shash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen);
struct crypto_cipher *lgecryptow_crypto_alloc_cipher(const char *alg_name, u32 type, u32 mask);
struct crypto_tfm *lgecryptow_crypto_rng_tfm(struct crypto_rng *tfm);
const char *lgecryptow_crypto_tfm_alg_driver_name(struct crypto_tfm *tfm);
int lgecryptow_crypto_register_rngs(struct rng_alg *algs, int count);
void lgecryptow_crypto_unregister_rngs(struct rng_alg *algs, int count);
struct crypto_ablkcipher *lgecryptow_skcipher_givcrypt_reqtfm(struct skcipher_givcrypt_request *req);
void *lgecryptow_crypto_ablkcipher_ctx(struct crypto_ablkcipher *tfm);
void *lgecryptow_skcipher_givcrypt_reqctx(struct skcipher_givcrypt_request *req);
void lgecryptow_ablkcipher_request_set_tfm(struct ablkcipher_request *req, struct crypto_ablkcipher *tfm);
struct crypto_ablkcipher *lgecryptow_skcipher_geniv_cipher(struct crypto_ablkcipher *geniv);
void lgecryptow_ablkcipher_request_set_callback(struct ablkcipher_request *req, u32 flags, crypto_completion_t complete, void *data);
void lgecryptow_ablkcipher_request_set_crypt(struct ablkcipher_request *req, struct scatterlist *src, struct scatterlist *dst, unsigned int nbytes, void *iv);
unsigned int lgecryptow_crypto_ablkcipher_ivsize(struct crypto_ablkcipher *tfm);
int lgecryptow_crypto_ablkcipher_encrypt(struct ablkcipher_request *req);
int lgecryptow_crypto_ablkcipher_decrypt(struct ablkcipher_request *req);
struct ablkcipher_tfm *lgecryptow_crypto_ablkcipher_crt(struct crypto_ablkcipher *tfm);
int lgecryptow_skcipher_geniv_init(struct crypto_tfm *tfm);
int lgecryptow_skcipher_enqueue_givcrypt(struct crypto_queue *queue, struct skcipher_givcrypt_request *request);
struct skcipher_givcrypt_request *lgecryptow_skcipher_dequeue_givcrypt(struct crypto_queue *queue);
void lgecryptow_skcipher_givcrypt_complete(struct skcipher_givcrypt_request *req, int err);
void lgecryptow_crypto_init_queue(struct crypto_queue *queue, unsigned int max_qlen);
void lgecryptow_skcipher_geniv_exit(struct crypto_tfm *tfm);
struct crypto_attr_type *lgecryptow_crypto_get_attr_type(struct rtattr **tb);
int lgecryptow_crypto_get_default_rng(void);
void lgecryptow_crypto_put_default_rng(void);
struct crypto_instance *lgecryptow_skcipher_geniv_alloc(struct crypto_template *tmpl, struct rtattr **tb, u32 type, u32 mask);
int lgecryptow_crypto_requires_sync(u32 type, u32 mask);
void lgecryptow_skcipher_geniv_free(struct crypto_instance *inst);
unsigned int lgecryptow_crypto_ablkcipher_alignmask(struct crypto_ablkcipher *tfm);
void lgecryptow_scatterwalk_crypto_chain(struct scatterlist *head, struct scatterlist *sg, int chain, int num);
struct crypto_ablkcipher *lgecryptow_crypto_ablkcipher_cast(struct crypto_tfm *tfm);
void *lgecryptow_aead_givcrypt_reqctx(struct aead_givcrypt_request *req);
struct crypto_aead *lgecryptow_aead_givcrypt_reqtfm(struct aead_givcrypt_request *req);
unsigned int lgecryptow_crypto_aead_ivsize(struct crypto_aead *tfm);
void lgecryptow_aead_givcrypt_complete(struct aead_givcrypt_request *req, int err);
void *lgecryptow_crypto_aead_ctx(struct crypto_aead *tfm);
void lgecryptow_aead_request_set_tfm(struct aead_request *req, struct crypto_aead *tfm);
struct crypto_aead *lgecryptow_aead_geniv_base(struct crypto_aead *geniv);
unsigned int lgecryptow_crypto_aead_alignmask(struct crypto_aead *tfm);
void lgecryptow_aead_request_set_callback(struct aead_request *req, u32 flags, crypto_completion_t complete, void *data);
void lgecryptow_aead_request_set_crypt(struct aead_request *req, struct scatterlist *src, struct scatterlist *dst, unsigned int cryptlen, u8 *iv);
void lgecryptow_aead_request_set_assoc(struct aead_request *req, struct scatterlist *assoc, unsigned int assoclen);
int lgecryptow_crypto_aead_encrypt(struct aead_request *req);
struct aead_tfm *lgecryptow_crypto_aead_crt(struct crypto_aead *tfm);
struct crypto_aead *lgecryptow_crypto_aead_cast(struct crypto_tfm *tfm);
int lgecryptow_aead_geniv_init(struct crypto_tfm *tfm);
struct crypto_instance *lgecryptow_aead_geniv_alloc(struct crypto_template *tmpl, struct rtattr **tb, u32 type, u32 mask);
void lgecryptow_aead_geniv_exit(struct crypto_tfm *tfm);
void lgecryptow_aead_geniv_free(struct crypto_instance *inst);
struct crypto_tfm *lgecryptow_crypto_ahash_tfm(struct crypto_ahash *tfm);
struct ahash_request *lgecryptow_ahash_request_alloc(struct crypto_ahash *tfm, gfp_t gfp);
void lgecryptow_ahash_request_set_callback(struct ahash_request *req, u32 flags, crypto_completion_t complete, void *data);
void lgecryptow_crypto_ahash_clear_flags(struct crypto_ahash *tfm, u32 flags);
int lgecryptow_crypto_ahash_setkey(struct crypto_ahash *tfm, const u8 *key, unsigned int keylen);
void lgecryptow_ahash_request_set_crypt(struct ahash_request *req, struct scatterlist *src, u8 *result, unsigned int nbytes);
unsigned int lgecryptow_crypto_ahash_digestsize(struct crypto_ahash *tfm);
int lgecryptow_crypto_ahash_digest(struct ahash_request *req);
int lgecryptow_crypto_ahash_init(struct ahash_request *req);
int lgecryptow_crypto_ahash_update(struct ahash_request *req);
int lgecryptow_crypto_ahash_final(struct ahash_request *req);
void lgecryptow_ahash_request_free(struct ahash_request *req);
struct crypto_tfm *lgecryptow_crypto_ablkcipher_tfm(struct crypto_ablkcipher *tfm);
struct ablkcipher_request *lgecryptow_ablkcipher_request_alloc(struct crypto_ablkcipher *tfm, gfp_t gfp);
void lgecryptow_crypto_ablkcipher_clear_flags(struct crypto_ablkcipher *tfm, u32 flags);
void lgecryptow_crypto_ablkcipher_set_flags(struct crypto_ablkcipher *tfm, u32 flags);
int lgecryptow_crypto_ablkcipher_setkey(struct crypto_ablkcipher *tfm, const u8 *key, unsigned int keylen);
u32 lgecryptow_crypto_ablkcipher_get_flags(struct crypto_ablkcipher *tfm);
void lgecryptow_ablkcipher_request_free(struct ablkcipher_request *req);
struct crypto_ablkcipher *lgecryptow_crypto_alloc_ablkcipher(const char *alg_name, u32 type, u32 mask);
void lgecryptow_crypto_free_ablkcipher(struct crypto_ablkcipher *tfm);
struct crypto_ahash *lgecryptow_crypto_alloc_ahash(const char *alg_name, u32 type, u32 mask);
void lgecryptow_crypto_free_ahash(struct crypto_ahash *tfm);
struct crypto_blkcipher *lgecryptow_crypto_alloc_blkcipher(const char *alg_name, u32 type, u32 mask);
unsigned int lgecryptow_crypto_blkcipher_blocksize(struct crypto_blkcipher *tfm);
int lgecryptow_crypto_blkcipher_setkey(struct crypto_blkcipher *tfm, const u8 *key, unsigned int keylen);
int lgecryptow_crypto_blkcipher_encrypt(struct blkcipher_desc *desc, struct scatterlist *dst, struct scatterlist *src, unsigned int nbytes);
void lgecryptow_crypto_free_blkcipher(struct crypto_blkcipher *tfm);

extern int  lgecrypto_init(void);
extern void lgecrypto_exit(void);

extern int des_generic_mod_init(void);
extern void des_generic_mod_fini(void);

extern int aes_generic_mod_init(void);
extern void aes_generic_mod_exit(void);

extern int crypto_ecb_module_init(void);
extern void crypto_ecb_module_exit(void);

extern int crypto_cbc_module_init(void);
extern void crypto_cbc_module_exit(void);

extern int crypto_ctr_module_init(void);
extern void crypto_ctr_module_exit(void);

extern int chainiv_module_init(void);
extern void chainiv_module_exit(void);

extern int eseqiv_module_init(void);
extern void eseqiv_module_exit(void);

extern int seqiv_module_init(void);
extern void seqiv_module_exit(void);

extern int crypto_module_init(void);
extern void crypto_module_exit(void);

extern int sha1_generic_mod_init(void);
extern void sha1_generic_mod_fini(void);

extern int sha256_generic_mod_init(void);
extern void sha256_generic_mod_fini(void);

extern int sha512_generic_mod_init(void);
extern void sha512_generic_mod_fini(void);

extern int hmac_module_init(void);
extern void hmac_module_exit(void);

extern int drbg_init(void);
extern void drbg_exit(void);

//ARMv8 Instruction
#ifdef ARM64_PLATFORM
extern int aes_armv8_core_64_init(void);
extern void aes_armv8_core_64_exit(void);

extern int aes_armv8_ce_64_init(void);
extern void aes_armv8_ce_64_exit(void);

extern int sha1_armv8_ce_64_init(void);
extern void sha1_armv8_ce_64_exit(void);

extern int sha2_armv8_ce_64_init(void);
extern void sha2_armv8_ce_64_exit(void);
#else
extern int aes_armv8_neon_32_init(void);
extern void aes_armv8_neon_32_exit(void);

extern int aes_armv8_ce_32_init(void);
extern void aes_armv8_ce_32_exit(void);

extern int sha1_armv8_neon_32_init(void);
extern void sha1_armv8_neon_32_exit(void);

extern int sha2_armv8_ce_32_init(void);
extern void sha2_armv8_ce_32_exit(void);

extern int sha256_armv8_neon_32_init(void);
extern void sha256_armv8_neon_32_exit(void);

extern int sha512_armv8_neon_32_init(void);
extern void sha512_armv8_neon_32_exit(void);
#endif

extern int tcrypt_mod_init(void);
extern void tcrypt_mod_fini(void);

#endif // __LGE_FIPSCRYPTO_WRAPPER_HEADER__
