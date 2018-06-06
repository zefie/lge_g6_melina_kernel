#include <asm/neon.h>
#ifdef ARM64_PLATFORM
#include <linux/types.h>
#include <asm/cpu.h>
#include <asm/hwcap.h>
#include <linux/cpufeature.h>
#endif
#include "lgecrypto_fips.h"

static inline int tcrypt_test(const char *alg, u32 type, u32 mask)
{
    int ret;

    ret = alg_selftest(alg, alg, type, mask);

    return ret;
}

static int do_test(int m)
{
    int ret = 0;

    switch (m) {
    case 1:
        ret += tcrypt_test("lge-ecb(lge-des3_ede)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 2:
        ret += tcrypt_test("lge-cbc(lge-des3_ede)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 3:
        ret += tcrypt_test("lge-ctr(lge-des3_ede)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 4:
        ret += tcrypt_test("lge-ecb(lge-aes)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 5:
        ret += tcrypt_test("lge-cbc(lge-aes)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 6:
        ret += tcrypt_test("lge-xts(lge-aes)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 7:
        ret += tcrypt_test("lge-ctr(lge-aes)", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 10:
        ret += tcrypt_test("lge-sha1-generic", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 11:
        ret += tcrypt_test("lge-sha224-generic", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 12:
        ret += tcrypt_test("lge-sha256-generic", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 13:
        ret += tcrypt_test("lge-sha384-generic", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 14:
        ret += tcrypt_test("lge-sha512-generic", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

#ifdef ARM64_PLATFORM
    case 15:
        if(cpu_have_feature(cpu_feature(SHA1)))
            ret += tcrypt_test("lge-sha1", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 16:
        if(cpu_have_feature(cpu_feature(SHA2)))
            ret += tcrypt_test("lge-sha224", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 17:
        if(cpu_have_feature(cpu_feature(SHA2)))
            ret += tcrypt_test("lge-sha256", CRYPTO_ALG_TYPE_SHASH, 0);
        break;
#else
    case 15:
        if (cpu_has_neon())
            ret += tcrypt_test("lge-sha1-neon", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 16:
        if (elf_hwcap2 & HWCAP2_SHA2 || cpu_has_neon())
            ret += tcrypt_test("lge-sha224", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 17:
        if (elf_hwcap2 & HWCAP2_SHA2 || cpu_has_neon())
            ret += tcrypt_test("lge-sha256", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 18:
        if (cpu_has_neon())
            ret += tcrypt_test("lge-sha384", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 19:
        if (cpu_has_neon())
            ret += tcrypt_test("lge-sha512", CRYPTO_ALG_TYPE_SHASH, 0);
        break;
#endif

    case 20:
        ret += tcrypt_test("lge-hmac(lge-sha1)", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 21:
        ret += tcrypt_test("lge-hmac(lge-sha224)", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 22:
        ret += tcrypt_test("lge-hmac(lge-sha256)", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 23:
        ret += tcrypt_test("lge-hmac(lge-sha384)", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 24:
        ret += tcrypt_test("lge-hmac(lge-sha512)", CRYPTO_ALG_TYPE_SHASH, 0);
        break;

    case 30:
        ret += tcrypt_test("drbg_nopr_ctr_aes128", CRYPTO_ALG_TYPE_RNG, 0);
        break;

    case 31:
        ret += tcrypt_test("drbg_nopr_ctr_aes192", CRYPTO_ALG_TYPE_RNG, 0);
        break;

    case 32:
        ret += tcrypt_test("drbg_nopr_ctr_aes256", CRYPTO_ALG_TYPE_RNG, 0);
        break;

    case 33:
        ret += tcrypt_test("drbg_nopr_hmac_sha256", CRYPTO_ALG_TYPE_RNG, 0);
        /* covered by drbg_nopr_hmac_sha256 test */
        //ret += tcrypt_test("drbg_nopr_hmac_sha1", 0);
        //ret += tcrypt_test("drbg_nopr_hmac_sha384", 0);
        //ret += tcrypt_test("drbg_nopr_hmac_sha512", 0);
        break;

    case 34:
        ret += tcrypt_test("drbg_nopr_sha256", CRYPTO_ALG_TYPE_RNG, 0);
        /* covered by drbg_nopr_sha256 test */
        //ret += tcrypt_test("drbg_nopr_sha1", 0);
        //ret += tcrypt_test("drbg_nopr_sha384", 0);
        //ret += tcrypt_test("drbg_nopr_sha512", 0);
        break;

    case 35:
        ret += tcrypt_test("drbg_pr_ctr_aes128", CRYPTO_ALG_TYPE_RNG, 0);
        /* covered by drbg_pr_ctr_aes128 test */
        //ret += tcrypt_test("drbg_pr_ctr_aes192", 0);
        //ret += tcrypt_test("drbg_pr_ctr_aes256", 0);
        break;

    case 36:
        ret += tcrypt_test("drbg_pr_hmac_sha256", CRYPTO_ALG_TYPE_RNG, 0);
        /* covered by drbg_pr_hmac_sha256 test */
        //ret += tcrypt_test("drbg_pr_hmac_sha1", 0);
        //ret += tcrypt_test("drbg_pr_hmac_sha384", 0);
        //ret += tcrypt_test("drbg_pr_hmac_sha512", 0);
        break;

    case 37:
        ret += tcrypt_test("drbg_pr_sha256", CRYPTO_ALG_TYPE_RNG, 0);
        /* covered by drbg_pr_sha256 test */
        //ret += tcrypt_test("drbg_pr_sha1", 0);
        //ret += tcrypt_test("drbg_pr_sha384", 0);
        //ret += tcrypt_test("drbg_pr_sha512", 0);
        break;

#ifdef ARM64_PLATFORM
    case 40:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("lge-cbc(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 41:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("lge-ctr(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 42:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("lge-ecb(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 43:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("lge-xts(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 44:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("__lge-cbc-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 45:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("__lge-ctr-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 46:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("__lge-ecb-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 47:
        if(cpu_have_feature(cpu_feature(AES)))
            ret += tcrypt_test("__lge-xts-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;
#else
    case 40:
        if (elf_hwcap2 & HWCAP2_AES || cpu_has_neon())
            ret += tcrypt_test("lge-cbc(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 41:
        if (elf_hwcap2 & HWCAP2_AES || cpu_has_neon())
            ret += tcrypt_test("lge-ctr(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 42:
        if (elf_hwcap2 & HWCAP2_AES)
            ret += tcrypt_test("lge-ecb(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 43:
        if (elf_hwcap2 & HWCAP2_AES || cpu_has_neon())
            ret += tcrypt_test("lge-xts(aes)", CRYPTO_ALG_TYPE_ABLKCIPHER, CRYPTO_ALG_ASYNC);
        break;

    case 44:
        if (elf_hwcap2 & HWCAP2_AES)
            ret += tcrypt_test("__lge-cbc-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 45:
        if (elf_hwcap2 & HWCAP2_AES)
            ret += tcrypt_test("__lge-ctr-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 46:
        if (elf_hwcap2 & HWCAP2_AES)
            ret += tcrypt_test("__lge-ecb-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 47:
        if (elf_hwcap2 & HWCAP2_AES)
            ret += tcrypt_test("__lge-xts-aes-ce", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 53:
        if (cpu_has_neon())
            ret += tcrypt_test("__lge-cbc-aes-neonbs", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 54:
        if (cpu_has_neon())
            ret += tcrypt_test("__lge-ctr-aes-neonbs", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;

    case 55:
        if (cpu_has_neon())
            ret += tcrypt_test("__lge-xts-aes-neonbs", CRYPTO_ALG_TYPE_BLKCIPHER, 0);
        break;
#endif
    }

    return ret;
}

static int run_selftest(void)
{
    int i;
    int ret = 0;

    for (i = 1; i < 100; i++) {
        ret += do_test(i);

        if (ret) {
            printk(KERN_ERR "FIPS: FIPS140-2 Known Answer Tests : Failed\n");
            break;
        }
    }

    if (ret) {
        set_klmfips_state(FIPS_STATUS_FAIL);
        panic("FIPS: ERROR! could not start FIPS mode - Known Answer Tests failed\n");
    } else {
        printk(KERN_INFO "FIPS: FIPS140-2 Known Answer Tests: Successful!\n");

        if (g_fips_state != FIPS_STATUS_PASS)
            set_klmfips_state(FIPS_STATUS_PASS_CRYPTO);
    }

    return ret;
}

int __init lgecrypto_init(void)
{
    int ret;

    des_generic_mod_init();

#ifdef ARM64_PLATFORM
    aes_armv8_core_64_init();
    aes_armv8_ce_64_init();

    sha1_armv8_ce_64_init();
    sha2_armv8_ce_64_init();
#else
    aes_armv8_neon_32_init();
    aes_armv8_ce_32_init();

    sha1_armv8_neon_32_init();
    sha2_armv8_ce_32_init();
    sha256_armv8_neon_32_init();
    sha512_armv8_neon_32_init();
#endif

    aes_generic_mod_init();
    chainiv_module_init();
    eseqiv_module_init();
    seqiv_module_init();
    crypto_ecb_module_init();
    crypto_cbc_module_init();
    crypto_ctr_module_init();
    crypto_module_init();

    sha1_generic_mod_init();
    sha256_generic_mod_init();
    sha512_generic_mod_init();

    hmac_module_init();

    drbg_init();

    printk(KERN_INFO "FIPS: running power-on self-tests!\n");

    ret = run_selftest();

    if (ret)
        goto err;
#if 0
    ret = fips_integrity_check();

    if (ret)
        goto err;
#endif  // only needed for CMVP certification
    printk(KERN_INFO "FIPS: power-on self-tests passes!\n");
    set_klmfips_state(FIPS_STATUS_PASS);

    return 0;

err:
    return -1;
}

void __exit lgecrypto_exit(void)
{
    printk(KERN_INFO "lgecrypto was unregistered\n");

    des_generic_mod_fini();

#ifdef ARM64_PLATFORM
    aes_armv8_core_64_exit();
    aes_armv8_ce_64_exit();

    sha1_armv8_ce_64_exit();
    sha2_armv8_ce_64_exit();
#else
    aes_armv8_neon_32_exit();
    aes_armv8_ce_32_exit();

    sha1_armv8_neon_32_exit();
    sha2_armv8_ce_32_exit();
    sha256_armv8_neon_32_exit();
    sha512_armv8_neon_32_exit();
#endif

    aes_generic_mod_exit();
    crypto_ecb_module_exit();
    crypto_cbc_module_exit();
    crypto_ctr_module_exit();
    chainiv_module_exit();
    eseqiv_module_exit();
    seqiv_module_exit();
    crypto_module_exit();

    sha1_generic_mod_fini();
    sha256_generic_mod_fini();
    sha512_generic_mod_fini();

    hmac_module_exit();

    drbg_exit();
}

