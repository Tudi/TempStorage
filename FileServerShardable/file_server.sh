#!/bin/bash

LOG_LEVEL_ARG=${LOG_LEVEL:-"debug"}
NUM_SERVERS_ARG=${NUM_EXTERNAL_SHARDS:-1}
NUM_ENTRIES_PER_FILE_ARG=${NUM_ENTRIES_PER_FILE:-1}
PORT_ARG=${PORT:-3002}
REQUEST_ARRIVAL_TIMEOUT_ARG=${REQUEST_ARRIVAL_TIMEOUT:-300000}
CONNECTION_TIMEOUT_ARG=${CONNECTION_TIMEOUT:-300000}
NUM_CONNECTIONS_ARG=${NUM_CONNECTIONS:-16}
DATA_DIR_ARG=${DATA_DIR:-"./data"} 
DIR_DEPTH_ARG=${DIR_DEPTH:-2} 
DIR_COUNT_ARG=${DIR_COUNT:-100} 

HOSTNAME_SUFFIX=$((echo "${HOSTNAME}" | tr -dc 0-9))
SERVER_ID_ARG=$((HOSTNAME_SUFFIX+1))

exec ./fs_server_app/bin/fs_server_app --log_level "${LOG_LEVEL_ARG}" --server_id "${SERVER_ID_ARG}" --num_servers "${NUM_SERVERS_ARG}" --num_entries_per_file "${NUM_ENTRIES_PER_FILE_ARG}" --port "${PORT_ARG}" --num_connections "${NUM_CONNECTIONS_ARG}" --request_arrival_timeout "${REQUEST_ARRIVAL_TIMEOUT_ARG}" --connection_timeout "${CONNECTION_TIMEOUT_ARG}" --data_dir "${DATA_DIR_ARG}" --dir_depth "${DIR_DEPTH_ARG}" --dir_count "${DIR_COUNT_ARG}"
