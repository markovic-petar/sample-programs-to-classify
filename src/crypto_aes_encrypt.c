/*-
* Copyright 2021-2025 The OpenSSL Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License").  You may not use
* this file except in compliance with the License.  You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://www.openssl.org/source/license.html
*/
#include "aesgmc.h"

#include "helper_io.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>

#define B

int main(int argc, char** argv)
{
    int ret = EXIT_FAILURE;

    /*
     * A library context and property query can be used to select & filter
     * algorithm implementations. If they are NULL then the default library
     * context and properties are used.
     */
    static OSSL_LIB_CTX *libctx = NULL;
    static const char *propq = NULL;
    EVP_CIPHER_CTX *ctx;
    EVP_CIPHER *cipher = NULL;
    int outlen, tmplen;  // ??
    size_t gcm_ivlen = sizeof(gcm_iv);
    unsigned char outtag[16];
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };

    /* Buffers */
    unsigned char *plain = NULL;
    size_t plain_len = 0;
    unsigned char *encrypted = NULL;
    size_t encrypted_len = 0;
    int inl, outl;

    /* Read plaintext */
    if (io_fread_to_char_buf("plaintext.txt", &plain, &plain_len))
        goto err;
    
    /* Convert from size_t to int for EncryptUpdate */
    if (plain_len > INT_MAX)
        goto err;
    inl = (int) plain_len;
    
    /* Allocate output buffer */
    encrypted_len = plain_len; // + EVP_CIPHER_CTX_block_size(ctx);
    encrypted = malloc(encrypted_len);

    /* Create a context for the encrypt operation */
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
        goto err;

    /* Fetch the cipher implementation */
    if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL)
        goto err;

    /* Set IV length if default 96 bits is not appropriate */
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
                                            &gcm_ivlen);

    /* Get program repeat duration */
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        fprintf(stderr, "Invalid time argument.\n");
        goto err;
    }

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < duration)
    {
        /*
         * Initialise an encrypt operation with the cipher/mode, key, IV and
         * IV length parameter.
         * For demonstration purposes the IV is being set here. In a compliant
         * application the IV would be generated internally so the iv passed in
         * would be NULL.
         */
        if (!EVP_EncryptInit_ex2(ctx, cipher, gcm_key, gcm_iv, params))
            goto err;

        /* !!! Encrypt plaintext !!! */
        if (!EVP_EncryptUpdate(ctx, encrypted, &outl, plain, inl))
            goto err;

        /* Do not finalize - no need for authentication here... */
    }

    /* Convert from int to size_t for fwrite */
    if (outl < 0)
        goto err;
    encrypted_len = (size_t) outl;

    /* Output encrypted txt to file */
    if (io_fwrite_from_char_buf("encrypted_aes.txt", encrypted, encrypted_len))
        goto err;

    ret = EXIT_SUCCESS;

err:
    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);
    if (plain) free(plain);
    if (encrypted) free(encrypted);

    return ret;
}