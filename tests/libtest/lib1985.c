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
#include "first.h"

#include "memdebug.h"

static CURLcode test_lib1985(const char *URL)
{
  CURL *curl;
  CURLcode res = CURLE_OK;

  if(curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
    return TEST_ERR_MAJOR_BAD;
  }

  curl = curl_easy_init();
  if(!curl) {
    curl_global_cleanup();
    return TEST_ERR_MAJOR_BAD;
  }

  test_setopt(curl, CURLOPT_WRITEDATA, stdout);
  test_setopt(curl, CURLOPT_URL, URL);
  test_setopt(curl, CURLOPT_AWS_SIGV4, "aws:amz:us-east-1:s3");
  test_setopt(curl, CURLOPT_USERPWD,
    "AKIAIOSFODNN7EXAMPLE:wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY:"
    "IQoJb3JpZ2luX2VjEXAMPLEXVzLWVhc3QtMSJIMEYCIQCGC7ruw358GnqRtZR8JekvTo9l"
    "GSPG+9RE5oGx+GbBBwIhAOV3H1KMtusW4Ad6gg2E9coOdc4V7kh6YTCNzvTdDt38KrYDCN"
    "////////wEQARoMMzUyMDI2NTY0NDk3Igywkvm9IrZnJE5G06QqigMmv4WZjZh+Xv4ftN"
    "2oPlAPv78OWoX1y/LZa+mXqqtciE00hDL5oHgg4jweYkZV8QIEFrhhbvvi8+lLZtOsxXt5"
    "C1YRpY9KJHLNcpnq7OfUJFwwZd6b42KQO+nE3SZj+MMge7cJHSpXWWlSwjnbVqH12flLZr"
    "CTNq97sMeRvBCJkPHlG06e79NpY+I/5YeckXWxAOaOZDJZM3cDDEPAyxDmJbI5vFU60W5N"
    "MSQ9h1WBJPgpBJprMRvcN5O+gLve5+Di1HYOeW8Y7scOZSJ0Wa1cg4BzOSz+Pbw1g+durr"
    "ZQZ5IrYhkLsrkSNTI/qrNCaetShI8OyhD9CjNyrED2dS2qrdx4I1qDuEZRHghWMbqZKopK"
    "9mMnZx8FEw1QY5ruxAAnqvQ80AIkmmN97UgO/HbvDR3BeGZ/s+bD8kgyPdwm5E3iHvJ/aS"
    "gJVVRstydgwlmPLwnodp7WUbW8NWJKP2PfQYK5pfQlUPBdxlOA6I+DK2wVgx0ECV7pvvTw"
    "BvB0ZcwStFjJQ9BJUeG2MN+41ccGOpwBLVWtPobQ8DqObRK4Epg2PM1MPF5h84jQSaAiBo"
    "PrGJDZXOTd7NQRIhyqb5smydZKSgqv8Z/RBOM7BMONDYgzOqgyAOYHSXHXhcOhfrBH4z/4"
    "g0QQ3v0/nlEAQrSbjzc5tOEEBLRy6bLSik46hlJC0gCvv3YATHTIebQXcSFOscMGcp96Lq"
    "mjmDAUJej9asbyGePIERZbR4+KyyqJ");

  res = curl_easy_perform(curl);

test_cleanup:
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return res;
}
