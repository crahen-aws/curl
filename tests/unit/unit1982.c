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
#include "curlcheck.h"

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if !defined(CURL_DISABLE_HTTP) && !defined(CURL_DISABLE_AWS) && \
    defined(USE_OPENSSL)

static CURLcode unit_setup(void)
{
  return CURLE_OK;
}

static void unit_stop(void)
{
}

static CURLcode test_unit1982(const char *arg)
{
  CURLcode result;

  UNITTEST_BEGIN_SIMPLE
  /* Test NULL parameters - these should all fail */
  result = Curl_aws_sigv4a_sign(NULL, NULL, "20101109", "*",
                               "service", "aws4_request",
                               NULL, NULL, NULL, NULL);
  fail_unless(result != CURLE_OK, "NULL key should fail");

  result = Curl_aws_sigv4a_sign(NULL, "", "20101109", "*",
                               "service", "aws4_request",
                               NULL, NULL, NULL, NULL);
  fail_unless(result != CURLE_OK, "Empty key should fail");

  result = Curl_aws_sigv4a_sign(NULL, "AWS4testkey", "20101109", "*",
                               "service", "aws4_request",
                               NULL, NULL, NULL, NULL);
  fail_unless(result != CURLE_OK, "NULL data should fail");
  UNITTEST_END_SIMPLE

#else

static CURLcode test_unit1982(const char *arg)
{
  UNITTEST_BEGIN_SIMPLE
  /* AWS SigV4A not supported */
  UNITTEST_END_SIMPLE
}

#endif
