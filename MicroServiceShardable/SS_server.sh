#!/bin/bash

LOG_LEVEL_ARG=${LOG_LEVEL:-"debug"}
COMPANY_DIR_ARG=${COMPANY_DIR:-"/scoring-similarity-service/data/companies"}
INDUSTRY_DIR_ARG=${COMPANY_DIR:-"/scoring-similarity-service/data/industries"}
TITLE_DIR_ARG=${COMPANY_DIR:-"/scoring-similarity-service/data/titles"}
PROFILE_DIR_ARG=${PROFILE_DIR:-"/scoring-similarity-service/data/profiles"}
PORT_ARG=${PORT:-3002}
REQUEST_ARRIVAL_TIMEOUT_ARG=${REQUEST_ARRIVAL_TIMEOUT:-120000}
CONNECTION_TIMEOUT_ARG=${CONNECTION_TIMEOUT:-2000}
NUM_CONNECTIONS_ARG=${NUM_CONNECTIONS:-8}

mkdir -p "${COMPANY_DIR_ARG}"
mkdir -p "${INDUSTRY_DIR_ARG}"
mkdir -p "${TITLE_DIR_ARG}"
mkdir -p "${PROFILE_DIR_ARG}"

exec ./ss_server_app/bin/ss_server_app --log_level "${LOG_LEVEL_ARG}" --company_dir "${COMPANY_DIR_ARG}" --industry_dir "${INDUSTRY_DIR_ARG}" --title_dir "${TITLE_DIR_ARG}" --profile_dir "${PROFILE_DIR_ARG}" --port "${PORT_ARG}" --num_connections "${NUM_CONNECTIONS_ARG}" --request_arrival_timeout "${REQUEST_ARRIVAL_TIMEOUT_ARG}" --connection_timeout "${CONNECTION_TIMEOUT_ARG}"
