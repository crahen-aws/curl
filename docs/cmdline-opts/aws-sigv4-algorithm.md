---
c: Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
SPDX-License-Identifier: curl
Long: aws-sigv4-algorithm
Protocols: HTTP
Arg: <algorithm>
Help: AWS V4 signature algorithm
Category: auth http
Added: 8.18.0
Multi: single
See-also:
  - aws-sigv4
  - aws-sigv4-mode
  - user
Example:
  - --aws-sigv4 "aws:amz:us-east-2:es" --aws-sigv4-algorithm "HMAC-SHA256" --user "key:secret" $URL
  - --aws-sigv4 "aws:amz:us-east-2:es" --aws-sigv4-algorithm "ECDSA-P256-SHA256" --user "key:secret" $URL
---

# `--aws-sigv4-algorithm`

Specify the cryptographic algorithm to use for AWS SigV4 signature calculation.

Valid values are:
- **HMAC-SHA256** (default) - Uses HMAC with SHA-256 for signature calculation
- **ECDSA-P256-SHA256** - Uses ECDSA with P-256 curve and SHA-256 for signature calculation

The HMAC-SHA256 algorithm is the standard method used by most AWS services and is the default if this option is not specified.

The ECDSA-P256-SHA256 algorithm provides an alternative signing method that may be required by specific AWS services or configurations.
