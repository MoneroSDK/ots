#!/bin/bash
if [ -z "$1" ]; then
  echo "Usage: $0 <tx_file>"
  exit 1
fi
if [ ! -f "$1" ]; then
  echo "File not found!"
  exit 1
fi
MONERO_WALLET_RPC=""
MONERO_WALLET_FILE=""
MONERO_WALLET_PASSWORD=""
MONERO_WALLET_IP="127.0.0.1"
MONERO_WALLET_PORT="9666"
${MONERO_WALLET_RPC} --wallet-file ${MONERO_WALLET_FILE} --password "${MONERO_WALLET_PASSWORD}" --disable-rpc-login --rpc-bind-ip ${MONERO_WALLET_IP} --rpc-bind-port ${MONERO_WALLET_PORT} &
HEX=$(hex -x "$1")
sleep 60
curl -X POST http://${MONERO_WALLET_IP}:${MONERO_WALLET_PORT}/json_rpc -d "{\"jsonrpc\": \"2.0\", \"id\": \"0\", \"method\": \"describe_transfer\", \"params\": {\"unsigned_txset\": \"${HEX}\"}" -H 'Content-Type: application/json' > $1_json
kill %1
