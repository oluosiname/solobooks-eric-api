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

# A skeleton E10. Deliberately incomplete: enough for ERiC to load the ESt
# plugin and reject on content, which is what proves the plugin resolved.
read -r -d '' EST_XML <<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<Elster xmlns="http://www.elster.de/elsterxml/schema/v11">
  <TransferHeader version="11">
    <Verfahren>ElsterErklaerung</Verfahren>
    <DatenArt>ESt</DatenArt>
    <Vorgang>send-Auth</Vorgang>
    <Testmerker>700000004</Testmerker>
  </TransferHeader>
  <DatenTeil>
    <Nutzdatenblock>
      <NutzdatenHeader version="11"><NutzdatenTicket>1</NutzdatenTicket></NutzdatenHeader>
      <Nutzdaten>
        <E10 xmlns="http://finkonsens.de/elster/elstererklaerung/est/e10/v2025"/>
      </Nutzdaten>
    </Nutzdatenblock>
  </DatenTeil>
</Elster>
XML

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

python3 - "$TMP" <<PY
import base64, json, os, sys
tmp = sys.argv[1]
xml = """$EST_XML"""
json.dump({"xml": xml}, open(os.path.join(tmp, "validate.json"), "w"))
json.dump({"xml": xml, "datenartversion": "ESt_2025"},
          open(os.path.join(tmp, "validate_explicit.json"), "w"))
json.dump({"xml": xml,
           "cert_base64": base64.b64encode(b"not-a-real-pfx").decode(),
           "password": "wrong", "datenartversion": "ESt_2025", "return_pdf": False},
          open(os.path.join(tmp, "submit.json"), "w"))
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
echo "ESt datenart detection"
check "POST /validate, no datenartversion" "200" \
  "$(post /validate "$TMP/v1" "$TMP/validate.json")"
# 610301200 is a content validation failure, which only happens once the ESt
# plugin has loaded. A datenart failure would surface as HTTP 400 instead.
check "ESt plugin ran (err 610301200)" "610301200" \
  "$(python3 -c "import json;print(json.load(open('$TMP/v1'))['error_code'])" 2>/dev/null || echo parse-error)"
check "POST /validate, explicit ESt_2025" "200" \
  "$(post /validate "$TMP/v2" "$TMP/validate_explicit.json")"
check "explicit matches auto-detect" "610301200" \
  "$(python3 -c "import json;print(json.load(open('$TMP/v2'))['error_code'])" 2>/dev/null || echo parse-error)"

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
