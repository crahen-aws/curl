#!/bin/bash

echo "Testing AWS SigV4 querystring mode..."

# Test with existing query parameters
echo "=== Test 1: With existing query parameters ==="
./src/curl -v --aws-sigv4 "aws:s3:us-east-1:s3" --aws-sigv4-mode "querystring" --user "AKIAIOSFODNN7EXAMPLE:wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY" "http://example-bucket.s3.amazonaws.com/test.txt?existing=param&another=value" 2>&1 | grep -E "(Canonical request|GET /test.txt)" | head -10

echo ""
echo "=== Test 2: Without existing query parameters ==="
./src/curl -v --aws-sigv4 "aws:s3:us-east-1:s3" --aws-sigv4-mode "querystring" --user "AKIAIOSFODNN7EXAMPLE:wJalrXUtnFEMI/K7MDENG/bPxRfiCYEXAMPLEKEY" "http://example-bucket.s3.amazonaws.com/test.txt" 2>&1 | grep -E "(Canonical request|GET /test.txt)" | head -10

echo ""
echo "Tests completed. Check that:"
echo "1. Canonical request shows sorted parameters for signature calculation"
echo "2. Actual HTTP request line preserves original parameter order and appends AWS params"
