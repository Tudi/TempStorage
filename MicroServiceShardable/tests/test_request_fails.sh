#!/bin/bash

# create directories where we will import score files
mkdir -p ./data/companies

# run the server
../ss_server_app/bin/ss_server_app --log_level debug --company_dir ./data/companies --industry_dir ./data/industries --title_dir ./data/titles --profile_dir ./data/profiles --port 3005 --request_arrival_timeout 2000 --connection_timeout 2000 --num_connections 2 &
pid1=$!
echo "pid of the server : " && echo $pid1

#wait a bit for the server to start up
sleep 2

# run the client to import test data
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "save_score" --id 1 --similarity_score_file 1.score --similarity_type 1

# run the client and make requests

#file 2 is missing
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_12.bin --request_ascii_data 2,1,1,2
# same file is merged
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_22.bin --request_ascii_data 2,1,2,2
# out of order ids
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_21.bin --request_ascii_data 2,1,2,1
# block 0 is empty, block 1 generates data
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_3421.bin --request_ascii_data 2,1,3,4,2,1,2,1
# block 0 generates data, block 1 is empty
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_2134.bin --request_ascii_data 2,1,2,1,2,1,3,4
# block 0 is empty, block 1 is empty
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_001.bin --request_ascii_data 2,1,3,4,2,2,2,1
# block 0 is empty, block 1 is empty
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_002.bin --request_ascii_data 2,2,2,1,2,2,1,2
# block 0 invalid score type, block 1 generates data
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_003.bin --request_ascii_data 2,5,2,1,2,1,1,2
# block 1 generates data, block 1 is empty due to invalid block type
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3005 --request "get_score_ascii" --log_level debug --save_repl_packet resp3_004.bin --request_ascii_data 2,1,2,1,2,5,1,2

# allow the server to close the connections properly
sleep 1

# tell the server to terminate
kill $pid1 >/dev/null 2>&1

# allow server to shut down
timeout 2 tail --pid=$pid1 -f /dev/null

# cause sometimes it deadlocks
kill -9 $pid1 >/dev/null 2>&1

#check if the saved file is the same as the ones that got saved
TEST_RESULT=0

function compareResultFile() {
	cmp -s "${1}" "${2}"
	CMP_RESULT=$?
	if [ $CMP_RESULT -ne 0 ]
	then
		echo -e "\033[0;31m Unexpected result mismatch between '$1' and '$2'\033[0m"
		TEST_RESULT=1
	fi    
}

compareResultFile "expected_resp3_12.bin" "resp3_12.bin"
compareResultFile "expected_resp3_22.bin" "resp3_22.bin" 
compareResultFile "expected_resp3_21.bin" "resp3_21.bin" 
compareResultFile "expected_resp3_3421.bin" "resp3_3421.bin" 
compareResultFile "expected_resp3_2134.bin" "resp3_2134.bin" 
compareResultFile "expected_resp3_001.bin" "resp3_001.bin" 
compareResultFile "expected_resp3_002.bin" "resp3_002.bin" 
compareResultFile "expected_resp3_12.bin" "resp3_003.bin" 
compareResultFile "expected_resp3_12.bin" "resp3_004.bin" 
	
# cleanup
rm -r ./data
rm resp3_12.bin
rm resp3_22.bin
rm resp3_21.bin
rm resp3_2134.bin
rm resp3_3421.bin
rm resp3_001.bin
rm resp3_002.bin
rm resp3_003.bin
rm resp3_004.bin
	
echo "Test done"

exit $TEST_RESULT