/*
 * Copyright 2012-2024 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * Simple AES GCM authenticated encryption with additional data (AEAD)
 * demonstration program.
 */

#include "aesgmc.h"

#include <stdio.h>

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>

/*
 * A library context and property query can be used to select & filter
 * algorithm implementations. If they are NULL then the default library
 * context and properties are used.
 */
static OSSL_LIB_CTX *libctx = NULL;
static const char *propq = NULL;

static int aes_gcm_encrypt(void)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx;
    EVP_CIPHER *cipher = NULL;
    int outlen, tmplen;
    size_t gcm_ivlen = sizeof(gcm_iv);
    unsigned char outbuf[1024];
    unsigned char outtag[16];
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };

    printf("AES GCM Encrypt:\n");
    printf("Plaintext:\n");
    BIO_dump_fp(stdout, gcm_pt, sizeof(gcm_pt));

    /* Create a context for the encrypt operation */
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
        goto err;

    /* Fetch the cipher implementation */
    if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL)
        goto err;

    /* Set IV length if default 96 bits is not appropriate */
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
                                            &gcm_ivlen);

    /*
     * Initialise an encrypt operation with the cipher/mode, key, IV and
     * IV length parameter.
     * For demonstration purposes the IV is being set here. In a compliant
     * application the IV would be generated internally so the iv passed in
     * would be NULL.
     */
    if (!EVP_EncryptInit_ex2(ctx, cipher, gcm_key, gcm_iv, params))
        goto err;

    /* Zero or more calls to specify any AAD */
    if (!EVP_EncryptUpdate(ctx, NULL, &outlen, gcm_aad, sizeof(gcm_aad)))
        goto err;

    /* Encrypt plaintext */
    if (!EVP_EncryptUpdate(ctx, outbuf, &outlen, gcm_pt, sizeof(gcm_pt)))
        goto err;

    /* Output encrypted block */
    printf("Ciphertext:\n");
    BIO_dump_fp(stdout, outbuf, outlen);

    /* Finalise: note get no output for GCM */
    if (!EVP_EncryptFinal_ex(ctx, outbuf, &tmplen))
        goto err;

    /* Get tag */
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG,
                                                  outtag, 16);

    if (!EVP_CIPHER_CTX_get_params(ctx, params))
        goto err;

    /* Output tag */
    printf("Tag:\n");
    BIO_dump_fp(stdout, outtag, 16);

    ret = 1;
err:
    if (!ret)
        ERR_print_errors_fp(stderr);

    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);

    return ret;
}

static int aes_gcm_decrypt(void)
{
    int ret = 0;
    EVP_CIPHER_CTX *ctx;
    EVP_CIPHER *cipher = NULL;
    int outlen, rv;
    size_t gcm_ivlen = sizeof(gcm_iv);
    unsigned char outbuf[1024];
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };

    printf("AES GCM Decrypt:\n");
    printf("Ciphertext:\n");
    BIO_dump_fp(stdout, gcm_ct, sizeof(gcm_ct));

    if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
        goto err;

    /* Fetch the cipher implementation */
    if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL)
        goto err;

    /* Set IV length if default 96 bits is not appropriate */
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
                                            &gcm_ivlen);

    /*
     * Initialise an encrypt operation with the cipher/mode, key, IV and
     * IV length parameter.
     */
    if (!EVP_DecryptInit_ex2(ctx, cipher, gcm_key, gcm_iv, params))
        goto err;

    /* Zero or more calls to specify any AAD */
    if (!EVP_DecryptUpdate(ctx, NULL, &outlen, gcm_aad, sizeof(gcm_aad)))
        goto err;

    /* Decrypt plaintext */
    if (!EVP_DecryptUpdate(ctx, outbuf, &outlen, gcm_ct, sizeof(gcm_ct)))
        goto err;

    /* Output decrypted block */
    printf("Plaintext:\n");
    BIO_dump_fp(stdout, outbuf, outlen);

    /* Set expected tag value. */
    params[0] = OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG,
                                                  (void*)gcm_tag, sizeof(gcm_tag));

    if (!EVP_CIPHER_CTX_set_params(ctx, params))
        goto err;

    /* Finalise: note get no output for GCM */
    rv = EVP_DecryptFinal_ex(ctx, outbuf, &outlen);
    /*
     * Print out return value. If this is not successful authentication
     * failed and plaintext is not trustworthy.
     */
    printf("Tag Verify %s\n", rv > 0 ? "Successful!" : "Failed!");

    ret = 1;
err:
    if (!ret)
        ERR_print_errors_fp(stderr);

    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);

    return ret;
}

int main(int argc, char **argv)
{
    if (!aes_gcm_encrypt())
        return EXIT_FAILURE;

    if (!aes_gcm_decrypt())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}