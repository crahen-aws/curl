#ifndef HEADER_CURL_HTTP_AWS_SIGV4A_H
#define HEADER_CURL_HTTP_AWS_SIGV4A_H
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

/*
 * Generate SigV4A signature using ECDSA P-256
 * 
 * Parameters:
 *   data: curl handle
 *   secret: AWS secret access key
 *   date: date string (YYYYMMDD format)

 *   region_set: "*", single region, or comma-delimited list for SigV4A
 *   request_type: "aws4_request"
 *   str_to_sign: the canonical string to sign
 *   signature_hex: output buffer for hex-encoded signature (must be at least 129 bytes)
 *
 * Returns CURLE_OK on success, error code on failure
 */
CURLcode Curl_aws_sigv4a_sign(struct Curl_easy *data,
                              const char *secret,
                              const char *date,
                              const char *region_set,
                              const char *service,
                              const char *request_type,
                              const char *str_to_sign,
                              char *signature_hex);

#endif /* !CURL_DISABLE_HTTP && !CURL_DISABLE_AWS */

#endif /* HEADER_CURL_HTTP_AWS_SIGV4A_H */
