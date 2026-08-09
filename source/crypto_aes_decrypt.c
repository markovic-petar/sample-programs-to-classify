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

int main(int argc, char** argv) {
    int ret = EXIT_FAILURE;

    /*
     * A library context and property query can be used to select & filter
     * algorithm implementations. If they are NULL then the default library
     * context and properties are used.
     */
    static OSSL_LIB_CTX *libctx = NULL;
    static const char *propq = NULL;
    EVP_CIPHER_CTX *ctx = NULL;
    EVP_CIPHER *cipher = NULL;
    size_t gcm_ivlen = sizeof(gcm_iv);
    OSSL_PARAM params[2] = {
        OSSL_PARAM_END, OSSL_PARAM_END
    };
    size_t tmplen = 0;

    /* Buffers */
    unsigned char *encrypted = NULL;
    size_t encrypted_len = 0;
    unsigned char *decrypted = NULL;
    size_t decrypted_len = 0;
    int inl, outl;

    /* Read input */
    if (io_fread_to_char_buf("encrypted_aes.txt", &encrypted, &encrypted_len)) {
        fprintf(stderr, "Fread failed.\n");
        goto err;
    }

    /* Convert from size_t to int for DecryptUpdate */
    if (encrypted_len > INT_MAX)
        goto err;
    inl = (int) encrypted_len;

    /* Allocate output */
    decrypted_len = encrypted_len;
    decrypted = malloc(decrypted_len);

    /* Create a context for the decrypt operation */
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL) {
        fprintf(stderr, "EVP cipher context creation failed.\n");
        goto err;
    }

    /* Fetch the cipher implementation */
    if ((cipher = EVP_CIPHER_fetch(libctx, "AES-256-GCM", propq)) == NULL) {
        fprintf(stderr, "EVP cipher implementation fetch failed.\n");
        goto err;
    }

    /* Set IV length if default 96 bits is not appropriate */
    params[0] = OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN,
                                            &gcm_ivlen);

    /* Get program repeat duration */
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK) {
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
        if (!EVP_DecryptInit_ex2(ctx, cipher, gcm_key, gcm_iv, params)) {
            fprintf(stderr, "EVP decrypt init. failed.\n");
            goto err;
        }

        /* !!! Decrypt plaintext !!! */
        if (!EVP_DecryptUpdate(ctx, decrypted, &outl, encrypted, inl)) {
            fprintf(stderr, "EVP decrypt init. failed.\n");
            goto err;
        }
    }

    /* Do not finalize - Encryption does not produce a tag for this test... */

    /* Convert from int to size_t for fwrite */
    if (outl < 0)
        goto err;
    decrypted_len = (size_t) outl;

    /* Output to file */
    if (io_fwrite_from_char_buf("decrypted_aes.txt", decrypted, decrypted_len)) {
        goto err;
    }

    ret = EXIT_SUCCESS;

err:
    EVP_CIPHER_free(cipher);
    EVP_CIPHER_CTX_free(ctx);
    if (encrypted) free(encrypted);
    if (decrypted) free(decrypted);


    return ret;
}