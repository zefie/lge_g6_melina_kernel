/*
 * Integrity check code for crypto module.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */
#include <crypto/hash.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <asm-generic/sections.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/memblock.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#include "sha.h"

#include "lgecrypto_fips.h"

// the key for the HMAC-SHA-1 integrity check
#define KEY "b1d6e1acdc8425dc5160aa4f7b7b42c931ca7e"

#define LGECRYPTO_MODULE        "/sbin/lgecrypto_module.ko"
#define LGECRYPTO_HMAC_FILE        "/sbin/lgecrypto_module.hmac"

static bool need_integrity_check = true;

int fips_integrity_check(void)
{
    struct file* filp = NULL;
    mm_segment_t oldfs;
    int err = 0;
    int ret;
    int i;
    struct hash_desc desc;
    struct scatterlist sg;
    int step_len = PAGE_SIZE;
    u8 *key = KEY;
    u8 *pAllocBuf = 0;
    u8 *rbuf = 0;
    u8 hmac[SHA256_DIGEST_SIZE];
    u8 expected_buf[SHA256_DIGEST_SIZE*2+1];
    u8 expected[SHA256_DIGEST_SIZE];
    loff_t offset = 0;
    u32 file_size = 0;

    if (g_fips_state == FIPS_STATUS_FAIL) {
        printk(KERN_INFO "FIPS: skipping integrity check because of "
            "algorithm self-test errors\n");
        return -1;
    }

    if (unlikely(!need_integrity_check)) {
        printk(KERN_INFO "FIPS: skipping integrity check because it already passed\n");
        return 0;
    }

    printk(KERN_INFO "FIPS: kernel integrity check start\n");

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(LGECRYPTO_HMAC_FILE, O_RDONLY, S_IRUSR|S_IWUSR);
    if(IS_ERR(filp)) {
        printk(KERN_ERR "FIPS: %s file open error", LGECRYPTO_HMAC_FILE);
        set_klmfips_state(FIPS_STATUS_FAIL);
        return -1;
    }

    memset(expected_buf, 0x0, SHA256_DIGEST_SIZE*2+1);
    memset(expected, 0x0, SHA256_DIGEST_SIZE);
    memset(hmac, 0x0, SHA256_DIGEST_SIZE);

    ret = vfs_read(filp, expected_buf, SHA256_DIGEST_SIZE*2, &offset);

    filp_close(filp, NULL); 
    set_fs(oldfs);

    if (strlen(expected_buf) != 2 * SHA256_DIGEST_SIZE) {
        printk(KERN_ERR "FIPS: invalid integrity check HMAC parameter %s"
            " (must be %d characters long)\n", expected_buf, 2 * SHA256_DIGEST_SIZE);
        set_klmfips_state(FIPS_STATUS_FAIL);
        return -1;
    } else {
        hextobin(expected_buf, expected, SHA256_DIGEST_SIZE);
        printk(KERN_INFO "FIPS: FIPS expected integrity check HMAC = ");
        for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
            printk(KERN_CONT "%02x", expected[i]);
        }
    }

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(LGECRYPTO_MODULE, O_RDONLY, S_IRUSR|S_IWUSR);
    if(IS_ERR(filp)) {
        printk(KERN_ERR "FIPS: %s file open error", LGECRYPTO_MODULE);
        set_klmfips_state(FIPS_STATUS_FAIL);
        return -1;
    }

    file_size = vfs_llseek(filp, 0, SEEK_END);

    rbuf = vmalloc(file_size);
    if (!rbuf) {
        printk(KERN_INFO "FIPS: failed to allocate memory, length %d\n", file_size);
        set_klmfips_state(FIPS_STATUS_FAIL);
        goto err1;
    }

    vfs_llseek(filp, 0, SEEK_SET);
    offset = 0;

    ret = vfs_read(filp, rbuf, file_size, &offset);

    printk(KERN_INFO "FIPS: loadable kernel crypto integrity Image length = %d, read length = %d\n", file_size, ret);

    filp_close(filp, NULL);
    set_fs(oldfs);

    desc.tfm = crypto_alloc_hash("lge-hmac(lge-sha256)", 0, 0);

    if (IS_ERR(desc.tfm)) {
        printk(KERN_ERR "FIPS: failed to allocate tfm %ld\n",
               PTR_ERR(desc.tfm));
        set_klmfips_state(FIPS_STATUS_FAIL);
        vfree(rbuf);
        goto err1;
    }
