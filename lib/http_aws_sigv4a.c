/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * SPDX-License-Identifier: curl
 *
 ***************************************************************************/

#include "curl_setup.h"

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_AWS)

#include "urldata.h"
#include "http_aws_sigv4a.h"
#include "curl_sha256.h"
#include "curl_hmac.h"
#include "strdup.h"
#include "escape.h"

#ifdef USE_OPENSSL
#include <openssl/opensslv.h>
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/ecdsa.h>
#endif
#endif

/* The last 2 #include files should be in this order */
#include "curl_memory.h"
#include "memdebug.h"

#if defined(USE_OPENSSL) && OPENSSL_VERSION_NUMBER >= 0x10100000L

/*
 * Derive ECDSA P-256 private key from SigV4A signing key
 * This implements the deterministic key derivation for AWS SigV4A
 */
static CURLcode derive_ecdsa_key(const unsigned char *signing_key,
                                size_t key_len,
                                EC_KEY **ec_key_out)
{
  EC_KEY *ec_key = NULL;
  BIGNUM *priv_bn = NULL;
  BIGNUM *order = NULL;
  const EC_GROUP *group;
  unsigned char hash[32];
  int counter = 0;
  CURLcode result = CURLE_OUT_OF_MEMORY;

  /* Create secp256r1 curve */
  ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  if(!ec_key)
    return CURLE_OUT_OF_MEMORY;

  group = EC_KEY_get0_group(ec_key);
  if(!group) {
    EC_KEY_free(ec_key);
    return CURLE_SSL_CIPHER;
  }

  order = BN_new();
  if(!order) {
    EC_KEY_free(ec_key);
    return CURLE_OUT_OF_MEMORY;
  }

  if(!EC_GROUP_get_order(group, order, NULL)) {
    BN_free(order);
    EC_KEY_free(ec_key);
    return CURLE_SSL_CIPHER;
  }

  /* Deterministic key generation using counter method */
  do {
    unsigned char seed[36]; /* 32 bytes key + 4 bytes counter */

    /* Prepare seed: signing_key || counter */
    memcpy(seed, signing_key, key_len < 32 ? key_len : 32);
    if(key_len < 32) {
      memset(seed + key_len, 0, 32 - key_len);
    }
    seed[32] = (counter >> 24) & 0xFF;
    seed[33] = (counter >> 16) & 0xFF;
    seed[34] = (counter >> 8) & 0xFF;
    seed[35] = counter & 0xFF;

    /* Hash the seed */
    if(Curl_sha256it(hash, seed, sizeof(seed))) {
      result = CURLE_SSL_CIPHER;
      goto cleanup;
    }

    /* Convert hash to BIGNUM */
    BN_free(priv_bn);
    priv_bn = BN_bin2bn(hash, sizeof(hash), NULL);
    if(!priv_bn) {
      result = CURLE_OUT_OF_MEMORY;
      goto cleanup;
    }

    counter++;

    /* Ensure private key is in valid range [1, order-1] */
  } while(BN_is_zero(priv_bn) || BN_cmp(priv_bn, order) >= 0);

  /* Set the private key */
  if(!EC_KEY_set_private_key(ec_key, priv_bn)) {
    result = CURLE_SSL_CIPHER;
    goto cleanup;
  }

  /* Generate corresponding public key */
  if(!EC_KEY_generate_key(ec_key)) {
    result = CURLE_SSL_CIPHER;
    goto cleanup;
  }

  *ec_key_out = ec_key;
  result = CURLE_OK;

cleanup:
  BN_free(priv_bn);
  BN_free(order);
  if(result != CURLE_OK && ec_key) {
    EC_KEY_free(ec_key);
  }

  return result;
}

/*
 * Sign data using ECDSA P-256 and return hex-encoded signature
 */
