---
c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: aws-sigv4-mode
Protocols: HTTP
Arg: <mode>
Help: AWS V4 signature mode
Category: auth http
Added: 8.17.0
Multi: single
See-also:
  - aws-sigv4
  - user
Example:
  - --aws-sigv4 "aws:amz:us-east-2:es" --aws-sigv4-mode querystring --user "key:secret" $URL
  - --aws-sigv4 "aws:amz:us-east-2:es" --aws-sigv4-mode querystring --user "key:secret:token" $URL
---



## --aws-sigv4-mode

Specify the AWS SigV4 signing mode. Valid values are "header" (default) and "querystring".

When set to "header" (which is the default), AWS SigV4 authentication parameters are added as HTTP headers instead of query string parameters. This
is the standard method for AWS API requests.

When set to "querystring", AWS SigV4 authentication parameters are added to the URL query string instead of HTTP headers. This is useful for creating pre-signed URLs that
can be shared or used without additional authentication headers.

Example:
curl --aws-sigv4 "aws:s3:us-east-1:s3" --aws-sigv4-mode "querystring" \
     --user "keyId:secretKey" \
     "https://example-bucket.s3.amazonaws.com/test.txt"


## --aws-sigv4-expires

Set the expiration time in seconds for AWS SigV4 querystring signatures. 
Only valid when used with --aws-sigv4-mode "querystring". The value must be a positive integer representing the number of seconds the signature remains valid.

Example:
curl --aws-sigv4 "aws:s3:us-east-1:s3" --aws-sigv4-mode "querystring" \
     --aws-sigv4-expires 3600 \
     --user "keyId:secretKey" \
     "https://example-bucket.s3.amazonaws.com/test.txt"

This adds an X-Amz-Expires parameter to the signed URL, creating a time-limited pre-signed URL that expires after the specified duration.