#if FIPS_FUNC_TEST == 3
    rbuf[1024] = rbuf[1024] + 1;
#endif
    crypto_hash_setkey(desc.tfm, key, strlen(key));

    pAllocBuf = kmalloc(step_len, GFP_KERNEL);
    if (!pAllocBuf) {
        printk(KERN_INFO "FIPS: failed to allocate memory, length %d\n", step_len);
        set_klmfips_state(FIPS_STATUS_FAIL);
        vfree(rbuf);
        goto err1;
    }

    err = crypto_hash_init(&desc);
    if (err) {
        printk(KERN_INFO "FIPS: failed at crypto_hash_init\n");
        set_klmfips_state(FIPS_STATUS_FAIL);
        kfree(pAllocBuf);
        vfree(rbuf);
        goto err1;
    }

    for (i = 0; i < file_size - CONFIG_LGECRYPTO_FIPS_ADDED_SIGNATURE_SIZE; i += step_len) {
        if (i + step_len >= file_size - CONFIG_LGECRYPTO_FIPS_ADDED_SIGNATURE_SIZE - 1)  {
            memcpy(pAllocBuf, &rbuf[i], file_size - CONFIG_LGECRYPTO_FIPS_ADDED_SIGNATURE_SIZE - i);
            sg_init_one(&sg, pAllocBuf, file_size - CONFIG_LGECRYPTO_FIPS_ADDED_SIGNATURE_SIZE - i);
            err = crypto_hash_update(&desc, &sg, file_size - CONFIG_LGECRYPTO_FIPS_ADDED_SIGNATURE_SIZE - i);
            if (err) {
                printk(KERN_INFO "FIPS: failed at crypto_hash_update (1)\n");
                set_klmfips_state(FIPS_STATUS_FAIL);
                goto err;
            }
            err = crypto_hash_final(&desc, hmac);
            if (err) {
                printk(KERN_INFO "FIPS: failed at crypto_hash_final\n");
                set_klmfips_state(FIPS_STATUS_FAIL);
                goto err;
            }
        } else {
            memcpy(pAllocBuf, &rbuf[i], step_len);
            sg_init_one(&sg, pAllocBuf, step_len);
            err = crypto_hash_update(&desc, &sg, step_len);

            if (err) {
                printk(KERN_INFO "FIPS: failed at crypto_hash_update (2)\n");
                set_klmfips_state(FIPS_STATUS_FAIL);
                goto err;
            }
        }
    }

#if FIPS_FUNC_TEST == 3
    rbuf[1024] = rbuf[1024] - 1;
#endif

    printk(KERN_INFO "FIPS: result of hmac_sha256 is ");
    for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
        printk(KERN_CONT "%02x", hmac[i]);
    }
    printk(KERN_CONT "\n");

    if (!strncmp(hmac, expected, SHA256_DIGEST_SIZE)) {
        printk(KERN_INFO "FIPS: kernel integrity check passed\n");
        set_klmfips_state(FIPS_STATUS_CRYPTO_ALLOWED);
        need_integrity_check = false;
        return 0;
    } else {
        printk(KERN_ERR "FIPS: kernel integrity check failed, expected value was ");
        for (i = 0; i < SHA256_DIGEST_SIZE; i++) {
            printk(KERN_CONT "%02x", expected[i]);
        }
        printk(KERN_CONT "\n");
        set_klmfips_state(FIPS_STATUS_FAIL);
    }

 err:
    kfree(pAllocBuf);
    vfree(rbuf);
    crypto_free_hash(desc.tfm);
 err1:
    need_integrity_check = false;
    panic("FIPS: ERROR! could not start FIPS mode - Integrity check failed\n");
    return -1;
}

EXPORT_SYMBOL_GPL(fips_integrity_check);
