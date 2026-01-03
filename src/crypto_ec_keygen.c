#include "helper_io.h"

#include <string.h>
#include <stdio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/core_names.h>
#include <openssl/pem.h>

static const char *propq = NULL;

static EVP_PKEY *generate_ec_key_short(OSSL_LIB_CTX *libctx, const char* type)
{
    EVP_PKEY *pkey = NULL;

    // fprintf(stdout, "Generating RSA key, this may take some time...\n");
    pkey = EVP_PKEY_Q_keygen(libctx, propq, type, NULL);

    // if (pkey == NULL)
    //     fprintf(stderr, "EVP_PKEY_Q_keygen() failed\n");

    return pkey;
}

/*
 * Prints information on an EVP_PKEY object representing an EC key pair.
 */
static int dump_key(const EVP_PKEY *pkey)
{
    int ret = 0;

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
    return ret;
}

int main(int argc, char** argv)
{
    int ret = EXIT_FAILURE;

    OSSL_LIB_CTX *libctx = NULL;
    EVP_PKEY *pkey = NULL;
    // ED25519, ED448, SM2, X25519, X448, ML-DSA-44, ML-DSA-65, ML-DSA-87, ML-KEM-512, ML-KEM-768, or ML-KEM-1024
    const unsigned char curvename[] = "ED25519";

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
        pkey = generate_ec_key_short(libctx, curvename);
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