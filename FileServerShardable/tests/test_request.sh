#!/bin/bash

# create directories where we will import score files
mkdir -p ./data
PORT=3005

# run the server
../fs_server_app/bin/fs_server_app --log_level debug --server_id 666275 --num_servers 1 --num_entries_per_file 128 --port $PORT --num_connections 2 --request_arrival_timeout 300000 --connection_timeout 2000 --data_dir ./data --dir_depth 3 --dir_count 1000 2 &
pid1=$!
echo "pid of the server : " && echo $pid1

#wait a bit for the server to start up
sleep 2

# run the client to import test data
../fs_client_app/bin/fs_client_app  --server_addr 127.0.0.1 --server_port $PORT --request "save_file" --file_type 1 --id 1 --file_name 1.score

# run the client and make requests
../fs_client_app/bin/fs_client_app  --server_addr 127.0.0.1 --server_port $PORT --request "get_file" --file_type 1 --id 1 --file_name resp1.score

# tell the server to terminate
kill $pid1

# allow server to shut down
timeout 2 tail --pid=$pid1 -f /dev/null

# cause sometimes it deadlocks
kill -9 $pid1

#check if the saved file is the same as the ones that got saved
TEST_RESULT=0

function compareResultFile() {
	cmp -s "${1}" "${2}"
	CMP_RESULT=$?
	if [ $CMP_RESULT -ne 0 ]
	then
		echo -e "\033[0;31m Expected result mismatch between '$1' and '$2'\033[0m"
		TEST_RESULT=1
	fi    
}

compareResultFile "1.score" "resp1.score"
	
# cleanup
rm -r ./data
rm resp1.score
	
echo "Test done"

exit $TEST_RESULT