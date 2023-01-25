#!/bin/bash

# create directories where we will import score files
mkdir -p ./data/companies
mkdir -p ./data/industries

# run the server
../ss_server_app/bin/ss_server_app --log_level debug --company_dir ./data/companies --industry_dir ./data/industries --title_dir ./data/titles --profile_dir ./data/profiles --port 3004 --request_arrival_timeout 2000 --connection_timeout 2000 --num_connections 2 &
pid1=$!
echo "pid of the server : " && echo $pid1

#wait a bit for the server to start up
sleep 2

# run the client to import test data
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "save_score" --id 1 --similarity_score_file 1.score --similarity_type 1
# import second time to test any chance of mutex issues
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "save_score" --id 1 --similarity_score_file 1.score --similarity_type 1
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "save_score" --id 2 --similarity_score_file 2.score --similarity_type 1
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "save_score" --id 1 --similarity_score_file 1.score --similarity_type 2
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "save_score" --id 2 --similarity_score_file 2.score --similarity_type 2

# run the client and make requests
# request without any merge
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_1.bin --request_ascii_data 1,1,1
# request second time to test any chance of mutex issues
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_1.bin --request_ascii_data 1,1,1
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_1.bin --request_ascii_data 1,1,1
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_1.bin --request_ascii_data 1,1,1
# request is using merge 2 arrays algo
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_12.bin --request_ascii_data 2,1,1,2
# requests 2 blocks of score merges
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_1212.bin --request_ascii_data 2,1,1,2,2,2,1,2
# requests 4 blocks of score merges(each 3 files) : company, industry, title, profile
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_4types.bin --request_ascii_data 3,1,1,2,3,3,2,1,2,3,3,3,1,2,3,3,4,1,2,3
# merge 5 blocks to use "slice" merging
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_5files.bin --request_ascii_data 5,1,1,2,3,4,5
# merge 3 blocks, no merge, one uses arraymerge, other uses slice merge
../ss_client_app/bin/ss_client_app  --server_addr 127.0.0.1 --server_port 3004 --request "get_score_ascii" --log_level debug --save_req_packet req.bin --save_repl_packet resp_3algo.bin --request_ascii_data 1,1,1,2,2,1,2,5,3,1,2,3,4,5

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

compareResultFile "resp_expected1.bin" "resp_1.bin"
compareResultFile "resp_expected2.bin" "resp_12.bin"
compareResultFile "resp_expected3.bin" "resp_1212.bin"
compareResultFile "expected_resp_4types.bin" "resp_4types.bin"
compareResultFile "expected_resp_5files.bin" "resp_5files.bin"
compareResultFile "expected_resp_3algo.bin" "resp_3algo.bin"
	
# cleanup
rm -r ./data
rm req.bin
rm resp_1.bin
rm resp_12.bin
rm resp_1212.bin
rm resp_4types.bin
rm resp_5files.bin
rm resp_3algo.bin
	
echo "Test done"

exit $TEST_RESULT