static CURLcode ecdsa_sign_data(EC_KEY *ec_key,
                               const unsigned char *data,
                               size_t data_len,
                               char *signature_hex)
{
  unsigned char hash[32];
  unsigned char *sig_der = NULL;
  unsigned int sig_len = 0;
  CURLcode result = CURLE_SSL_CIPHER;

  /* Hash the data */
  if(Curl_sha256it(hash, data, data_len)) {
    return CURLE_SSL_CIPHER;
  }

  /* Sign the hash */
  sig_len = ECDSA_size(ec_key);
  sig_der = malloc(sig_len);
  if(!sig_der) {
    return CURLE_OUT_OF_MEMORY;
  }

  if(!ECDSA_sign(0, hash, sizeof(hash), sig_der, &sig_len, ec_key)) {
    free(sig_der);
    return CURLE_SSL_CIPHER;
  }

  /* Convert DER signature to hex */
  if(sig_len * 2 + 1 <= 129) { /* Max signature length check */
    Curl_hexencode(sig_der, sig_len, (unsigned char *)signature_hex,
                   sig_len * 2 + 1);
    result = CURLE_OK;
  }

  free(sig_der);
  return result;
}

/*
 * Derive ECDSA P-256 key from signing key and sign data
 */
static CURLcode sigv4a_sign_ecdsa(const unsigned char *signing_key,
                                  size_t key_len,
                                  const unsigned char *data,
                                  size_t data_len,
                                  char *signature_hex)
{
  EC_KEY *ec_key = NULL;
  BIGNUM *priv_key_bn = NULL;
  CURLcode result = CURLE_SSL_CIPHER;

  /* Create EC_KEY for P-256 curve */
  ec_key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
  if(!ec_key) {
    return CURLE_SSL_CIPHER;
  }

  /* Convert signing key to BIGNUM for private key */
  priv_key_bn = BN_bin2bn(signing_key, (int)key_len, NULL);
  if(!priv_key_bn) {
    EC_KEY_free(ec_key);
    return CURLE_SSL_CIPHER;
  }

  /* Set private key */
  if(!EC_KEY_set_private_key(ec_key, priv_key_bn)) {
    BN_free(priv_key_bn);
    EC_KEY_free(ec_key);
    return CURLE_SSL_CIPHER;
  }

  /* Sign the data */
  result = ecdsa_sign_data(ec_key, data, data_len, signature_hex);

  BN_free(priv_key_bn);
  EC_KEY_free(ec_key);
  return result;
}

#endif /* USE_OPENSSL */

/*
 * Generate SigV4A signature using ECDSA P-256
 * This is the main entry point for SigV4A signature generation
 */
CURLcode Curl_aws_sigv4a_sign(struct Curl_easy *data,
                              const char *secret,
                              const char *date,
                              const char *region_set,
                              const char *service,
                              const char *request_type,
                              const char *str_to_sign,
                              char *signature_hex)
{
#if defined(USE_OPENSSL) && OPENSSL_VERSION_NUMBER >= 0x10100000L
  unsigned char sign0[32], sign1[32];
  unsigned char signing_key[32];
  EC_KEY *ec_key = NULL;
  CURLcode result;

  (void)data;
  (void)region_set; /* Used for X-Amz-Region-Set header, signing uses "*" */
  (void)ec_key;

  /* Derive signing key using SigV4A method (region = "*") */
  result = Curl_hmacit(&Curl_HMAC_SHA256,
                      (const unsigned char *)secret, strlen(secret),
                      (const unsigned char *)date, strlen(date), sign0);
  if(result)
    return result;

  /* Use "*" for region in SigV4A */
  result = Curl_hmacit(&Curl_HMAC_SHA256, sign0, sizeof(sign0),
                      (const unsigned char *)"*", 1, sign1);
  if(result)
    return result;

  result = Curl_hmacit(&Curl_HMAC_SHA256, sign1, sizeof(sign1),
                      (const unsigned char *)service, strlen(service), sign0);
  if(result)
    return result;

  result = Curl_hmacit(&Curl_HMAC_SHA256, sign0, sizeof(sign0),
                      (const unsigned char *)request_type,
                      strlen(request_type), signing_key);
  if(result)
    return result;

  /* Generate ECDSA P-256 signature */
  result = sigv4a_sign_ecdsa(signing_key, sizeof(signing_key),
                            (const unsigned char *)str_to_sign,
                            strlen(str_to_sign), signature_hex);

  return result;

#else
  (void)data;
  (void)secret;
  (void)date;
  (void)region_set;
  (void)service;
  (void)request_type;
  (void)str_to_sign;
  (void)signature_hex;

  return CURLE_NOT_BUILT_IN;
#endif
}

#endif /* !CURL_DISABLE_HTTP && !CURL_DISABLE_AWS */
