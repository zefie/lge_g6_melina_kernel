/*
 * Cryptographic API.
 *
 * Copyright (c) 2002 James Morris <jmorris@intercode.com.au>
 * Copyright (c) 2005 Herbert Xu <herbert@gondor.apana.org.au>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */
#ifndef _LGECRYPTO_FIPS_H
#define _LGECRYPTO_FIPS_H

#include <linux/completion.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/rwsem.h>
#include <linux/slab.h>
#include <linux/fips.h>
#include <linux/types.h>
#include "../lgecrypto_wrapper.h"

/**
* fips_status: global FIPS140-2 status
* @FIPS_STATUS_NA:
* @FIPS_STATUS_PASS_CRYPTO: KAT self tests are passed.
* @FIPS_STATUS_CRYPTO_ALLOWED: Integrity test is passed.
* @FIPS_STATUS_PASS: All tests are passed
* @FIPS_STATUS_FAIL: One of the test is failed.
*/
enum fips_state {
    FIPS_STATUS_NA            = 0x0,
    FIPS_STATUS_PASS_CRYPTO    = 0x1,
    FIPS_STATUS_CRYPTO_ALLOWED    = 0x2,
    FIPS_STATUS_PASS        = 0x3,
    FIPS_STATUS_FAIL        = 0xFF
};


/*
 * FIPS functional test flags, for use in functional testing.
 *
 * FIPS_FUNC_TEST 1 will make all AES self-tests fail
 * FIPS_FUNC_TEST 2 will make the hmac(sha256) algorithm self-test fail
 * FIPS_FUNC_TEST 3 will make the integrity check fail by corrupting the
 * kernel image in memory
 * FIPS_FUNC_TEST 4 will log the necessary information for the zeroization test
 * consecutive generated blocks of bits to be identical
 *
 * All other values will have the same effect as the default value 0, which is
 * that FIPS self and integrity tests will proceed as normal and log all output.
 */

#ifdef CONFIG_CRYPTO_FIPS_TEST
#define FIPS_FUNC_TEST 0
#else
#define FIPS_FUNC_TEST 0
#endif

extern enum fips_state g_fips_state;

void set_klmfips_state(enum fips_state state);
enum fips_state get_klmfips_state(void);
int fips_integrity_check(void);
int alg_selftest(const char *driver, const char *alg, u32 type, u32 mask);
void hextobin(const char *str, u8 *bytes, size_t blen);
#endif    /* _LGECRYPTO_FIPS_H */

