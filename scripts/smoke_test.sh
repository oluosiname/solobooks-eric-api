#!/usr/bin/env bash
# Pre-deploy smoke test for the eric-api.
#
# Exercises the paths that have no automated coverage: ESt datenart detection,
# the ERiC plugin actually loading, and the status code returned on rejection.
# Run against a locally built production image before deploying.
#
#   ./scripts/smoke_test.sh [base_url]
#
# Defaults to http://localhost:5051 (the port build_and_test.sh uses).

set -uo pipefail

BASE_URL="${1:-http://localhost:5051}"
PASS=0
FAIL=0

check() {
  local label="$1" expected="$2" actual="$3"
  if [ "$expected" = "$actual" ]; then
    printf '  \033[32mPASS\033[0m  %-46s %s\n' "$label" "$actual"
    PASS=$((PASS + 1))
  else
    printf '  \033[31mFAIL\033[0m  %-46s expected %s, got %s\n' "$label" "$expected" "$actual"
    FAIL=$((FAIL + 1))
  fi
}

# The samples are complete, ERiC-valid documents. Asserting they VALIDATE is a
# far stronger check than the old skeleton, which could only assert a rejection.
SAMPLES_DIR="$(dirname "$0")/../postman"

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

for pair in "est:sample_est_2025.xml" "ustva:sample_ustva_2025.xml" "zmdo:sample_zmdo_2025.xml"; do
  key="${pair%%:*}"; file="${pair#*:}"
  if [ ! -f "$SAMPLES_DIR/$file" ]; then
    echo "  MISSING SAMPLE: $SAMPLES_DIR/$file" >&2
    exit 1
  fi
  python3 - "$SAMPLES_DIR/$file" "$TMP/$key.json" "$TMP/${key}_explicit.json" "$key" <<'PY'
import json, sys
src, auto_out, explicit_out, key = sys.argv[1:5]
xml = open(src, encoding="utf-8").read()
version = {"est": "ESt_2025", "ustva": "UStVA_2025", "zmdo": "ZMDO"}[key]
json.dump({"xml": xml}, open(auto_out, "w"))
json.dump({"xml": xml, "datenartversion": version}, open(explicit_out, "w"))
PY
done

python3 - "$TMP" <<'PY'
import base64, json, os, sys
tmp = sys.argv[1]
body = json.load(open(os.path.join(tmp, "est.json")))
body.update(cert_base64=base64.b64encode(b"not-a-real-pfx").decode(),
            password="wrong", datenartversion="ESt_2025", return_pdf=False)
json.dump(body, open(os.path.join(tmp, "submit.json"), "w"))
PY

post() {
  curl -s --max-time 180 -o "$2" -w '%{http_code}' \
    -X POST "$BASE_URL$1" -H 'Content-Type: application/json' -d @"$3"
}

echo
echo "eric-api smoke test -> $BASE_URL"
echo

echo "Health"
check "GET /health" "200" \
  "$(curl -s --max-time 30 -o "$TMP/h" -w '%{http_code}' "$BASE_URL/health")"
check "status is ok" "ok" \
  "$(python3 -c "import json;print(json.load(open('$TMP/h'))['status'])" 2>/dev/null || echo parse-error)"

echo
echo "Validation - all three form types"
for pair in "est:ESt" "ustva:UStVA" "zmdo:ZMDO"; do
  key="${pair%%:*}"; label="${pair#*:}"
  check "$label validates (datenart auto-detected)" "true" \
    "$(post /validate "$TMP/${key}_v1" "$TMP/$key.json" >/dev/null; \
       python3 -c "import json;print(str(json.load(open('$TMP/${key}_v1'))['valid']).lower())" 2>/dev/null || echo parse-error)"
  check "$label validates (explicit datenartversion)" "true" \
    "$(post /validate "$TMP/${key}_v2" "$TMP/${key}_explicit.json" >/dev/null; \
       python3 -c "import json;print(str(json.load(open('$TMP/${key}_v2'))['valid']).lower())" 2>/dev/null || echo parse-error)"
done

echo
echo "Rejection status code"
check "POST /submit, rejected -> 422" "422" \
  "$(post /submit "$TMP/s1" "$TMP/submit.json")"

echo
if [ "$FAIL" -eq 0 ]; then
  printf '\033[32m%d passed, 0 failed\033[0m\n\n' "$PASS"
else
  printf '\033[31m%d passed, %d FAILED\033[0m\n\n' "$PASS" "$FAIL"
fi
exit $(( FAIL > 0 ))
