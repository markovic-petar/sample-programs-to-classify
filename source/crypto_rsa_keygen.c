/*-
 * Copyright 2022-2025 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "helper_io.h"

#include <string.h>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/core_names.h>
#include <openssl/pem.h>

static const char *propq = NULL;

static EVP_PKEY *generate_rsa_key_short(OSSL_LIB_CTX *libctx, unsigned int bits)
{
    EVP_PKEY *pkey = NULL;

    // fprintf(stdout, "Generating RSA key, this may take some time...\n");
    pkey = EVP_PKEY_Q_keygen(libctx, propq, "RSA", (size_t)bits);

    // if (pkey == NULL)
    //     fprintf(stderr, "EVP_PKEY_Q_keygen() failed\n");

    return pkey;
}

/*
 * Prints information on an EVP_PKEY object representing an RSA key pair.
 */
static int dump_key(const EVP_PKEY *pkey)
{
    int ret = 0;
    int bits = 0;
    BIGNUM *n = NULL, *e = NULL, *d = NULL, *p = NULL, *q = NULL;

    /*
     * Retrieve value of n. This value is not secret and forms part of the
     * public key.
     *
     * Calling EVP_PKEY_get_bn_param with a NULL BIGNUM pointer causes
     * a new BIGNUM to be allocated, so these must be freed subsequently.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_N, &n) == 0) {
        fprintf(stderr, "Failed to retrieve n\n");
        goto cleanup;
    }

    /*
     * Retrieve value of e. This value is not secret and forms part of the
     * public key. It is typically 65537 and need not be changed.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_E, &e) == 0) {
        fprintf(stderr, "Failed to retrieve e\n");
        goto cleanup;
    }

    /*
     * Retrieve value of d. This value is secret and forms part of the private
     * key. It must not be published.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_D, &d) == 0) {
        fprintf(stderr, "Failed to retrieve d\n");
        goto cleanup;
    }

    /*
     * Retrieve value of the first prime factor, commonly known as p. This value
     * is secret and forms part of the private key. It must not be published.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR1, &p) == 0) {
        fprintf(stderr, "Failed to retrieve p\n");
        goto cleanup;
    }

    /*
     * Retrieve value of the second prime factor, commonly known as q. This value
     * is secret and forms part of the private key. It must not be published.
     *
     * If you are creating an RSA key with more than two primes for special
     * applications, you can retrieve these primes with
     * OSSL_PKEY_PARAM_RSA_FACTOR3, etc.
     */
    if (EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_RSA_FACTOR2, &q) == 0) {
        fprintf(stderr, "Failed to retrieve q\n");
        goto cleanup;
    }

    /*
     * We can also retrieve the key size in bits for informational purposes.
     */
    if (EVP_PKEY_get_int_param(pkey, OSSL_PKEY_PARAM_BITS, &bits) == 0) {
        fprintf(stderr, "Failed to retrieve bits\n");
        goto cleanup;
    }

    /* Output hexadecimal representations of the BIGNUM objects. */
    fprintf(stdout, "\nNumber of bits: %d\n\n", bits);
    fprintf(stdout, "Public values:\n");
    fprintf(stdout, "  n = 0x");
    BN_print_fp(stdout, n);
    fprintf(stdout, "\n");

    fprintf(stdout, "  e = 0x");
    BN_print_fp(stdout, e);
    fprintf(stdout, "\n\n");

    fprintf(stdout, "Private values:\n");
    fprintf(stdout, "  d = 0x");
    BN_print_fp(stdout, d);
    fprintf(stdout, "\n");

    fprintf(stdout, "  p = 0x");
    BN_print_fp(stdout, p);
    fprintf(stdout, "\n");

    fprintf(stdout, "  q = 0x");
    BN_print_fp(stdout, q);
    fprintf(stdout, "\n\n");

    /* Output a PEM encoding of the public key. */
    if (PEM_write_PUBKEY(stdout, pkey) == 0) {
        fprintf(stderr, "Failed to output PEM-encoded public key\n");
        goto cleanup;
    }

    /*
     * Output a PEM encoding of the private key. Please note that this output is
     * not encrypted. You may wish to use the arguments to specify encryption of
     * the key if you are storing it on disk. See PEM_write_PrivateKey(3).
     */
    if (PEM_write_PrivateKey(stdout, pkey, NULL, NULL, 0, NULL, NULL) == 0) {
        fprintf(stderr, "Failed to output PEM-encoded private key\n");
        goto cleanup;
    }

    ret = 1;
cleanup:
    BN_free(n); /* not secret */
    BN_free(e); /* not secret */
    BN_clear_free(d); /* secret - scrub before freeing */
    BN_clear_free(p); /* secret - scrub before freeing */
    BN_clear_free(q); /* secret - scrub before freeing */
    return ret;
}

int main(int argc, char** argv)
{
    int ret = EXIT_FAILURE;

    OSSL_LIB_CTX *libctx = NULL;
    EVP_PKEY *pkey = NULL;
    unsigned int bits = 2048; // 512, 1024, 2048, 4096

    /* Get program repeat duration */
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        fprintf(stderr, "Input error. Correct usage: %s [seconds]\n", argv[0]);
        goto cleanup;
    }

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration)
    {
        /* Generate key */
        pkey = generate_rsa_key_short(libctx, bits);
        if (pkey == NULL)
        {
            fprintf(stderr, "EVP_PKEY_Q_keygen() failed\n");
            goto cleanup;
        }
    }
    
    /* Dump the integers comprising the key. */
    if (dump_key(pkey) == 0) {
        fprintf(stderr, "Failed to dump key\n");
        goto cleanup;
    }

    ret = EXIT_SUCCESS;

cleanup:
    EVP_PKEY_free(pkey);
    OSSL_LIB_CTX_free(libctx);

    return ret;
}