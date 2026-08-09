/*-
 * Copyright 2021-2025 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "rsa_encrypt.h"

#include "helper_io.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/decoder.h>
#include <openssl/core_names.h>


/* Input data to encrypt */
static const unsigned char msg[] =
    "To be, or not to be, that is the question,\n"
    "Whether tis nobler in the minde to suffer\n"
    "The slings and arrowes of outragious fortune,\n"
    "Or to take Armes again in a sea of troubles";

static EVP_PKEY *get_key(OSSL_LIB_CTX *libctx, const char *propq, int public)
{
    OSSL_DECODER_CTX *dctx = NULL;
    EVP_PKEY *pkey = NULL;
    int selection;
    const unsigned char *data;
    size_t data_len;

    if (public) {
        selection = EVP_PKEY_PUBLIC_KEY;
        data = pub_key_der;
        data_len = sizeof(pub_key_der);
    } else {
        selection = EVP_PKEY_KEYPAIR;
        data = priv_key_der;
        data_len = sizeof(priv_key_der);
    }
    dctx = OSSL_DECODER_CTX_new_for_pkey(&pkey, "DER", NULL, "RSA",
                                         selection, libctx, propq);
    (void)OSSL_DECODER_from_data(dctx, &data, &data_len);
    OSSL_DECODER_CTX_free(dctx);
    return pkey;
}

/* Set optional parameters for RSA OAEP Padding */
static void set_optional_params(OSSL_PARAM *p, const char *propq)
{
    static unsigned char label[] = "label";

    /* "pkcs1" is used by default if the padding mode is not set */
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_PAD_MODE,
                                            OSSL_PKEY_RSA_PAD_MODE_OAEP, 0);
    /* No oaep_label is used if this is not set */
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_ASYM_CIPHER_PARAM_OAEP_LABEL,
                                             label, sizeof(label));
    /* "SHA1" is used if this is not set */
    *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST,
                                            "SHA256", 0);
    /*
     * If a non default property query needs to be specified when fetching the
     * OAEP digest then it needs to be specified here.
     */
    if (propq != NULL)
        *p++ = OSSL_PARAM_construct_utf8_string(OSSL_ASYM_CIPHER_PARAM_OAEP_DIGEST_PROPS,
                                                (char *)propq, 0);

    /*
     * OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST and
     * OSSL_ASYM_CIPHER_PARAM_MGF1_DIGEST_PROPS can also be optionally added
     * here if the MGF1 digest differs from the OAEP digest.
     */

    *p = OSSL_PARAM_construct_end();
}

int main(int argc, char** argv)
{
    int ret = EXIT_FAILURE;
    const char file_name[] = "encrypted_rsa.txt";
    FILE* output = NULL;

    /* Buffers */
    // unsigned char *plain = NULL;
    // size_t plain_len = 0;

    /* Encryption context */
    size_t msg_len = sizeof(msg) - 1;
    unsigned char *encrypted = NULL;
    size_t encrypted_len = 0;
    OSSL_LIB_CTX *libctx = NULL;

    /* Public key context */
    int public = 1;
    const char *propq = NULL;
    EVP_PKEY_CTX *pkey_ctx = NULL;
    EVP_PKEY *pub_key = NULL;
    OSSL_PARAM params[5];

    /* Load plaintext */
    // if (io_fread_to_char_buf("plaintext.txt", &plain, &plain_len)) {
    //     fprintf(stderr, "Fread failed.\n");
    //     goto cleanup;
    // }

    /* Get public key */
    pub_key = get_key(libctx, propq, public);
    if (pub_key == NULL) {
        fprintf(stderr, PROGRAM_NAME ": " "Get public key failed.\n");
        goto cleanup;
    }
    pkey_ctx = EVP_PKEY_CTX_new_from_pkey(libctx, pub_key, propq);
    if (pkey_ctx == NULL) {
        fprintf(stderr, PROGRAM_NAME ": " "EVP_PKEY_CTX_new_from_pkey() failed.\n");
        goto cleanup;
    }
    set_optional_params(params, propq);
    /* If no optional parameters are required then NULL can be passed */
    if (EVP_PKEY_encrypt_init_ex(pkey_ctx, params) <= 0) {
        fprintf(stderr, PROGRAM_NAME ": " "EVP_PKEY_encrypt_init_ex() failed.\n");
        goto cleanup;
    }
    /* Calculate the size required to hold the encrypted data */
    if (EVP_PKEY_encrypt(pkey_ctx, NULL, &encrypted_len, msg, msg_len) <= 0) {
        fprintf(stderr, PROGRAM_NAME ": " "EVP_PKEY_encrypt() failed.\n");
        goto cleanup;
    }
    /* Allocate the buffer for encrypted data */
    encrypted = OPENSSL_zalloc(encrypted_len);
    if (encrypted  == NULL) {
        fprintf(stderr, PROGRAM_NAME ": " "Malloc failed.\n");
        goto cleanup;
    }

    /* Get program repeat duration */
    time_t duration;
    if (io_time_from_args(argc, argv, &duration, 1) != IO_OK)
    {
        fprintf(stderr, PROGRAM_NAME ": " "Input error. Correct usage: %s [seconds]\n", argv[0]);
        goto cleanup;
    }

    time_t start_time = time(NULL);
    size_t tmp_encrypted_len = 0;
    while (time(NULL) - start_time < duration)
    {
        /* !!! Perform encryption !!! */
        tmp_encrypted_len = encrypted_len;
        if (EVP_PKEY_encrypt(pkey_ctx, encrypted, &tmp_encrypted_len, msg, msg_len) <= 0) {
            fprintf(stderr, PROGRAM_NAME ": " "EVP_PKEY_encrypt() failed.\n");
            goto cleanup;
        }
    }

    /* Write to file */
    encrypted_len = tmp_encrypted_len;
    if (io_fwrite_from_char_buf(file_name, encrypted, encrypted_len))
    {
        fprintf(stderr, PROGRAM_NAME ": " "Fwrite failed.\n");
        goto cleanup;
    }

    ret = EXIT_SUCCESS;

cleanup:
    EVP_PKEY_free(pub_key);
    EVP_PKEY_CTX_free(pkey_ctx);
    OPENSSL_free(encrypted);
    OSSL_LIB_CTX_free(libctx);
    if (ret != EXIT_SUCCESS)
        ERR_print_errors_fp(stderr);
    return ret;
}
