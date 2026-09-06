#!/usr/bin/env bash
# Build the PRODUCTION image and smoke test it before deploying.
#
# `docker compose up` runs Dockerfile (Flask dev server, root user). Production
# runs Dockerfile.prod (gunicorn, non-root, healthcheck). Those differ enough
# that testing the dev container does not tell you the deploy will work — this
# script tests what actually ships.
#
#   ./scripts/build_and_test.sh

set -euo pipefail

cd "$(dirname "$0")/.."

IMAGE="eric-api:pre-deploy"
CONTAINER="eric-api-pre-deploy"
PORT="${PORT:-5051}"

cleanup() { docker rm -f "$CONTAINER" >/dev/null 2>&1 || true; }
trap cleanup EXIT

echo
echo "==> Building $IMAGE from Dockerfile.prod"
docker build --platform linux/amd64 -f Dockerfile.prod -t "$IMAGE" . >/dev/null

echo "==> Starting $CONTAINER on port $PORT (gunicorn, non-root)"
cleanup
docker run -d --name "$CONTAINER" --platform linux/amd64 -p "$PORT:5000" "$IMAGE" >/dev/null

echo -n "==> Waiting for health"
for i in $(seq 1 30); do
  if curl -sf --max-time 10 "http://localhost:$PORT/health" >/dev/null 2>&1; then
    echo " ok"
    break
  fi
  if [ "$i" -eq 30 ]; then
    echo " TIMED OUT"
    echo
    docker logs "$CONTAINER" 2>&1 | tail -30
    exit 1
  fi
  echo -n "."
  sleep 2
done

./scripts/smoke_test.sh "http://localhost:$PORT"
STATUS=$?

if [ "$STATUS" -ne 0 ]; then
  echo "Container logs:"
  docker logs "$CONTAINER" 2>&1 | tail -30
fi

exit "$STATUS"
